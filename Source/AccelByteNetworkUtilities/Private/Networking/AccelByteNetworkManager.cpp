// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteNetworkManager.h"
#include "HAL/UnrealMemory.h"
#include "Misc/Base64.h"
#include "AccelByteSignaling.h"
#include "AccelByteICEBase.h"
#include "AccelByteJuice.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "Api/AccelByteTurnManagerApi.h"
#include "AccelByteSignalingConstants.h"
#include "AccelByteSignalingModels.h"

using namespace AccelByte::NetworkUtilities;

struct ICEData
{
	FString PeerId;
	int32 Channel;
	TArray<uint8> Data;
	ICEData(const FString& PeerId, int32 Channel, const TArray<uint8>& Data) :PeerId(PeerId), Channel(Channel), Data(Data)
	{
	}
};

struct ICETimeLog
{
	FString PeerId;
	FDateTime DateTime;
	ICETimeLog(const FString &PeerId, const FDateTime &Date): PeerId(PeerId), DateTime(Date){}
};

AccelByteNetworkManager& AccelByteNetworkManager::Instance()
{
	static AccelByteNetworkManager AccelByteNetworkManagerInstance;
	return AccelByteNetworkManagerInstance;
}

bool AccelByteNetworkManager::RequestConnect(const FString& PeerChannel)
{
	if (PeerIdToICEConnectionMap.Contains(PeerChannel))
	{
		if(PeerIdToICEConnectionMap[PeerChannel]->IsConnected())
		{
			RTCConnected(PeerChannel);
			return true;
		}
		return false;
	}

	if(!CachedTurnServer.Ip.IsEmpty())
	{
		RequestConnectWithTurnServer(PeerChannel, CachedTurnServer);
		return true;
	}

	if(bIsUseTurnManager)
	{
		ApiClientPtr->TurnManager.GetClosestTurnServer(THandler<FAccelByteModelsTurnServer>::CreateLambda(
			[this, PeerChannel](const FAccelByteModelsTurnServer &Result)
				{
					RequestConnectWithTurnServer(PeerChannel, Result);
				}),
				FErrorHandler::CreateLambda([this, PeerChannel](int32, const FString &ErrorMessage)
				{
					UE_LOG_ABNET(Error, TEXT("Error getting turn server from turn manager: %s"), *ErrorMessage);
					OnWebRTCDataChannelConnectedDelegate.ExecuteIfBound(PeerChannel, false);
					OnWebRTCRequestConnectFinishedDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::FailedGettingTurnServer);
				}));
	}
	else
	{
		FString TurnHost;
		int TurnPort = 0;
		FString TurnUserName;
		FString TurnPassword;
		FString TurnError;
		if(!GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerUrl"), TurnHost, GEngineIni))
		{
			UE_LOG_ABNET(Error, TEXT("TurnServerUrl was missing in DefaultEngine.ini"));
		}

		if(!GConfig->GetInt(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerPort"), TurnPort, GEngineIni))
		{
			UE_LOG_ABNET(Error, TEXT("TurnServerPort was missing in DefaultEngine.ini"));
		}

		// Username password possible empty if no authentication required on TURN server
		GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerUsername"), TurnUserName, GEngineIni);
		GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerPassword"), TurnPassword, GEngineIni);
		TSharedPtr<AccelByteICEBase> Rtc = CreateNewConnection(PeerChannel);
		PeerIdToICEConnectionMap.Add(PeerChannel, Rtc);
		Rtc->RequestConnect(TurnHost, TurnPort, TurnUserName, TurnPassword);
	}
	return true;
}

void AccelByteNetworkManager::Setup(AccelByte::FApiClientPtr InApiClientPtr)
{
	ApiClientPtr = InApiClientPtr;
	Signaling.Reset();
	Signaling = MakeShared<AccelByteSignaling>(InApiClientPtr);
	Signaling->Init();

	if(!Signaling->IsConnected())
	{
		Signaling->Connect();
	}

	Signaling->SetOnWebRTCSignalingMessage(AccelByteSignalingBase::OnWebRTCSignalingMessage::CreateRaw(this, &AccelByteNetworkManager::OnSignalingMessage));

	GConfig->GetInt(TEXT("AccelByteNetworkUtilities"), TEXT("RequestConnectTimeout"), RequestConnectTimeout, GEngineIni);
	GConfig->GetInt(TEXT("AccelByteNetworkUtilities"), TEXT("ConnectionIdleTimeout"), ConnectionIdleTimeout, GEngineIni);
	GConfig->GetBool(TEXT("AccelByteNetworkUtilities"), TEXT("UseTurnManager"), bIsUseTurnManager, GEngineIni);

	// setup tick
	FTickerDelegate TickerDelegate = FTickerDelegate::CreateRaw(this, &AccelByteNetworkManager::Tick);
	FTickerAlias::GetCoreTicker().AddTicker(TickerDelegate, 0.5);
}

bool AccelByteNetworkManager::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FString& PeerChannel)
{
	if (!PeerIdToICEConnectionMap.Contains(PeerChannel))
	{
		return false;
	}
	TSharedPtr<AccelByteICEBase> Rtc = PeerIdToICEConnectionMap[PeerChannel];
	return Rtc->Send(Data, Count, BytesSent);
}

bool AccelByteNetworkManager::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FString &PeerId, int32 Channel)
{
	return SendTo(Data, Count, BytesSent, FString::Printf(TEXT("%s:%d"), *PeerId, Channel));
}

