// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifdef LIBJUICE

#include "AccelByteJuice.h"
#include "AccelByteSignalingBase.h"
#include "Dom/JsonObject.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "Async/TaskGraphInterfaces.h" 
#include "Misc/Base64.h"
#include "Misc/CommandLine.h"
#include "HAL/UnrealMemory.h"
#include "Core/AccelByteDefines.h"
#include "AccelByteSignalingConstants.h"

#define DO_TASK(task) FFunctionGraphTask::CreateAndDispatchWhenReady(task, TStatId(), nullptr);
#define MAX_ADDRESS_LENGTH 256

/**
 * Registered to LibJuice Log
 */
static void LogCallback(juice_log_level_t level, const char *message)
{
	UE_LOG_ABNET(Log, TEXT("JUICE LOG : %hs"), message);
}

AccelByteJuice::AccelByteJuice(const FString& PeerId): AccelByteICEBase(PeerId)
{
	juice_set_log_handler(LogCallback);	

	FString ParamValue;
	if(FParse::Value(FCommandLine::Get(), TEXT("iceforcerelay"), ParamValue))
	{
		bIsForceIceUsingRelay = true;
	}
	if(FParse::Value(FCommandLine::Get(), TEXT("juiceverbose"), ParamValue))
	{
		juice_set_log_level(JUICE_LOG_LEVEL_VERBOSE);
	}
	else
	{
		juice_set_log_level(JUICE_LOG_LEVEL_NONE);
	}

	if (!GConfig->GetInt(TEXT("AccelByteNetworkUtilities"), TEXT("HostCheckTimeout"), HostCheckTimeout, GEngineIni))
	{
		UE_LOG_ABNET(Log, TEXT("Using default HostCheckTimeout (%d seconds) because its missing from DefaultEngine.ini"), HostCheckTimeout);
	}
}

void AccelByteJuice::OnSignalingMessage(const FString& Message)
{
	if (Message.Contains(FString::Printf(TEXT("%s%s"), HOST_CHECK_MESSAGE, HOST_CHECK_REPLY_DELIMITER)))
	{
		UpdatePeerStatus(Message);
		return;
	}
	
	//Signaling message encoded to base64 string because sometimes it having special character that I am not sure if it supported by normal json string
	FString Base64Decoded;
	FBase64::Decode(Message, Base64Decoded);
	const TSharedPtr<FJsonObject> Json = StringToJson(Base64Decoded);
	HandleMessage(Json);
}

bool AccelByteJuice::RequestConnect(const FString &ServerUrl, int ServerPort, const FString &Username, const FString &Password)
{
	InitiateConnectionTime = FDateTime::Now();

	if (!Signaling->IsConnected())
	{
		UE_LOG_ABNET(Error, TEXT("Signaling server is disconnected"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, SignalingServerDisconnected);
		return false;
	}

	bIsInitiator = true;
	bIsCheckingHost = true;

	SelectedTurnServerCred = FAccelByteModelsTurnServerCredential{
		ServerUrl, ServerPort, FString(), Username, Password};

	Signaling->SendMessage(PeerId, HOST_CHECK_MESSAGE);
	PeerStatus = WaitingReply;

	const FTickerDelegate TickerDelegate = FTickerDelegate::CreateRaw(this, &AccelByteJuice::Tick);
	FTickerAlias::GetCoreTicker().AddTicker(TickerDelegate, 0.5);

	return true;
}

bool AccelByteJuice::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
	if (!bIsConnected || JuiceAgent == nullptr)
	{
		return false;
	}
	check(Count >= 0);
	const int Result = juice_send(JuiceAgent, (char*)Data, Count);
	if(Result == JUICE_ERR_SUCCESS)
	{
		BytesSent = Count;
		return true;
	}	
	return false;
}

void AccelByteJuice::ClosePeerConnection()
{
	UE_LOG_ABNET(Log, TEXT("Closing peer connection : %s"), *PeerId);
	bIsConnected = false;
	DescriptionReadyMutex.Lock();
	bIsDescriptionReady = false;
	DescriptionReadyMutex.Unlock();
	if(JuiceAgent != nullptr)
	{
		juice_agent_t* ToDestroy = JuiceAgent;
		JuiceAgent = nullptr;
		juice_destroy(ToDestroy);		
	}
}

