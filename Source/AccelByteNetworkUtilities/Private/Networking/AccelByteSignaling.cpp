// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteSignaling.h"
#include "Misc/Base64.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"


AccelByteSignaling::AccelByteSignaling(AccelByte::FApiClientPtr InApiClientPtr)
	: bIsInitialized { false }
	, ApiClientWPtr { InApiClientPtr }
{
}

void AccelByteSignaling::Init()
{
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		return;
	}

	auto Delegate = AccelByte::Api::Lobby::FSignalingP2P::CreateSP(SharedThis(this), &AccelByteSignaling::OnSignalingMessage);
	ApiClientPtr->Lobby.SetSignalingP2PDelegate(Delegate);
	bIsInitialized = true;
}

bool AccelByteSignaling::IsConnected() const
{
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		return false;
	}

	return bIsInitialized && ApiClientPtr->Lobby.IsConnected();
}

void AccelByteSignaling::Connect()
{	
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		return;
	}

	ApiClientPtr->Lobby.Connect();
}

void AccelByteSignaling::SendMessage(const FString& PeerId, const FAccelByteSignalingMessage& Message)
{
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		UE_LOG_ABSIGNALING(Error, TEXT("unable to send signaling message due to invalid ApiClient"));
		return;
	}

	FString StringMessage;
	if(FJsonObjectConverter::UStructToJsonObjectString(Message, StringMessage))
	{
		ApiClientPtr->Lobby.SendSignalingMessage(PeerId, FBase64::Encode(StringMessage));
	}
	else
	{
		UE_LOG_ABSIGNALING(Error, TEXT("unable to convert signaling message to json string"));
	}
}

void AccelByteSignaling::OnSignalingMessage(const FString& PeerId, const FString& Message)
{
	OnWebRtcSignalingMessageDelegate.ExecuteIfBound(PeerId, Message);
}
