// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteSignalingBase.h"
#include "Core/AccelByteApiClient.h"

/*
 * AccelByteSignaling implementation of Signaling using AccelByte lobby backend
 */

class AccelByteSignaling : public AccelByteSignalingBase, public TSharedFromThis<AccelByteSignaling>
{
public:
	AccelByteSignaling(AccelByte::FApiClientPtr InApiClientPtr);
	virtual void Init() override;
	virtual bool IsConnected() const override;
	virtual void Connect() override;
	virtual void SendMessage(const FString &PeerId, const FAccelByteSignalingMessage &Message) override;

private:
	bool bIsInitialized{ false };
	AccelByte::FApiClientWPtr ApiClientWPtr{ nullptr };
	
	void OnSignalingMessage(const FString &PeerId, const FString &Message);
};