bool AccelByteNetworkManager::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FString& PeerId, int32 &Channel)
{
	bool bIsReadSuccess = false;
	if (!IsDataReadyToRead())
	{
		const bool bIsDequeueSuccess = QueueDatas.Dequeue(LastReadData);
		if (bIsDequeueSuccess)
		{
			Offset = 0;
		}
		else
		{
			BytesRead = 0;
			Offset = -1;
			return false;
		}
	}
	if(!LastReadData.IsValid())
	{
		Offset = -1;
		BytesRead = 0;
		return false;
	}
	const int Num = LastReadData->Data.Num();
	if (Num > 0)
	{
		bIsReadSuccess = true;
		PeerId = LastReadData->PeerId;
		Channel = LastReadData->Channel;
		const int32 LastOffset = Offset;
		const int32 RealNum = Num - Offset;
		if (BufferSize >= RealNum)
		{
			BytesRead = RealNum;
			Offset = -1;
		}
		else
		{
			Offset += BufferSize;
			BytesRead = BufferSize;
		}
		FMemory::Memcpy(Data, LastReadData->Data.GetData() + LastOffset, BytesRead);
	} else
	{
		Offset = -1;
		BytesRead = 0;
	}
	return bIsReadSuccess;
}

bool AccelByteNetworkManager::HasPendingData(uint32& PendingDataSize)
{
	if (LastReadData.IsValid())
	{
		PendingDataSize = LastReadData->Data.Num();
		return true;
	}
	TSharedPtr<ICEData> *Item = QueueDatas.Peek();
	if (Item != nullptr)
	{
		PendingDataSize = Item->Get()->Data.Num();
		return true;
	}
	PendingDataSize = 0;
	return false;
}

void AccelByteNetworkManager::ClosePeerConnection(const FString& PeerId)
{	
	if (PeerIdToICEConnectionMap.Contains(PeerId))
	{
		UE_LOG_ABNET(Log, TEXT("Closing ICE connection to: %s"), *PeerId);
		PeerIdToICEConnectionMap[PeerId]->ClosePeerConnection();
		PeerIdToICEConnectionMap.Remove(PeerId);
		OnWebRTCDataChannelClosedDelegate.ExecuteIfBound(PeerId);
	}
}

void AccelByteNetworkManager::CloseAllPeerConnections()
{
	TArray<TSharedPtr<AccelByteICEBase>> ToDestroy;
	for (const auto& pair : PeerIdToICEConnectionMap)
	{
		ToDestroy.Add(pair.Value);
	}
	for(const auto &Item: ToDestroy)
	{
		Item->ClosePeerConnection();
	}
	PeerIdToICEConnectionMap.Empty();
	OnWebRTCDataChannelClosedDelegate.ExecuteIfBound("");
}

void AccelByteNetworkManager::EnableHosting()
{
	bIsHosting = true;
}

void AccelByteNetworkManager::DisableHosting()
{
	bIsHosting = false;
}

