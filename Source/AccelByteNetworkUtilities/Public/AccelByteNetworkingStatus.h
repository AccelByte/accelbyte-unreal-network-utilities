// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

enum EAccelByteP2PConnectionStatus
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

enum EAccelBytePeerStatus
{
	NotHosting,
	WaitingReply,
	Hosting
};
