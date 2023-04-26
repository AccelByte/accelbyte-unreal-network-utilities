// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NetConnectionAccelByte.h"
#include "AccelByteNetworkUtilitiesConstant.h"
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
		AccelByteNetworkManager::Instance().ClosePeerConnection(PeerId);
	}
	Super::CleanUp();
}
