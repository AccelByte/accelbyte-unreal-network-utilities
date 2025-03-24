// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteSignaling.h"
#include "Misc/Base64.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "Api/AccelByteLobbyApi.h"
#include "AccelByteNetworkUtilitiesLog.h"


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
		UE_LOG_ABSIGNALING(Error, TEXT("Unable to init Signaling class, Invalid API Client"));
		return;
	}

	auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
	if (Lobby.IsValid())
	{
		auto Delegate = AccelByte::Api::Lobby::FSignalingP2P::CreateSP(SharedThis(this), &AccelByteSignaling::OnSignalingMessage);
		Lobby->SetSignalingP2PDelegate(Delegate);
		bIsInitialized = true;
	}
	else
	{
		UE_LOG_ABSIGNALING(Warning, TEXT("Unable to init Signaling class, Invalid Lobby API from API Client"));
	}
}

bool AccelByteSignaling::IsConnected() const
{
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		UE_LOG_ABSIGNALING(Error, TEXT("Cannot check connection status, Invalid API Client"));
		return false;
	}

	bool bIsConnected = false;
	auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
	if (Lobby.IsValid())
	{
		bIsConnected = Lobby->IsConnected();
	}
	else
	{
		UE_LOG_ABSIGNALING(Warning, TEXT("Cannot check connection status, Invalid Lobby API from API Client"));
	}
	return bIsInitialized && bIsConnected;
}

void AccelByteSignaling::Connect()
{	
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		UE_LOG_ABSIGNALING(Error, TEXT("Cannot connect, Invalid API Client"));
		return;
	}

	auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
	if (Lobby.IsValid())
	{
		Lobby->Connect();
	}
	else
	{
		UE_LOG_ABSIGNALING(Warning, TEXT("Cannot connect, Invalid Lobby API from API Client"));
	}
}

void AccelByteSignaling::SendMessage(const FString& PeerId, const FAccelByteSignalingMessage& Message)
{
	auto ApiClientPtr = ApiClientWPtr.Pin();

	if (!ApiClientPtr.IsValid())
	{
		UE_LOG_ABSIGNALING(Error, TEXT("Unable to send signaling message, invalid ApiClient"));
		return;
	}

	FString StringMessage;
	if(FJsonObjectConverter::UStructToJsonObjectString(Message, StringMessage))
	{
		auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
		if (Lobby.IsValid())
		{
			Lobby->SendSignalingMessage(PeerId, FBase64::Encode(StringMessage));
		}
		else
		{
			UE_LOG_ABSIGNALING(Warning, TEXT("Unable to send signaling message, Invalid Lobby API from API Client"));
		}
	}
	else
	{
		UE_LOG_ABSIGNALING(Error, TEXT("Unable to convert signaling message to json string"));
	}
}

void AccelByteSignaling::OnSignalingMessage(const FString& PeerId, const FString& Message)
{
	OnWebRtcSignalingMessageDelegate.ExecuteIfBound(PeerId, Message);
}
