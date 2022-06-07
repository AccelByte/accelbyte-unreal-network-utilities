// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteSignaling.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

AccelByteSignaling::AccelByteSignaling(AccelByte::FApiClientPtr InApiClient):ApiClientPtr(InApiClient)
{
}

void AccelByteSignaling::Init()
{
	auto Delegate = AccelByte::Api::Lobby::FSignalingP2P::CreateRaw(this, &AccelByteSignaling::OnSignalingMessage);
	ApiClientPtr->Lobby.SetSignalingP2PDelegate(Delegate);
}

bool AccelByteSignaling::IsConnected() const
{
	return ApiClientPtr->Lobby.IsConnected();
}

void AccelByteSignaling::Connect()
{	
	ApiClientPtr->Lobby.Connect();
}

void AccelByteSignaling::SendMessage(const FString& PeerId, const FString& Message)
{
	ApiClientPtr->Lobby.SendSignalingMessage(PeerId, Message);
}

void AccelByteSignaling::OnSignalingMessage(const FString& PeerId, const FString& Message)
{
	OnWebRtcSignalingMessageDelegate.ExecuteIfBound(PeerId, Message);
}