void AccelByteNetworkManager::RequestConnectWithTurnServer(const FString& PeerChannel, const FAccelByteModelsTurnServer &TurnServer)
{
	UE_LOG_ABNET(Log, TEXT("Selected TURN server : %s:%d"), *TurnServer.Ip, TurnServer.Port);

	CachedTurnServer = TurnServer;
	SelectedTurnServerRegion = *TurnServer.Region;

	FString TurnSecret;
	FString TurnUsername;
	if(!GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerSecret"), TurnSecret, GEngineIni))
	{
		UE_LOG_ABNET(Error, TEXT("TurnServerSecret was missing in DefaultEngine.ini"));
	}
	if(!GConfig->GetString(TEXT("AccelByteNetworkUtilities"), TEXT("TurnServerUsername"), TurnUsername, GEngineIni))
	{
		TurnUsername = TEXT("accelbyte");
		UE_LOG_ABNET(Warning, TEXT("TurnServerUsername was missing in DefaultEngine.ini, using default username"));
	}

	if(!TurnSecret.IsEmpty())
	{
		UE_LOG_ABNET(Log, TEXT("TURN using static auth secret"));
		// This only used once to login to turn server. Once logged to turn, the lifetime does not take into account.
		// The turn session already handled automatically by the ICE library and turn server
		const int64 lifeTime = 5 * 60; // 5 minutes.
		const int64 curTime = TurnServer.Current_time + lifeTime;
		FString Username = FString::Printf(TEXT("%lld:%s"), curTime, *TurnUsername);
		uint8 DataOut[20]; //20 is sha1 length
		FSHA1::HMACBuffer(TCHAR_TO_ANSI(*TurnSecret), TurnSecret.Len(), TCHAR_TO_ANSI(*Username), Username.Len(), DataOut);
		FString Password = FBase64::Encode(DataOut, 20);

		TSharedPtr<AccelByteICEBase> Rtc = CreateNewConnection(PeerChannel);
		PeerIdToICEConnectionMap.Add(PeerChannel, Rtc);
		Rtc->RequestConnect(TurnServer.Ip, TurnServer.Port, Username, Password);
	} else
	{
		UE_LOG_ABNET(Log, TEXT("TURN using dynamic auth secret"));
		RequestCredentialAndConnect(PeerChannel, TurnServer);
	}
}

bool AccelByteNetworkManager::IsDataReadyToRead() const
{
	return Offset >= 0;
}

void AccelByteNetworkManager::OnSignalingMessage(const FString& PeerId, const FString& Message)
{
	FString Base64Decoded;
	FBase64::Decode(Message, Base64Decoded);
	UE_LOG_ABSIGNALING(Verbose, TEXT("Signaling message from: %s; Message: %s"), *PeerId, *Base64Decoded);
	FAccelByteSignalingMessage SignalingMessage;
	if(!FJsonObjectConverter::JsonObjectStringToUStruct(Base64Decoded, &SignalingMessage))
	{
		UE_LOG_ABSIGNALING(Error, TEXT("unable to convert json string to FAccelByteSignalingMessage"))
		return;
	}
	const FString PeerChannel = FString::Printf(TEXT("%s:%d"), *PeerId, SignalingMessage.Channel);
	
	if(SignalingMessage.Type.Equals(SIGNALING_TYPE_CHECK_HOST))
	{
		SignalingMessage.Type = SIGNALING_TYPE_CHECK_HOST_REPLY;
		SignalingMessage.Data = bIsHosting ? HOST_CHECK_MESSAGE_HOSTING : HOST_CHECK_MESSAGE_NOT_HOSTING;
		Signaling->SendMessage(PeerId, SignalingMessage);
		return;
	}
	
	TSharedPtr<AccelByteICEBase> Conn;
	if (!PeerIdToICEConnectionMap.Contains(PeerChannel))
	{
		Conn = CreateNewConnection(PeerChannel);
		PeerIdToICEConnectionMap.Add(PeerChannel, Conn);
	}
	else
	{
		Conn = PeerIdToICEConnectionMap[PeerChannel];
	}
	check(Conn.IsValid());
	Conn->OnSignalingMessage(SignalingMessage);
}

void AccelByteNetworkManager::IncomingData(const FString& FromPeerId, int32 Channel, const uint8* Data, int32 Count)
{
	if(Count > 0)
	{
		QueueDatas.Enqueue(MakeShared<ICEData>(FromPeerId, Channel, TArray<uint8>((uint8*)Data, Count)));
		LastReceiveData.Enqueue(MakeShared<ICETimeLog>(FString::Printf(TEXT("%s:%d"), *FromPeerId, Channel), FDateTime::Now()));
	}
}

void AccelByteNetworkManager::RTCConnected(const FString& PeerChannel, const EP2PConnectionType& P2PConnectionType)
{
	UE_LOG_ABNET(Log, TEXT("ICE connected: %s"), *PeerChannel);
	FScopeLock ScopeLock(&LockObject);
	PeerRequestConnectTime.Remove(PeerChannel);
	OnWebRTCDataChannelConnectedDelegate.ExecuteIfBound(PeerChannel, true);
	OnWebRTCRequestConnectFinishedDelegate.ExecuteIfBound(PeerChannel, EAccelByteP2PConnectionStatus::Success);

	SendMetricData(P2PConnectionType);
}

void AccelByteNetworkManager::RTCClosed(const FString& PeerChannel)
{
	UE_LOG_ABNET(Log, TEXT("ICE closed: %s"), *PeerChannel);
	FScopeLock ScopeLock(&LockObject);
	PeerRequestConnectTime.Remove(PeerChannel);
	if (PeerIdToICEConnectionMap.Contains(PeerChannel))
	{
		ScheduleToDestroy.Enqueue(PeerChannel);
		OnWebRTCDataChannelClosedDelegate.ExecuteIfBound(PeerChannel);
	}
}

TSharedPtr<AccelByteICEBase> AccelByteNetworkManager::CreateNewConnection(const FString& PeerChannel)
{
#ifdef LIBJUICE
	TSharedPtr<AccelByteICEBase> Rtc = MakeShared<AccelByteJuice>(PeerChannel);
#else
	TSharedPtr<AccelByteICEBase> Rtc = MakeShared<AccelByteNullICEConnection>(PeerChannel);
#endif
	Rtc->SetSignaling(Signaling.Get());
	Rtc->SetOnICEDataChannelConnectedDelegate(AccelByteICEBase::OnICEDataChannelConnected::CreateRaw(this, &AccelByteNetworkManager::RTCConnected));
	Rtc->SetOnICEDataChannelClosedDelegate(AccelByteICEBase::OnICEDataChannelClosed::CreateRaw(this, &AccelByteNetworkManager::RTCClosed));
	Rtc->SetOnICEDataReadyDelegate(AccelByteICEBase::OnICEDataReady::CreateRaw(this, &AccelByteNetworkManager::IncomingData));
	Rtc->SetOnICEDataChannelConnectionErrorDelegate(AccelByteICEBase::OnICEDataChannelConnectionError::CreateRaw(this, &AccelByteNetworkManager::OnICEConnectionErrorCallback));
	Rtc->SetOnICEConnectionLostDelegate(AccelByteICEBase::OnICEConnectionLost::CreateRaw(this, &AccelByteNetworkManager::OnICEConnectionLostCallback));
	{
		FScopeLock ScopeLock(&LockObject);
		PeerRequestConnectTime.Add(PeerChannel, FDateTime::Now());
	}
	return Rtc;
}

bool AccelByteNetworkManager::Tick(float /*DeltaTime*/)
{
	FDateTime now = FDateTime::Now();
	TArray<FString> ToRemove;
	{
		FScopeLock ScopeLock(&LockObject);
		for(auto &Elem: PeerRequestConnectTime)
		{
			// 30 seconds for connection timeout
			if(now.ToUnixTimestamp() - Elem.Value.ToUnixTimestamp() > RequestConnectTimeout)
			{
				if(PeerRequestConnectTime.Contains(Elem.Key))
				{
					ToRemove.Add(Elem.Key);
				}
			}
		}
		for (int i = ToRemove.Num() - 1; i >= 0; i--) {
			FString key = ToRemove[i];

			PeerRequestConnectTime.Remove(key);
			if (PeerIdToICEConnectionMap.Contains(key))
			{
				PeerIdToICEConnectionMap[key]->ClosePeerConnection();
				PeerIdToICEConnectionMap.Remove(key);
				OnWebRTCDataChannelClosedDelegate.ExecuteIfBound(key);
			}
		}
	}
	
	ToRemove.Empty();
	TMap<FString, FDateTime> LastReceiveMap;
	TSharedPtr<ICETimeLog> LastReceiveDataItem;
	while(LastReceiveData.Dequeue(LastReceiveDataItem))
	{
		LastReceiveMap.Add(LastReceiveDataItem->PeerId, LastReceiveDataItem->DateTime);
	}
	for(auto &Elem : LastReceiveMap)
	{
		// close idle connection
		if(now.ToUnixTimestamp() - Elem.Value.ToUnixTimestamp() > ConnectionIdleTimeout)
		{
			ScheduleToDestroy.Enqueue(Elem.Key);
		}
	}
	
	FScopeLock ScopeLock(&LockObjectDisconnect);
	for(auto &Elem : LastTouchedMap)
	{
		// timeout for 20 seconds
		if(now.ToUnixTimestamp() - Elem.Value.ToUnixTimestamp() > 20)
		{
			ToRemove.Add(Elem.Key);
		}
	}
	for (int i = ToRemove.Num() - 1; i >= 0; i--)
	{
		LastTouchedMap.Remove(ToRemove[i]);
	}

	for(auto &Elem : ScheduledCloseMap)
	{
		// timeout for 20 seconds
		if(now.ToUnixTimestamp() - Elem.Value.ToUnixTimestamp() > 20)
		{
			ScheduleToDestroy.Enqueue(Elem.Key);
		}
	}

	FString PeerToDestroy;
	while(ScheduleToDestroy.Dequeue(PeerToDestroy))
	{
		UE_LOG_ABNET(Log, TEXT("CLOSING: %s"), *PeerToDestroy);
		ScheduledCloseMap.Remove(PeerToDestroy);
		LastTouchedMap.Remove(PeerToDestroy);
		ClosePeerConnection(PeerToDestroy);
	}
	
	return true;
}

void AccelByteNetworkManager::RequestCredentialAndConnect(const FString& PeerId, const FAccelByteModelsTurnServer& SelectedTurnServer)
{
	ApiClientPtr->TurnManager.GetTurnCredential(SelectedTurnServer.Region, SelectedTurnServer.Ip, SelectedTurnServer.Port,
		THandler<FAccelByteModelsTurnServerCredential>::CreateLambda([this, PeerId](const FAccelByteModelsTurnServerCredential &Credential)
		{
			TSharedPtr<AccelByteICEBase> Rtc = CreateNewConnection(PeerId);
			PeerIdToICEConnectionMap.Add(PeerId, Rtc);
			FString Password = Credential.Password;
			FString ParamValue;
			if(FParse::Value(FCommandLine::Get(), TEXT("juiceforcefailed"), ParamValue))
			{
				Password = TEXT("failedpassword");
			}
			Rtc->RequestConnect(Credential.Ip, Credential.Port, Credential.Username, Password);
		}), FErrorHandler::CreateLambda([this, PeerId](int32 Code, const FString &ErrorMessage)
		{
			UE_LOG_ABNET(Error, TEXT("Error getting turn server credential: %s"), *ErrorMessage);
			OnWebRTCDataChannelConnectedDelegate.ExecuteIfBound(PeerId, false);
			OnWebRTCRequestConnectFinishedDelegate.ExecuteIfBound(PeerId, EAccelByteP2PConnectionStatus::FailedGettingTurnServerCredential);
		}));
}

void AccelByteNetworkManager::OnICEConnectionErrorCallback(const FString& PeerId, const EAccelByteP2PConnectionStatus& Status)
{
	FScopeLock ScopeLock(&LockObject);
	OnWebRTCDataChannelConnectedDelegate.ExecuteIfBound(PeerId, false);
	OnWebRTCRequestConnectFinishedDelegate.ExecuteIfBound(PeerId, Status);

	// this need to be destroyed on next tick
	ScheduleToDestroy.Enqueue(PeerId);
	if(PeerRequestConnectTime.Contains(PeerId))
	{
		PeerRequestConnectTime.Remove(PeerId);
	}
}

void AccelByteNetworkManager::OnICEConnectionLostCallback(const FString& PeerChannel)
{
	UE_LOG_ABNET(Log, TEXT("Connection to %s lost, attempting to reconnect"), *PeerChannel);
	
	if (!PeerIdToICEConnectionMap.Contains(PeerChannel))
	{
		return;
	}

	const TSharedPtr<AccelByteICEBase> Connection = PeerIdToICEConnectionMap[PeerChannel];

	if (!bIsUseTurnManager)
	{
		Connection->ScheduleReconnection(FAccelByteModelsTurnServerCredential());
		return;
	}

	ApiClientPtr->TurnManager.GetClosestTurnServer(
		THandler<FAccelByteModelsTurnServer>::CreateLambda([this, Connection, PeerChannel](const FAccelByteModelsTurnServer &Result)
		{
			ApiClientPtr->TurnManager.GetTurnCredential(Result.Region, Result.Ip, Result.Port,
				THandler<FAccelByteModelsTurnServerCredential>::CreateLambda([this, Connection](const FAccelByteModelsTurnServerCredential &Credential)
				{
					Connection->ScheduleReconnection(Credential);
				}),
				FErrorHandler::CreateLambda([this, PeerChannel](int32 Code, const FString &ErrorMessage)
				{
					UE_LOG_ABNET(Error, TEXT("Error getting turn server credential when executing reconnection: %s"), *ErrorMessage);
					ClosePeerConnection(PeerChannel);
				}));
		}),
		FErrorHandler::CreateLambda([this, PeerChannel](int32, const FString &ErrorMessage)
		{
			UE_LOG_ABNET(Error, TEXT("Error getting turn server from turn manager when executing reconnection: %s"), *ErrorMessage);
			ClosePeerConnection(PeerChannel);
		}));
}

void AccelByteNetworkManager::SendMetricData(const EP2PConnectionType& P2PConnectionType)
{
	// Host doesn't know turn server region, SendMetricData only get called from client
	// therefore in host SelectedTurnServerRegion always empty and
	// P2PConnectionType empty means it already connected previously
	if (SelectedTurnServerRegion.IsEmpty() || P2PConnectionType == EP2PConnectionType::None)
	{
		return;
	}

	UE_LOG_ABNET(Log, TEXT("Selected turn server region: %s"), *SelectedTurnServerRegion);

	ApiClientPtr->TurnManager.SendMetric(SelectedTurnServerRegion, P2PConnectionType,
		FVoidHandler::CreateLambda([this]()
	{
		SelectedTurnServerRegion.Empty();
	}),
	FErrorHandler::CreateLambda([this](const int32 &ErrorCode, const FString &ErrorMessage)
	{
		SelectedTurnServerRegion.Empty();
		UE_LOG_ABNET(Error, TEXT("Error sending metric data: %d %s"), ErrorCode, *ErrorMessage);
	}));
}

void AccelByteNetworkManager::TouchConnection(const FString &PeerChannel)
{
	FScopeLock ScopeLock(&LockObjectDisconnect);
	if(ScheduledCloseMap.Contains(PeerChannel))
	{
		ScheduledCloseMap.Remove(PeerChannel);
	}
	LastTouchedMap.Add(PeerChannel, FDateTime::Now());
}

void AccelByteNetworkManager::ScheduleClose(const FString &PeerChannel)
{
	FScopeLock ScopeLock(&LockObjectDisconnect);
	if(LastTouchedMap.Contains(PeerChannel))
	{
		return;
	}
	ScheduledCloseMap.Add(PeerChannel, FDateTime::Now());
}

void AccelByteNetworkManager::SimulateNetworkSwitching()
{
	for (const auto& Connection : PeerIdToICEConnectionMap)
	{
		Connection.Value->SimulateNetworkSwitching();
	}
}

bool AccelByteNetworkManager::IsPeerConnected()
{
	if (PeerIdToICEConnectionMap.Num() <= 0)
	{
		return false;
	}

	for (const auto& Connection : PeerIdToICEConnectionMap)
	{
		if(Connection.Value->IsConnected())
		{
			continue;
		}

		return false;
	}

	return true;
}
