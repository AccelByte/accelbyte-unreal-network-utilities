// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "IpNetDriver.h"
#include "NetDriverAccelByte.generated.h"

class Error;
class FNetworkNotify;

/*
 * This class handles choosing which the appropriate socket type to use. If ICE connections are enabled, FSocketAccelByte will be used.
 */

UCLASS(transient, config = Engine)
class UIpNetDriverAccelByte : public UIpNetDriver
{

	GENERATED_BODY()

	//~ Begin UNetDriver Interface.
	virtual void PostInitProperties() override;
	//~ End UNetDriver Interface.
	
	//~ Begin UIpNetDriver Interface.
	virtual class ISocketSubsystem* GetSocketSubsystem() override;
	virtual bool IsAvailable() const override;
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual void Shutdown() override;
	//~ End UIpNetDriver Interface

public:
	bool bICEConnectionsEnabled = true;
};