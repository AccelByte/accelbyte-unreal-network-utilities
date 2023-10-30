// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NetDriverAccelByte.h"
#include "AccelByteNetworkUtilitiesConstant.h"
#include "SocketAccelByte.h"
#include "Misc/CommandLine.h"
#include "EngineLogs.h"

void UIpNetDriverAccelByte::PostInitProperties() 
{
	Super::PostInitProperties();
}

ISocketSubsystem* UIpNetDriverAccelByte::GetSocketSubsystem() 
{
	return ISocketSubsystem::Get(bICEConnectionsEnabled ? ACCELBYTE_SUBSYSTEM : PLATFORM_SOCKETSUBSYSTEM);
}

bool UIpNetDriverAccelByte::IsAvailable() const
{
	ISocketSubsystem* Sockets = ISocketSubsystem::Get(ACCELBYTE_SUBSYSTEM);
	if (Sockets)
	{
		return true;
	}

	return false;
}

bool UIpNetDriverAccelByte::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!bICEConnectionsEnabled)
	{
		return Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
	}

	/**
	 * Skip the UIpNetDriver::InitBase because UIPNetDriver does not know AccelByteSocket
	 * so we handle the creation and setup below
	 */
	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (SocketSubsystem == nullptr)
	{
		UE_LOG(LogNet, Warning, TEXT("Unable to find socket subsystem"));
		Error = TEXT("Unable to find socket subsystem");
		return false;
	}

	if (GetSocket() == nullptr)
	{
		Error = TEXT("Error get socket AccelByte");
		return false;
	}

	LocalAddr = SocketSubsystem->GetLocalBindAddr(*GLog);

	LocalAddr->SetPort(URL.Port);

	const int32 BoundPort = SocketSubsystem->BindNextPort(GetSocket(), *LocalAddr, MaxPortCountToTry + 1, 1);
	UE_LOG(LogNet, Display, TEXT("%s bound to port %d"), *GetName(), BoundPort);

	return true;
}

bool UIpNetDriverAccelByte::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	ISocketSubsystem* SocketAccelByte = ISocketSubsystem::Get(ACCELBYTE_SUBSYSTEM);
	if (SocketAccelByte)
	{
		if (ConnectURL.Host.StartsWith(ACCELBYTE_URL_PREFIX))
		{
			// use AccelByte socket when the connection is ICE connection
			TSharedPtr<FSocket> NewSocket = MakeShareable(SocketAccelByte->CreateSocket(SOCKET_ACCELBYTE_FNAME, TEXT("Unreal client (AccelByte)"), ACCELBYTE_SUBSYSTEM));
			SetSocketAndLocalAddress(NewSocket);
		}
		else
		{
			bICEConnectionsEnabled = false;
		}
	}
	return Super::InitConnect(InNotify, ConnectURL, Error);
}

bool UIpNetDriverAccelByte::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
	ISocketSubsystem* SocketAccelByte = ISocketSubsystem::Get(ACCELBYTE_SUBSYSTEM);
	FString ParamValue;
	bool bIsICEEnabled = false;
	if (FParse::Value(FCommandLine::Get(), TEXT("dsaccelbyteice"), ParamValue))
	{
		bIsICEEnabled = true;
	}
	if (SocketAccelByte && !ListenURL.HasOption(TEXT("bIsLanMatch")) && (!IsRunningDedicatedServer() || (IsRunningDedicatedServer() && bIsICEEnabled)))
	{
		// use AccelByte socket when the connection is ICE connection
		TSharedPtr<FSocket> NewSocket = MakeShareable(SocketAccelByte->CreateSocket(SOCKET_ACCELBYTE_FNAME, TEXT("Unreal server (AccelByte)"), ACCELBYTE_SUBSYSTEM));
		SetSocketAndLocalAddress(NewSocket);
	}
	else
	{
		bICEConnectionsEnabled = false;
	}

	return Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void UIpNetDriverAccelByte::Shutdown()
{
	Super::Shutdown();
}