bool AccelByteJuice::IsPeerReady() const
{
	return JuiceAgent != nullptr;
}

void AccelByteJuice::CreatePeerConnection(const FString& Host, const FString &Username, const FString &Password, int port)
{
	if(IsPeerReady())
	{
		// peer juice agent already created
		return;
	}
	
	UE_LOG_ABNET(Log, TEXT("Create peer connection to : %s"), *Host);
	juice_turn_server TurnServerConfig[1];

	/*
	 * Host, Username, and password are temporary and don't know when it get destroyed
	 * So save it on its own char storage helper, because not sure when libjuice need it
	 */
	FMemory::Memzero(ConfigHelper.Host, TURN_MAX_LENGTH);
	FPlatformString::Strcpy(ConfigHelper.Host, Host.Len(), TCHAR_TO_ANSI(*Host));
	FMemory::Memzero(ConfigHelper.Username, TURN_MAX_LENGTH);
	FPlatformString::Strcpy(ConfigHelper.Username, Username.Len(), TCHAR_TO_ANSI(*Username));
	FMemory::Memzero(ConfigHelper.Password, TURN_MAX_LENGTH);
	FPlatformString::Strcpy(ConfigHelper.Password, Password.Len(), TCHAR_TO_ANSI(*Password));
	
	TurnServerConfig[0].host = ConfigHelper.Host;
	TurnServerConfig[0].port = port;
	TurnServerConfig[0].username = ConfigHelper.Username;
	TurnServerConfig[0].password = ConfigHelper.Password;
	memset(&JuiceConfig, 0, sizeof(JuiceConfig));
	SetupCallback();
	JuiceConfig.concurrency_mode = JUICE_CONCURRENCY_MODE_THREAD;
	JuiceConfig.turn_servers = TurnServerConfig;
	JuiceConfig.turn_servers_count = 1;
	JuiceConfig.user_ptr = this;
#if (defined(PLATFORM_PS4) && PLATFORM_PS4) || (defined(PLATFORM_PS5) && PLATFORM_PS5)
	// Bind any port not working on PS4, but it works when the bind port is set to 10000-10100
	JuiceConfig.local_port_range_begin = 10000;
	JuiceConfig.local_port_range_end = 10100;
#endif
	JuiceAgent = juice_create(&JuiceConfig);

	SetupLocalDescription();
	// Apply Session Description Protocol (SDP) from remote that is in queue
	TSharedPtr<FJsonObject> Json;
	if (SdpQueue.Dequeue(Json))
	{
		HandleMessage(Json);
	}
}

void AccelByteJuice::SetupCallback()
{
	JuiceConfig.cb_state_changed = [](juice_agent_t *agent, juice_state_t state, void *user_ptr)
	{
		AccelByteJuice* Owner = static_cast<AccelByteJuice*>(user_ptr);
		if(Owner != nullptr)
		{
			Owner->JuiceStateChanged(state);
		}
	};
	JuiceConfig.cb_candidate = [](juice_agent_t *agent, const char *sdp, void *user_ptr)
	{
		AccelByteJuice* Owner = static_cast<AccelByteJuice*>(user_ptr);
		if(Owner != nullptr)
		{
			Owner->JuiceCandidate(sdp);
		}
	};
	JuiceConfig.cb_recv = [](juice_agent_t *agent, const char *data, size_t size, void *user_ptr)
	{
		AccelByteJuice* Owner = static_cast<AccelByteJuice*>(user_ptr);
		if(Owner != nullptr)
		{
			Owner->JuiceDataRecv(data, size);
		}
	};
	JuiceConfig.cb_gathering_done = [](juice_agent_t *agent, void *user_ptr)
	{
		AccelByteJuice* Owner = static_cast<AccelByteJuice*>(user_ptr);
		if(Owner != nullptr)
		{
			Owner->JuiceGatheringDone();
		}
	};
}

