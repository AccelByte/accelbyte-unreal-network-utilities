// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#ifdef WITH_ACCELBYTE_P2P_MOCK

#include "CoreMinimal.h"
#include "AccelByteNetworkingStatus.h"

/**
 * @brief Interface for mocking P2P connection behavior in tests
 *
 * Implement this interface in your test code to control P2P connection
 * outcomes without requiring actual network infrastructure.
 *
 * This interface is only available in test builds (WITH_ACCELBYTE_P2P_MOCK).
 * It provides a clean way to inject mock P2P behavior for testing OSS logic.
 *
 * USAGE EXAMPLE:
 *   class FMyP2PMock : public IAccelByteP2PConnectionMockHandler
 *   {
 *       virtual bool HandleConnectionRequest(...) override { ... }
 *   };
 *
 *   auto Mock = MakeShared<FMyP2PMock>();
 *   FAccelByteNetworkUtilitiesModule::Get().SetMockP2PHandler(Mock);
 */
class ACCELBYTETESTUTILITIES_API IAccelByteP2PConnectionMockHandler
{
public:
	virtual ~IAccelByteP2PConnectionMockHandler() = default;

	/**
	 * @brief Called when P2P connection is requested
	 *
	 * This method is called whenever OSS calls RequestConnect on the NetworkUtilities module.
	 * Your mock implementation can simulate success/failure and trigger the callback accordingly.
	 *
	 * @param PeerIdWithChannel The peer to connect to (format: "userId:channel", e.g. "abc123:2048")
	 * @param OnComplete Callback to trigger when mock connection completes
	 *                   Call with Connected for success, Failed for failure
	 *
	 * @return true if mock handles this request (prevents real P2P), false to use real P2P
	 */
	virtual bool HandleConnectionRequest(
		const FString& PeerIdWithChannel,
		TFunction<void(AccelByte::NetworkUtilities::EAccelByteP2PConnectionStatus)> OnComplete) = 0;

	/**
	 * @brief Called when closing all P2P connections
	 *
	 * Override this to track when the test closes P2P connections.
	 * Default implementation does nothing.
	 */
	virtual void HandleCloseAll() {}
};

#endif // WITH_ACCELBYTE_P2P_MOCK
