// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

/*
 * The name of the AccelByte Socket Subsystem
 */
#define ACCELBYTE_SUBSYSTEM FName(TEXT("ACCELBYTE"))

/*
 * Prefix of the custom AccelByte IP Address
 * The format of IP : ACCELBYTE_URL_PREFIX.[user_id]:[channel]
 * Example value: accelbyte.12345678abcd:3314
 */
#define ACCELBYTE_URL_PREFIX TEXT("accelbyte.")

/*
 * The name of the AccelByte Socket
 */
#define SOCKET_ACCELBYTE_NAME TEXT("SocketAccelByte")

/*
 * The socket accelbyte fname as the identifier of the accelbyte socket instance
 */
#define SOCKET_ACCELBYTE_FNAME FName(TEXT("SocketAccelByte"))