void AccelByteJuice::HandleMessage(TSharedPtr<FJsonObject> Json)
{
	/*
	 * Message coming from signaling message, the message is actually from remote peer that want to connect
	 * Peers need to exchange data of turn server, candidate and sdp
	 */
	const FString Type = Json->GetStringField(TEXT("type"));
	FString JsonString;
	JsonToString(JsonString, Json.ToSharedRef());
	UE_LOG_ABSIGNALING(Verbose, TEXT("Signaling Message : %s"), *JsonString);
	if (Type.Equals(TEXT("ice")))
	{
		/*
		 * Message type of "ice" means that the remote peer wants to connect with a specific TURN server
		 * so the peer and host connected to the same turn server to make the nat relay happen correctly
		 */
		const FString ServerType = Json->GetStringField(TEXT("server_type"));
		if (ServerType.Equals(TEXT("offer")))
		{
			const FString Host = Json->GetStringField(TEXT("host"));
			const FString Username = Json->GetStringField(TEXT("username"));
			const FString Password = Json->GetStringField(TEXT("password"));
			const int32 Port = Json->GetIntegerField(TEXT("port"));
			DO_TASK(([this, Host, Username, Password, Port]() {
				CreatePeerConnection(Host, Username, Password, Port);
			}));
		}
	}
	else if (Type.Equals(TEXT("sdp")))
	{
		/*
		* There is SDP info from remote peer that will be apply to juice instance
		* https://tools.ietf.org/id/draft-nandakumar-rtcweb-sdp-01.html
		*/
		const FString Sdp = Json->GetStringField(TEXT("sdp"));		
		if(IsPeerReady())
		{
			DO_TASK(([this, Sdp]()
			{
				juice_set_remote_description(JuiceAgent, TCHAR_TO_ANSI(*Sdp));
				UE_LOG_ABNET(Log, TEXT("Set remote description : %s"), *Sdp);
				const int Result = juice_gather_candidates(JuiceAgent);
				UE_LOG_ABNET(Log, TEXT("Gather Candidate Result : %d"), Result);
				if(Result == JUICE_ERR_SUCCESS)
				{
					DescriptionReadyMutex.Lock();
					bIsDescriptionReady = true;
					DescriptionReadyMutex.Unlock();
					
					/*
					* The local description is ready and all peer candidate can be applied to juice instance
					*/
					TSharedPtr<FJsonObject> JsonCandidate;
					while (CandidateQueue.Dequeue(JsonCandidate))
					{
						HandleMessage(JsonCandidate);
					}
				}
				else
				{
					UE_LOG_ABNET(Error, TEXT("Failed gathering juice candidate"));
					OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, JuiceGatherFailed);
				}
			}));
		}
		else
		{
			// Queue it and apply when local description ready
			SdpQueue.Enqueue(Json);
		}		
	}
	else if (Type.Equals(TEXT("candidate")))
	{
		/*
		 * Candidate of IP address that can be connected to from peer
		 * This need to apply juice instance to make the peer-to-peer connection
		 */
		DescriptionReadyMutex.Lock();
		const bool bIsDescriptionReadyCopy = bIsDescriptionReady;
		DescriptionReadyMutex.Unlock();
		if(bIsDescriptionReadyCopy)
		{
			const FString Candidate = Json->GetStringField(TEXT("candidate"));
			if(bIsForceIceUsingRelay && !Candidate.Contains(TEXT("typ relay")))
			{
				return;
			}
			UE_LOG_ABNET(Log, TEXT("Set remote candidate : %s"), *Candidate);
			DO_TASK(([JuiceAgent = this->JuiceAgent, Candidate]()
			{
				juice_add_remote_candidate(JuiceAgent, TCHAR_TO_ANSI(*Candidate));				
			}));
		}
		else
		{
			/*
			 * Undefined behaviour when candidate applied but the local description is not ready yet
			 */
			CandidateQueue.Enqueue(Json);
		}
	}
	else if (Type.Equals(TEXT("done")))
	{
		DO_TASK(([JuiceAgent = this->JuiceAgent]()
		{
			juice_set_remote_gathering_done(JuiceAgent);
		}));
	}
}

