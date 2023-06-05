// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifdef LIBJUICE

#include "AccelByteJuice.h"

#include "Async/Async.h"
#include "Misc/ConfigCacheIni.h"

#include "AccelByteSignalingBase.h"
#include "Dom/JsonObject.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "Async/TaskGraphInterfaces.h" 
#include "Misc/Base64.h"
#include "Misc/CommandLine.h"
#include "HAL/UnrealMemory.h"
#include "Misc/ConfigCacheIni.h"
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

void AccelByteJuice::OnSignalingMessage(const FAccelByteSignalingMessage &Message)
{
	HandleMessage(Message);
}

bool AccelByteJuice::RequestConnect(const FString &ServerUrl, int ServerPort, const FString &Username, const FString &Password)
{
	InitiateConnectionTime = FDateTime::Now();

	if (!Signaling->IsConnected())
	{
		UE_LOG_ABNET(Error, TEXT("Signaling server is disconnected"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::SignalingServerDisconnected);
		return false;
	}

	bIsInitiator = true;
	bIsCheckingHost = true;

	SelectedTurnServerCred = FAccelByteModelsTurnServerCredential{
		ServerUrl, ServerPort, FString(), Username, Password};

	FAccelByteSignalingMessage SignalingMessage;
	SignalingMessage.Type = SIGNALING_TYPE_CHECK_HOST;
	SignalingMessage.Channel = Channel;
	
	Signaling->SendMessage(PeerId, SignalingMessage);
	PeerStatus = EAccelBytePeerStatus::WaitingReply;

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
	FAccelByteSignalingMessage Message;
	if (SdpQueue.Dequeue(Message))
	{
		HandleMessage(Message);
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

void AccelByteJuice::HandleMessage(const FAccelByteSignalingMessage &Message)
{
	/*
	 * Message coming from signaling message, the message is actually from remote peer that want to connect
	 * Peers need to exchange data of turn server, candidate and sdp
	 */
	if (Message.Type.Equals(SIGNALING_TYPE_CHECK_HOST_REPLY))
	{
		UpdatePeerStatus(Message.Data.Equals(HOST_CHECK_MESSAGE_HOSTING));
	}
	else if (Message.Type.Equals(SIGNALING_TYPE_ICE))
	{
		/*
		 * Message type of "ice" means that the remote peer wants to connect with a specific TURN server
		 * so the peer and host connected to the same turn server to make the nat relay happen correctly
		 */
		if (Message.Data.Equals(SIGNALING_OFFER))
		{
			DO_TASK(([this, Message]() {
				CreatePeerConnection(Message.TurnServer.Host, Message.TurnServer.Username, Message.TurnServer.Password, Message.TurnServer.Port);
			}));
		}
	}
	else if (Message.Type.Equals(SIGNALING_TYPE_SDP))
	{
		/*
		* There is SDP info from remote peer that will be apply to juice instance
		* https://tools.ietf.org/id/draft-nandakumar-rtcweb-sdp-01.html
		*/
		const FString Sdp = Message.Data;		
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
					FAccelByteSignalingMessage JsonCandidate;
					while (CandidateQueue.Dequeue(JsonCandidate))
					{
						HandleMessage(JsonCandidate);
					}
				}
				else
				{
					UE_LOG_ABNET(Error, TEXT("Failed gathering juice candidate"));
					OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::JuiceGatherFailed);
				}
			}));
		}
		else
		{
			// Queue it and apply when local description ready
			SdpQueue.Enqueue(Message);
		}		
	}
	else if (Message.Type.Equals(SIGNALING_TYPE_CANDIDATE))
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
			const FString Candidate = Message.Data;
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
			CandidateQueue.Enqueue(Message);
		}
	}
	else if (Message.Type.Equals(TEXT("done")))
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
		FAccelByteSignalingMessage Message;
		Message.Type = SIGNALING_TYPE_SDP;
		Message.Channel = Channel;
		Message.Data = FString(Buffer);
		check(Signaling != nullptr);
		Signaling->SendMessage(PeerId, Message);
	}
	else
	{
		UE_LOG_ABNET(Error, TEXT("Failed getting juice local description"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::JuiceGetLocalDescriptionFailed);
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

#if PLATFORM_MAC
		// mac issue, need to be called from game thread
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnICEDataChannelConnectedDelegate.ExecuteIfBound(PeerChannel, GetP2PConnectionType());
		});
#else		
		OnICEDataChannelConnectedDelegate.ExecuteIfBound(PeerChannel, GetP2PConnectionType());
#endif
	}
	else if(State == JUICE_STATE_FAILED)
	{
		UE_LOG_ABNET(Error, TEXT("Juice connection failed"));
		OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::JuiceConnectionFailed);
	}
	else if(State == JUICE_STATE_DISCONNECTED)
	{
		OnICEDataChannelClosedDelegate.ExecuteIfBound(PeerChannel);
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
	FAccelByteSignalingMessage Message;
	Message.Type = SIGNALING_TYPE_CANDIDATE;
	Message.Channel = Channel;
	Message.Data = Candidate;
	Signaling->SendMessage(PeerId, Message);
}

void AccelByteJuice::JuiceGatheringDone()
{
	UE_LOG_ABNET(Log, TEXT("Juice Gathering Done"));
	FAccelByteSignalingMessage Message;
	Message.Type = SIGNALING_TYPE_DONE;
	Message.Channel = Channel;
	Signaling->SendMessage(PeerId, Message);
}

void AccelByteJuice::JuiceDataRecv(const char* data, size_t size)
{
	check(!PeerId.IsEmpty());
	OnICEDataReadyDelegate.ExecuteIfBound(PeerId, Channel, (uint8*)data, size);
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
			OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::HostResponseTimeout);
			return false;
		}

		switch (PeerStatus)
		{
			case EAccelBytePeerStatus::WaitingReply:
				return true;

			case EAccelBytePeerStatus::Hosting:
				{
					FAccelByteSignalingMessage Message;
					Message.Type = SIGNALING_TYPE_ICE;
					Message.Channel = Channel;
					Message.TurnServer.Host = SelectedTurnServerCred.Ip;
					Message.TurnServer.Port = SelectedTurnServerCred.Port;
					Message.TurnServer.Username = SelectedTurnServerCred.Username;
					Message.TurnServer.Password = SelectedTurnServerCred.Password;
					Message.Data = SIGNALING_OFFER;
					Signaling->SendMessage(PeerId, Message);
					CreatePeerConnection(SelectedTurnServerCred.Ip, SelectedTurnServerCred.Username,
						SelectedTurnServerCred.Password, SelectedTurnServerCred.Port);

					break;
				}

			case EAccelBytePeerStatus::NotHosting:
				{
					UE_LOG_ABNET(Error, TEXT("Peer is not hosting"));
					OnICEDataChannelConnectionErrorDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::PeerIsNotHosting);
					break;
				}
		}

		bIsCheckingHost = false;
	}

	return false;
}

void AccelByteJuice::UpdatePeerStatus(bool InStatus)
{
	PeerStatus = InStatus ? EAccelBytePeerStatus::Hosting : EAccelBytePeerStatus::NotHosting;
}

#endif
