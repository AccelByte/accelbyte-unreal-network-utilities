// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "UObject/ObjectMacros.h"

#include "CoreUObject.h"

namespace AccelByte
{
namespace NetworkUtilities
{
UENUM()
enum class EAccelByteP2PConnectionStatus : uint8
{
	Success,
	SignalingServerDisconnected,
	HostResponseTimeout,
	PeerIsNotHosting,
	JuiceGatherFailed,
	JuiceGetLocalDescriptionFailed,
	JuiceConnectionFailed,
	FailedGettingTurnServer,
	FailedGettingTurnServerCredential
};

UENUM()
enum class EAccelBytePeerStatus : uint8
{
	NotHosting,
	WaitingReply,
	Hosting
};
}
}