void AccelByteJuice::SetupLocalDescription()
{
	char Buffer[JUICE_MAX_SDP_STRING_LEN];
	const int Result = juice_get_local_description(JuiceAgent, Buffer, JUICE_MAX_SDP_STRING_LEN);
	UE_LOG_ABNET(Log, TEXT("Juice Local Description Result : %d"), Result);
	if(Result == JUICE_ERR_SUCCESS)
	{
		UE_LOG_ABNET(Log, TEXT("Juice Local Description : %hs"), Buffer);
		TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("type"), TEXT("sdp"));
		Json->SetStringField(TEXT("sdp"), FString(Buffer));
		FString JsonString;
		JsonToString(JsonString, Json);
		const FString Base64String = FBase64::Encode(JsonString);
		check(Signaling != nullptr);
		Signaling->SendMessage(PeerId, Base64String);
	}
	else
	{
		UE_LOG_ABNET(Error, TEXT("Failed getting juice local description"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, JuiceGetLocalDescriptionFailed);
	}
}

void AccelByteJuice::JuiceStateChanged(juice_state_t State)
{
	UE_LOG_ABNET(Log, TEXT("Juice state changed : %d"), State);
	if(State == JUICE_STATE_CONNECTED)
	{
		bIsConnected = true;
#ifndef PLATFORM_SWITCH
		char localAddress[MAX_ADDRESS_LENGTH];
		char remoteAddress[MAX_ADDRESS_LENGTH];
		int result = juice_get_selected_addresses(JuiceAgent, localAddress, MAX_ADDRESS_LENGTH, remoteAddress, MAX_ADDRESS_LENGTH);
		UE_LOG_ABNET(Log, TEXT("selected address: %d; local : %hs; remote: %hs"), result, localAddress, remoteAddress);
		char localCandidate[MAX_ADDRESS_LENGTH];
		char remoteCandidate[MAX_ADDRESS_LENGTH];
		result = juice_get_selected_candidates(JuiceAgent, localCandidate, MAX_ADDRESS_LENGTH, remoteCandidate, MAX_ADDRESS_LENGTH);
		UE_LOG_ABNET(Log, TEXT("selected candidate: %d; local : %hs; remote: %hs"), result, localCandidate, remoteCandidate);
#endif
		OnICEDataChannelConnectedDelegate.ExecuteIfBound(PeerId, GetP2PConnectionType());
	}
	else if(State == JUICE_STATE_FAILED)
	{
		UE_LOG_ABNET(Error, TEXT("Juice connection failed"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, JuiceConnectionFailed);
	}
	else if(State == JUICE_STATE_DISCONNECTED)
	{
		OnICEDataChannelClosedDelegate.ExecuteIfBound(PeerId);
	}
	if(LastJuiceState == JUICE_STATE_CONNECTING && (State == JUICE_STATE_FAILED || State == JUICE_STATE_COMPLETED || State == JUICE_STATE_DISCONNECTED))
	{
		UE_LOG_ABNET(Error, TEXT("please double check your TurnServerSecret, make sure it has the correct value, config located at .ini file under [AccelByteNetworkUtilities] path"));
	}
	LastJuiceState = State;
}

void AccelByteJuice::JuiceCandidate(const char* Candidate)
{
	UE_LOG_ABNET(Log, TEXT("Juice local candidate : %hs"), Candidate);
	FString InCandidate(Candidate);
	if(bIsForceIceUsingRelay && !InCandidate.Contains(TEXT("typ relay")))
	{
	    return;
	}
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("candidate"));
	Json->SetStringField(TEXT("candidate"), InCandidate);
	FString JsonString;
	JsonToString(JsonString, Json);
	const FString Base64String = FBase64::Encode(JsonString);
	Signaling->SendMessage(PeerId, Base64String);
}

