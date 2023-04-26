// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

/*
 * define constant for check signaling prior to initiate connection
 */

#define SIGNALING_TYPE_ICE TEXT("ice")
#define SIGNALING_TYPE_SDP TEXT("sdp")
#define SIGNALING_TYPE_CANDIDATE TEXT("candidate")
#define SIGNALING_TYPE_DONE TEXT("done")
#define SIGNALING_TYPE_CHECK_HOST TEXT("hosting")
#define SIGNALING_TYPE_CHECK_HOST_REPLY TEXT("hostingreply")

#define HOST_CHECK_MESSAGE_HOSTING TEXT("hosting")
#define HOST_CHECK_MESSAGE_NOT_HOSTING TEXT("not_hosting")

#define SIGNALING_OFFER TEXT("offer")

