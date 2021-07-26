// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SocketSubsystemAccelByte.h"
#include "IpAddressAccelByte.h"
#include "SocketAccelByte.h"
#include "AccelByteNetworkUtilitiesConstant.h"

static FSocketSubsystemAccelByte* SocketSubsystemAccelByteInstance = nullptr;

FSocketSubsystemAccelByte* FSocketSubsystemAccelByte::Create()
{
	if (SocketSubsystemAccelByteInstance == nullptr) 
	{
		SocketSubsystemAccelByteInstance = new FSocketSubsystemAccelByte();
	}
	return SocketSubsystemAccelByteInstance;
}

bool FSocketSubsystemAccelByte::Init(FString& Error)
{	
	GConfig->GetBool(TEXT("AccelByteNetworkUtilities"), TEXT("bRequiresEncryptPackets"), bRequiresEncryptPackets, GEngineIni);
	return true;
}

void FSocketSubsystemAccelByte::Shutdown()
{
	//TODO: clean any ICE library code instance if any and make no leak on ICE connection
}

FSocket* FSocketSubsystemAccelByte::CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolName)
{
	FSocket* NewSocket = nullptr;
	if (SocketType == SOCKET_ACCELBYTE_FNAME)
	{
		NewSocket = new FSocketAccelByte();
	}
	else
	{
		ISocketSubsystem* PlatformSocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (PlatformSocketSub)
		{
			NewSocket = PlatformSocketSub->CreateSocket(SocketType, SocketDescription, ProtocolName);
		}
	}

	if (!NewSocket)
	{
		UE_LOG(LogSockets, Warning, TEXT("Failed to create socket %s [%s]"), *SocketType.ToString(), *SocketDescription);
	}

	return NewSocket;
}

void FSocketSubsystemAccelByte::DestroySocket(FSocket* Socket)
{
	if(Socket)
	{
		delete Socket;		
	}	
}

FAddressInfoResult FSocketSubsystemAccelByte::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName,
	EAddressInfoFlags QueryFlags, const FName ProtocolTypeName, ESocketType SocketType)
{
	FString RawAddress(HostName);	

	if (HostName != nullptr && RawAddress.RemoveFromStart(ACCELBYTE_URL_PREFIX))
	{
		FAddressInfoResult Result(HostName, ServiceName);

		FString PortString(ServiceName);
		Result.ReturnCode = SE_NO_ERROR;
		TSharedRef<FInternetAddrAccelByte> IpAddr = StaticCastSharedRef<FInternetAddrAccelByte>(CreateInternetAddr());
		IpAddr->NetId = RawAddress;
		Result.Results.Add(FAddressInfoResultData(IpAddr, 0, ACCELBYTE_SUBSYSTEM, SOCKTYPE_Unknown));
		return Result;
	}

	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType);
}

TSharedPtr<FInternetAddr> FSocketSubsystemAccelByte::GetAddressFromString(const FString& InAddress)
{
	FString RawAddress = InAddress;
	if(RawAddress.RemoveFromStart(ACCELBYTE_URL_PREFIX))
	{
		TSharedRef<FInternetAddrAccelByte> IpAddr = StaticCastSharedRef<FInternetAddrAccelByte>(CreateInternetAddr());
		IpAddr->NetId = RawAddress;
		return IpAddr;
	}
	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressFromString(InAddress);	
}

bool FSocketSubsystemAccelByte::RequiresChatDataBeSeparate()
{
	return false;
}

bool FSocketSubsystemAccelByte::RequiresEncryptedPackets()
{
	return bRequiresEncryptPackets;
}

bool FSocketSubsystemAccelByte::GetHostName(FString& HostName)
{
	//not supported on ICE connection
	return false;
}

bool FSocketSubsystemAccelByte::HasNetworkDevice()
{
	//not supported on ICE connection
	return false;
}

const TCHAR* FSocketSubsystemAccelByte::GetSocketAPIName() const
{
	return SOCKET_ACCELBYTE_NAME;
}

ESocketErrors FSocketSubsystemAccelByte::GetLastErrorCode()
{
	//TODO: check the ICE error connection
	return ESocketErrors::SE_NO_ERROR;
}

ESocketErrors FSocketSubsystemAccelByte::TranslateErrorCode(int32 Code)
{
	return ESocketErrors::SE_NO_ERROR;
}

bool FSocketSubsystemAccelByte::IsSocketWaitSupported() const
{
	//not supported on ICE connection
	return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemAccelByte::CreateInternetAddr()
{
	return MakeShareable(new FInternetAddrAccelByte());
}