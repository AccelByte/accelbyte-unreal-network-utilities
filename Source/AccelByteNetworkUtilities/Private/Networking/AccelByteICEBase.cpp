// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteICEBase.h"

#include "AccelByteNetworkUtilities.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"

AccelByteICEBase::AccelByteICEBase(const FString& PeerId): PeerChannel(PeerId)
{
	TTuple<FString, int32> Result = FAccelByteNetworkUtilitiesModule::ExtractPeerAndChannel(PeerId);
	this->PeerId = Result.Key;
	this->Channel = Result.Value;
}