void AccelByteJuice::JuiceGatheringDone()
{
	UE_LOG_ABNET(Log, TEXT("Juice Gathering Done"));
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("done"));
	FString JsonString;
	JsonToString(JsonString, Json);
	const FString Base64String = FBase64::Encode(JsonString);
	Signaling->SendMessage(PeerId, Base64String);
}

void AccelByteJuice::JuiceDataRecv(const char* data, size_t size)
{
	check(!PeerId.IsEmpty());
	OnICEDataReadyDelegate.ExecuteIfBound(PeerId, (uint8*)data, size);
}

EP2PConnectionType AccelByteJuice::GetP2PConnectionType() const
{
	char LocalCandidate[MAX_ADDRESS_LENGTH];
	char RemoteCandidate[MAX_ADDRESS_LENGTH];
	juice_get_selected_candidates(
		JuiceAgent, LocalCandidate, MAX_ADDRESS_LENGTH, RemoteCandidate, MAX_ADDRESS_LENGTH);

	// The connection type of remote and local candidates are the same, selected remote candidate used here
	const FString Candidate = FString(RemoteCandidate);
	EP2PConnectionType SelectedCandidateType = EP2PConnectionType::None;
	
	if (Candidate.Contains(TEXT("typ host")))
	{
		SelectedCandidateType = EP2PConnectionType::Host;
	}
	else if (Candidate.Contains(TEXT("typ srflx")))
	{
		SelectedCandidateType = EP2PConnectionType::Srflx;
	}
	else if (Candidate.Contains(TEXT("typ prflx")))
	{
		SelectedCandidateType = EP2PConnectionType::Prflx;
	}
	else if (Candidate.Contains(TEXT("typ relay")))
	{
		SelectedCandidateType = EP2PConnectionType::Relay;
	}

	UE_LOG_ABNET(Log, TEXT("Connection type: %s"), *UEnum::GetValueAsString(SelectedCandidateType));

	return SelectedCandidateType;
}

bool AccelByteJuice::Tick(float DeltaTime)
{
	if (bIsCheckingHost)
	{
		if (FDateTime::Now().ToUnixTimestamp() - InitiateConnectionTime.ToUnixTimestamp() > HostCheckTimeout)
		{
			UE_LOG_ABNET(Error, TEXT("Failed initiating connection to peer, timeout after %d seconds"), HostCheckTimeout);
			bIsCheckingHost = false;
			OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, HostResponseTimeout);
			return false;
		}

		switch (PeerStatus)
		{
			case WaitingReply:
				return true;

			case Hosting:
				{
					const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
					Json->SetStringField(TEXT("type"), TEXT("ice"));
					Json->SetStringField(TEXT("host"), SelectedTurnServerCred.Ip);
					Json->SetStringField(TEXT("username"), SelectedTurnServerCred.Username);
					Json->SetStringField(TEXT("password"), SelectedTurnServerCred.Password);
					Json->SetNumberField(TEXT("port"), SelectedTurnServerCred.Port);	
					Json->SetStringField(TEXT("server_type"), TEXT("offer"));

					FString JsonString;
					JsonToString(JsonString, Json);
					const FString Base64String = FBase64::Encode(JsonString);
					Signaling->SendMessage(PeerId, Base64String);
					CreatePeerConnection(SelectedTurnServerCred.Ip, SelectedTurnServerCred.Username,
						SelectedTurnServerCred.Password, SelectedTurnServerCred.Port);

					break;
				}

			case NotHosting:
				{
					UE_LOG_ABNET(Error, TEXT("Peer is not hosting"));
					OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerId, PeerIsNotHosting);
					break;
				}
		}

		bIsCheckingHost = false;
	}

	return false;
}

void AccelByteJuice::UpdatePeerStatus(const FString& Message)
{
	TArray<FString> ParsedStrings;
	Message.ParseIntoArray(ParsedStrings, HOST_CHECK_REPLY_DELIMITER);

	const bool bIsPeerHosting = ParsedStrings.Last().ToBool();

	if (bIsPeerHosting)
	{
		PeerStatus = Hosting;
	}
	else
	{
		PeerStatus = NotHosting;
	}
}

#endif
