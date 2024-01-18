// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/AccelByteMultiRegistry.h"
#include "AccelByteNetworkingStatus.h"

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

	/**
	 * @brief Delegate when request connect done
	 *
	 * @param param1 peer id of the remote
	 * @param param2 enum indicating status of the connection
	 */
	DECLARE_DELEGATE_TwoParams(OnICERequestConnectFinished, const FString&, const AccelByte::NetworkUtilities::EAccelByteP2PConnectionStatus&);

	/**
	* @brief Delegate when any ICE connection closed
	*
	* @param String peer id of the remote connection
	*/
	DECLARE_DELEGATE_OneParam(OnICEClosed, const FString&);

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
	 * @brief Register delegate after request connect finished
	 *
	 * @param Delegate to register
	 */
	void RegisterICERequestConnectFinishedDelegate(OnICERequestConnectFinished Delegate);

	/**
	 * @brief Register connection delegate
	 *
	 * @param Delegate to register
	 */
	void RegisterICEClosedDelegate(OnICEClosed Delegate);

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

	/**
	 * @brief set to indicate the player is hosting the game
	 */
	void EnableHosting();

	/**
	 * @brief set to indicate the player is no longer hosting the game
	 */
	void DisableHosting();
	
	/**
	 * [FOR GAUNTLET TEST ONLY] simulate network switching
	 */
	void SimulateNetworkSwitching();

	/**
	 * @brief [FOR GAUNTLET TEST ONLY] Check if all of the peer is connected or not
	 */
	bool IsAllPeerConnected();

	/*
	 * @brief Extract the peer id and the channel
	 *
	 * @param PeerId input of the userId:channel in string
	 */
	static TTuple<FString, int32> ExtractPeerAndChannel(const FString &PeerId);

	/*
	 * @brief Generate peer:channel string from peer id and channel
	 *
	 * @param PeerId The peer id in string (peer userid)
	 * @param Channel The channel in integer
	 */
	static FString GeneratePeerChannelString(const FString& PeerId, int32 Channel);
};
