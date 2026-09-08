// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteSignalingModels.generated.h"

USTRUCT(BlueprintType)
struct FAccelByteSignalingTurnServer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FString Host {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	int32 Port {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FString Username {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FString Password {};
};

USTRUCT(BlueprintType)
struct FAccelByteSignalingMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FString Type {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	int32 Channel {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FString Data {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AccelByte | NetworkUtilities")
	FAccelByteSignalingTurnServer TurnServer {};
};