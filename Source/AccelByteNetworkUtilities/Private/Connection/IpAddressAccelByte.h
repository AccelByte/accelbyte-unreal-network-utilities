// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "IPAddress.h"
#include "AccelByteNetworkUtilitiesPackage.h"

/*
 * FInternetAddrAccelByte will hold the custom AccelByte IP.
 * The IP address used is accelbyte.[user_id], accelbyte. defined as ACCELBYTE_URL_PREFIX
 * example valid Peer-to-peer IP Address: accelbyte.a92d313c56a845569b97af0714f50618
 */

class FInternetAddrAccelByte : public FInternetAddr
{
PACKAGE_SCOPE:
	/*
	 * String that hold the accelbyte IP Address like accelbyte.a92d313c56a845569b97af0714f50618
	 */
	FString NetId;

	FInternetAddrAccelByte(const FInternetAddrAccelByte& Src);

public:
	FInternetAddrAccelByte();
	FInternetAddrAccelByte(const FUniqueNetId& InId);

	//~ Begin FInternetAddr Interface
	virtual TArray<uint8> GetRawIp() const override;
	virtual void SetRawIp(const TArray<uint8>& RawAddr) override;
	void SetIp(uint32 InAddr) override;
	void SetIp(const TCHAR* InAddr, bool& bIsValid) override;
	void GetIp(uint32& OutAddr) const override;
	void SetPort(int32 InPort) override {};
	void GetPort(int32& OutPort) const override;
	int32 GetPort() const override;
	void SetAnyAddress() override {};
	void SetBroadcastAddress() override {};
	void SetLoopbackAddress() override {};
	FString ToString(bool bAppendPort) const override;
	virtual bool operator==(const FInternetAddr& Other) const override;
	bool operator!=(const FInternetAddrAccelByte& Other) const;
	virtual uint32 GetTypeHash() const override;	
	virtual bool IsValid() const override;
	virtual FName GetProtocolType() const override;
	virtual TSharedRef<FInternetAddr> Clone() const override;
	//~ End FInternetAddr Interface
};
