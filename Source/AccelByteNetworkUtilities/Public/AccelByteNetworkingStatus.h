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

/**
 * Measured quality of the selected P2P path. Populated only when latency-based selection is
 * enabled ([AccelByteNetworkUtilities] UseLatencyBasedSelection); fields are -1 / 0 otherwise.
 */
struct FAccelByteP2PConnectionStats
{
	int32 RttMs = -1;         // EWMA RTT in ms; -1 if unmeasured
	int32 LossPercent = -1;   // packet loss %; -1 if no probes were sent
	int32 ProbesSent = 0;     // measurement probes sent (sample size)
	int32 ProbesReceived = 0; // probe responses received

	bool HasMeasurement() const { return RttMs >= 0; }
};
