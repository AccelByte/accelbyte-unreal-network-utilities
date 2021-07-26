// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteNetworkManager.h"
#include "HAL/UnrealMemory.h"
#include "AccelByteSignaling.h"
#include "AccelByteICEBase.h"
#include "AccelByteJuice.h"

struct ICEData 
{
	FString PeerId;
	TArray<uint8> Data;
	ICEData(const FString& PeerId, const TArray<uint8>& Data) :PeerId(PeerId), Data(Data) 
	{
	}
};

AccelByteNetworkManager& AccelByteNetworkManager::Instance() 
{
	static AccelByteNetworkManager AccelByteNetworkManagerInstance;
	return AccelByteNetworkManagerInstance;
}

bool AccelByteNetworkManager::RequestConnect(const FString& PeerId) 
{
	if (PeerIdToICEConnectionMap.Contains(PeerId)) 
	{
		if(PeerIdToICEConnectionMap[PeerId]->IsConnected())
		{
			RTCConnected(PeerId);
			return true;
		}
		return false;
	}
	TSharedPtr<AccelByteICEBase> Rtc = CreateNewConnection(PeerId);
	PeerIdToICEConnectionMap.Add(PeerId, Rtc);
	return Rtc->RequestConnect();
}

bool AccelByteNetworkManager::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FString& PeerId) 
{
	if (!PeerIdToICEConnectionMap.Contains(PeerId)) 
	{
		return false;
	}
	TSharedPtr<AccelByteICEBase> Rtc = PeerIdToICEConnectionMap[PeerId];
	return Rtc->Send(Data, Count, BytesSent);
}

bool AccelByteNetworkManager::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FString& PeerId) 
{
	bool bIsReadSuccess = false;
	if (!IsDataReadyToRead()) 
	{
		bIsReadSuccess = QueueDatas.Dequeue(LastReadData);
		if (bIsReadSuccess) 
		{
			Offset = 0;
		}
		else 
		{
			return false;
		}
	}
	if(!LastReadData.IsValid())
	{
		return false;
	}
	const int Num = LastReadData->Data.Num();
	if (Num > 0) 
	{
		bIsReadSuccess = true;
		PeerId = LastReadData->PeerId;
		const int32 LastOffset = Offset;
		auto RealNum = Num - Offset;
		if (Offset + BufferSize >= Num) 
		{
			BytesRead = Num - Offset;
			Offset = -1;
		}
		else 
		{
			Offset += BufferSize;
			BytesRead = BufferSize;
		}
		FMemory::Memcpy(Data, LastReadData->Data.GetData() + LastOffset, BytesRead);
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
		PeerIdToICEConnectionMap[PeerId]->ClosePeerConnection();
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
}

void AccelByteNetworkManager::Run() 
{
	Signaling = MakeShared<AccelByteSignaling>();
	Signaling->Init();

	if(!Signaling->IsConnected())
	{
		Signaling->Connect();
	}
	
	Signaling->SetOnWebRTCSignalingMessage(AccelByteSignalingBase::OnWebRTCSignalingMessage::CreateRaw(this, &AccelByteNetworkManager::OnSignalingMessage));
}

bool AccelByteNetworkManager::IsDataReadyToRead() const 
{
	return Offset >= 0;
}

void AccelByteNetworkManager::OnSignalingMessage(const FString& PeerId, const FString& Message) 
{
	TSharedPtr<AccelByteICEBase> Conn; 
	if (!PeerIdToICEConnectionMap.Contains(PeerId)) 
	{
		Conn = CreateNewConnection(PeerId);
		PeerIdToICEConnectionMap.Add(PeerId, Conn);
	}
	else
	{
		Conn = PeerIdToICEConnectionMap[PeerId];
	}
	check(Conn.IsValid());
	Conn->OnSignalingMessage(Message);
}

void AccelByteNetworkManager::IncomingData(const FString& FromPeerId, const uint8* Data, int32 Count) 
{
	QueueDatas.Enqueue(MakeShared<ICEData>(FromPeerId, TArray<uint8>((uint8*)Data, Count)));
}

void AccelByteNetworkManager::RTCConnected(const FString& PeerId) 
{
	OnWebRTCDataChannelConnectedDelegate.ExecuteIfBound(PeerId, true);
}

void AccelByteNetworkManager::RTCClosed(const FString& PeerId) 
{
	if (PeerIdToICEConnectionMap.Contains(PeerId)) 
	{
		PeerIdToICEConnectionMap.Remove(PeerId);
		OnWebRTCDataChannelClosedDelegate.ExecuteIfBound(PeerId);
	}
}

TSharedPtr<AccelByteICEBase> AccelByteNetworkManager::CreateNewConnection(const FString& PeerId) 
{
#ifdef LIBJUICE
	TSharedPtr<AccelByteICEBase> Rtc = MakeShared<AccelByteJuice>(PeerId);
#else
	TSharedPtr<AccelByteICEBase> Rtc = MakeShared<AccelByteNullICEConnection>(PeerId);
#endif
	Rtc->SetSignaling(Signaling.Get());
	Rtc->SetOnICEDataChannelConnectedDelegate(AccelByteICEBase::OnICEDataChannelConnected::CreateRaw(this, &AccelByteNetworkManager::RTCConnected));
	Rtc->SetOnICEDataChannelClosedDelegate(AccelByteICEBase::OnICEDataChannelClosed::CreateRaw(this, &AccelByteNetworkManager::RTCClosed));
	Rtc->SetOnICEDataReadyDelegate(AccelByteICEBase::OnICEDataReady::CreateRaw(this, &AccelByteNetworkManager::IncomingData));
	return Rtc;
}