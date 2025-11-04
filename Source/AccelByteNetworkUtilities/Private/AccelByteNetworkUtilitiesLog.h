// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(AccelByteNetUtil, Log, All);

#define UE_LOG_ABNET(Verbosity, Format, ...) \
{ \
UE_LOG(AccelByteNetUtil, Verbosity, Format, ##__VA_ARGS__); \
}

DECLARE_LOG_CATEGORY_EXTERN(AccelByteSignalingMessage, Log, All);

#define UE_LOG_ABSIGNALING(Verbosity, Format, ...) \
{ \
UE_LOG(AccelByteSignalingMessage, Verbosity, Format, ##__VA_ARGS__); \
}