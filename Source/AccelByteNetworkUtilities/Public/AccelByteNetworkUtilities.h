// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/AccelByteMultiRegistry.h"

class ACCELBYTENETWORKUTILITIES_API FAccelByteNetworkUtilitiesModule : public IModuleInterface
{
public:
	/**
	 * @brief Delegate when any ICE connection request done, possible connected or fail
	 *
	 * @param param1 peer id of the remote
	 * @param param2 status of the connection
	 */
	DECLARE_DELEGATE_TwoParams(OnICEConnected, const FString&, bool);
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	static FAccelByteNetworkUtilitiesModule& Get();
	
	/**
	 * @brief Setup the network manager before use
	 *
	 * @param InApiClientPtr ApiClient to use for the P2P connection
	 */
	void Setup(AccelByte::FApiClientPtr InApiClientPtr);

	/**
	 * @brief Request to connect peer-to-peer to peer id
	 *
	 * @param PeerId the id of remote host peer want to connect
	 */
	void RequestConnect(const FString &PeerId);

	/**
	 * @brief Register connection delegate
	 *
	 * @param Delegate to register
	 */
	void RegisterICEConnectedDelegate(OnICEConnected Delegate);

	/**
	 * @brief Close all peer-to-peer connection to this host
	 */
	void CloseAllICEConnection();

	/**
	* @brief To use P2P feature, need to setup the default socket subsystem to accelbyte.
	*/
	void RegisterDefaultSocketSubsystem();

	/**
	* @brief Use default platform socket subsystem and unregister accelbyte socket subsystem
	*/
	void UnregisterDefaultSocketSubsystem();

private:
	OnICEConnected OnICEConnectedDelegate;
};
