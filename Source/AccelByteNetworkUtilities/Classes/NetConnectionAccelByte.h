// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "IpConnection.h"
#include "NetConnectionAccelByte.generated.h"

/**
* UIpConnectionAccelByte need to override the InitRemoteConnection and InitLocalConnection
* Need to disable the address resolution when the connection is Peer-to-peer (Nat punch connection)
*/

UCLASS(transient, config = Engine)
class UIpConnectionAccelByte : public UIpConnection
{
	GENERATED_BODY()

	bool bIsICEConnection = false;
	FString PeerId;

	//~ Begin UIpConnection Interface
	virtual void InitRemoteConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void CleanUp() override;
	//~ End UIpConnection Interface

	friend class FSocketSubsystemAccelByte;
};
