// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

namespace AccelByte
{
namespace NetworkUtilities
{
UENUM()
enum class EAccelByteP2PConnectionStatus
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
