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
#include "HAL/UnrealMemory.h"

#define DO_TASK(task) FFunctionGraphTask::CreateAndDispatchWhenReady(task, TStatId(), nullptr);

/**
 * Registered to LibJuice Log
 */
static void LogCallback(juice_log_level_t level, const char *message)
{
	UE_LOG_ABNET(Verbose, TEXT("JUICE LOG : %hs"), message);
}

AccelByteJuice::AccelByteJuice(const FString& PeerId): AccelByteICEBase(PeerId)
{
	juice_set_log_handler(LogCallback);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	juice_set_log_level(JUICE_LOG_LEVEL_VERBOSE);
#else
	juice_set_log_level(JUICE_LOG_LEVEL_NONE);
#endif
}

void AccelByteJuice::OnSignalingMessage(const FString& Message)
{
	//Signaling message encoded to base64 string because sometimes it having special character that I am not sure if it supported by normal json string
	FString Base64Decoded;
	FBase64::Decode(Message, Base64Decoded);
	const TSharedPtr<FJsonObject> Json = StringToJson(Base64Decoded);
	HandleMessage(Json);
}

bool AccelByteJuice::RequestConnect()
{
	bIsInitiator = true;
	//TODO: The turn server should get from our Qos server. This code can be move to base class.
	/*
	 * We don't have STUN server config here because we put it only 1 as TURN server
	 * the TURN server can act as both TURN and STUN server
	 * so the TURN server is mandatory here
	 */
	FString TurnHost;
	int TurnPort = 0;
	FString TurnUserName;
	FString TurnPassword;
	FString TurnError;
	if(!GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerUrl"), TurnHost, GEngineIni))
	{
		UE_LOG_ABNET(Error, TEXT("TurnServerUrl was missing in DefaultEngine.ini"));
        return false;
	}
	
	if(!GConfig->GetInt(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerPort"), TurnPort, GEngineIni))
	{
		UE_LOG_ABNET(Error, TEXT("TurnServerPort was missing in DefaultEngine.ini"));
		return false;
	}
	
	// Username password possible empty if no authentication required on TURN server
	GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerUsername"), TurnUserName, GEngineIni);
	GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerPassword"), TurnPassword, GEngineIni);
	
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("ice"));
	Json->SetStringField(TEXT("host"), TurnHost);
	Json->SetStringField(TEXT("username"), TurnUserName);
	Json->SetStringField(TEXT("password"), TurnPassword);
	Json->SetNumberField(TEXT("port"), TurnPort);	
	Json->SetStringField(TEXT("server_type"), TEXT("offer"));
	if (Signaling->IsConnected())
	{
		FString JsonString;
		JsonToString(JsonString, Json);
		const FString Base64String = FBase64::Encode(JsonString);
		Signaling->SendMessage(PeerId, Base64String);
		DO_TASK(([this, TurnHost, TurnUserName, TurnPassword, TurnPort]() {
            CreatePeerConnection(TurnHost, TurnUserName, TurnPassword, TurnPort);
        }));
		return true;
	}
	return false;
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
	JuiceConfig.turn_servers = TurnServerConfig;
	JuiceConfig.turn_servers_count = 1;
	JuiceConfig.user_ptr = this;
#if PLATFORM_PS4
	// Bind any port not working on PS4, but it works when the bind port is set to 10000-10100
	JuiceConfig.local_port_range_begin = 10000;
	JuiceConfig.local_port_range_end = 10100;
#endif
	JuiceAgent = juice_create(&JuiceConfig);

	if(IsPeerReady())
	{
		SetupLocalDescription();
		// Apply Session Description Protocol (SDP) from remote that is in queue
		TSharedPtr<FJsonObject> Json;
		if (SdpQueue.Dequeue(Json))
		{
			HandleMessage(Json);
		}
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
					OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound("Juice gather candidates failed");
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
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound("Juice get local description failed");
	}
}

void AccelByteJuice::JuiceStateChanged(juice_state_t State)
{
	UE_LOG_ABNET(Log, TEXT("Juice state changed : %d"), State);
	if(State == JUICE_STATE_CONNECTED)
	{
		bIsConnected = true;
		OnICEDataChannelConnectedDelegate.ExecuteIfBound(PeerId);
	}
	else if(State == JUICE_STATE_FAILED)
	{
		ClosePeerConnection();
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound("Juice connection failed");
	}
	else if(State == JUICE_STATE_DISCONNECTED)
	{
		ClosePeerConnection();
		OnICEDataChannelClosedDelegate.ExecuteIfBound(PeerId);
	}
}

void AccelByteJuice::JuiceCandidate(const char* Candidate)
{
	UE_LOG_ABNET(Log, TEXT("Juice local candidate : %hs"), Candidate);
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("candidate"));
	Json->SetStringField(TEXT("candidate"), FString(Candidate));
	FString JsonString;
	JsonToString(JsonString, Json);
	const FString Base64String = FBase64::Encode(JsonString);
	Signaling->SendMessage(PeerId, Base64String);
}

void AccelByteJuice::JuiceGatheringDone()
{
	UE_LOG_ABNET(Log, TEXT("Juice Gathering Done"));
}

void AccelByteJuice::JuiceDataRecv(const char* data, size_t size)
{
	check(!PeerId.IsEmpty());
	OnICEDataReadyDelegate.ExecuteIfBound(PeerId, (uint8*)data, size);
}

#endif
