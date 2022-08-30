// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "IpAddressAccelByte.h"
#include "Containers/UnrealString.h"
#include "AccelByteNetworkUtilitiesConstant.h"
#include "OnlineSubsystemTypes.h"
#include "AccelByteNetworkUtilitiesLog.h"

FInternetAddrAccelByte::FInternetAddrAccelByte(const FInternetAddrAccelByte& Src):NetId(Src.NetId) 
{
}

FInternetAddrAccelByte::FInternetAddrAccelByte() 
{
}

FInternetAddrAccelByte::FInternetAddrAccelByte(const FUniqueNetId& InSteamId) 
{
}

TArray<uint8> FInternetAddrAccelByte::GetRawIp() const 
{
	TArray<uint8> Array;
	FUniqueNetIdStringRef Temp = FUniqueNetIdString::Create(NetId, FName(TEXT("ACCELBYTE")));
	const uint8* It = Temp->GetBytes();
	while (Array.Num() < Temp->GetSize())
	{
		Array.Add(*It);
		++It;
	}
	return Array;
}

void FInternetAddrAccelByte::SetRawIp(const TArray<uint8>& RawAddr) 
{
	NetId = BytesToString(RawAddr.GetData(), RawAddr.Num());
}

void FInternetAddrAccelByte::SetIp(uint32 InAddr) 
{
	UE_LOG_ABNET(Warning, TEXT("FInternetAddrAccelByte::SetIp is not supported"));
}

void FInternetAddrAccelByte::SetIp(const TCHAR* InAddr, bool& bIsValid) 
{
	FString InURL(InAddr);
	bIsValid = InURL.StartsWith(ACCELBYTE_URL_PREFIX);
	InURL.RemoveFromStart(ACCELBYTE_URL_PREFIX);
	NetId = InURL;
}

void FInternetAddrAccelByte::GetIp(uint32& OutAddr) const 
{
	OutAddr = 0;
	UE_LOG_ABNET(Warning, TEXT("FInternetAddrAccelByte::GetIp is not supported and will set OutAddr to 0"));
}

void FInternetAddrAccelByte::GetPort(int32& OutPort) const 
{
	OutPort = ACCELBYTE_SOCKET_PORT;
}

int32 FInternetAddrAccelByte::GetPort() const 
{
	return ACCELBYTE_SOCKET_PORT;
}

FString FInternetAddrAccelByte::ToString(bool bAppendPort) const 
{
	return NetId;
}

bool FInternetAddrAccelByte::operator==(const FInternetAddr& Other) const 
{
	FInternetAddrAccelByte& OtherIp = (FInternetAddrAccelByte&)Other;
	return OtherIp.NetId == NetId;
}

bool FInternetAddrAccelByte::operator!=(const FInternetAddrAccelByte& Other) const 
{
	return !(FInternetAddrAccelByte::operator==(Other));
}

uint32 FInternetAddrAccelByte::GetTypeHash() const 
{
	return ::GetTypeHash(ToString(true));
}

bool FInternetAddrAccelByte::IsValid() const 
{
	return !NetId.IsEmpty();
}

FName FInternetAddrAccelByte::GetProtocolType() const 
{
	return ACCELBYTE_SUBSYSTEM;
}

TSharedRef<FInternetAddr> FInternetAddrAccelByte::Clone() const 
{
	TSharedRef<FInternetAddrAccelByte> Address = MakeShareable(new FInternetAddrAccelByte);
	Address->NetId = NetId;
	return Address;
}