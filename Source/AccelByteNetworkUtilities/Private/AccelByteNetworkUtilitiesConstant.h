// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

//PS5, XSX, and SWITCH only defined when they are enabled in unreal source
#ifndef PLATFORM_PS5
#define PLATFORM_PS5 0
#endif
#ifndef PLATFORM_XSX
#define PLATFORM_XSX 0
#endif
#ifndef PLATFORM_SWITCH
#define PLATFORM_SWITCH 0
#endif

/*
 * The name of the AccelByte Socket Subsystem
 */
#define ACCELBYTE_SUBSYSTEM FName(TEXT("ACCELBYTE"))

/*
 * Prefix of the custom AccelByte IP Address
 * The format of IP : ACCELBYTE_URL_PREFIX[user_id]
 * Example value: accelbyte.12345678abcd
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

/*
 * Dummy port that used as listening port of the listen server
 */
#define ACCELBYTE_SOCKET_PORT 11223;