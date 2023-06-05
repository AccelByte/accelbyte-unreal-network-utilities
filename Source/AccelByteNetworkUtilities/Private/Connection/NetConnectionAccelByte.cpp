// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NetConnectionAccelByte.h"

#include "AccelByteNetworkUtilities.h"
#include "AccelByteNetworkUtilitiesConstant.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "SocketAccelByte.h"
#include "NetDriverAccelByte.h"
#include "Networking/AccelByteNetworkManager.h"

void UIpConnectionAccelByte::InitRemoteConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL,
                                                  const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	bIsICEConnection = static_cast<UIpNetDriverAccelByte*>(InDriver)->bICEConnectionsEnabled;
	if (bIsICEConnection)
	{
		PeerId = InRemoteAddr.ToString(true);
		/*
		 * Disabled because the IP address passed here is not real IP Address, but AccelByteIpAddress
		 * it has its own format of IP, please check IPAddressAccelByte
		 */
		DisableAddressResolution();
	}
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
}

void UIpConnectionAccelByte::InitLocalConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL,
	EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	bIsICEConnection = InURL.Host.StartsWith(ACCELBYTE_URL_PREFIX);
	if (bIsICEConnection)
	{
		PeerId = FString::Printf(TEXT("%s:%d"), *InURL.Host, InURL.Port);
		PeerId.RemoveFromStart(ACCELBYTE_URL_PREFIX);

		FString ParamValue;
		bool bNonSeamlessTravelUseNewConnection = false;
		GConfig->GetBool(TEXT("AccelByteNetworkUtilities"), TEXT("bNonSeamlessTravelUseNewConnection"), bNonSeamlessTravelUseNewConnection, GEngineIni);

		if(bNonSeamlessTravelUseNewConnection)
		{
			if(InURL.HasOption(TEXT("listen")))
			{
				// this will be non seamless server travel
				UIpNetDriverAccelByte *AccelbyteNetDriver = dynamic_cast<UIpNetDriverAccelByte*>(InDriver);
				if(AccelbyteNetDriver != nullptr)
				{
					DisableAddressResolution();
					FSocketAccelByte *SocketAccelbyte = static_cast<FSocketAccelByte*>(AccelbyteNetDriver->GetSocket());
					const int32 NewPort = FMath::RandRange(1024, 4096);
					// let's request new P2P connection
					PeerId = FString::Printf(TEXT("%s:%d"), *InURL.Host, NewPort);
					PeerId.RemoveFromStart(ACCELBYTE_URL_PREFIX);
					AccelByteNetworkManager::Instance().RequestConnect(PeerId);
					SocketAccelbyte->SetRedirectChannel(NewPort);
				}
				UE_LOG_ABNET(Log, TEXT("Non Seamless travel: %s"), *InURL.ToString(true));
			}
		}
		AccelByteNetworkManager::Instance().TouchConnection(PeerId);
		
		/*
		* Disabled because the IP address passed here is not real IP Address, but AccelByteIpAddress
		* it has its own format of IP, please check IPAddressAccelByte
		*/
		DisableAddressResolution();
	}
	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
}

void UIpConnectionAccelByte::CleanUp()
{
	if(bIsICEConnection)
	{
		// destroy the P2P connection here
		AccelByteNetworkManager::Instance().ScheduleClose(PeerId);
	}
	Super::CleanUp();
}
