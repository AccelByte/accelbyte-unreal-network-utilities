// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/AccelByteApiClient.h"
#include "AccelByteNetworkingStatus.h"

class AccelByteICEBase;
class AccelByteSignalingBase;

struct ICEData;


/**
 * @brief Manager for ICE connection
 */
class AccelByteNetworkManager
{

public:
	/**
	 * @brief Delegate when any peer is connected
	 *
	 * @param String peer id of the remote connection
	 * @param bool true when connected and false connection failed
	 */
	DECLARE_DELEGATE_TwoParams(OnWebRTCDataChannelConnected, const FString&, bool);

	/**
	 * @brief Delegate when request connect done
	 *
	 * @param param1 peer id of the remote
	 * @param param2 enum indicating status of the connection
	 */
	DECLARE_DELEGATE_TwoParams(OnWebRTCRequestConnectFinished, const FString&, const EAccelByteP2PConnectionStatus&);

	/**
	 * @brief Delegate when any ICE connection closed
	 *
	 * @param String peer id of the remote connection
	 */
	DECLARE_DELEGATE_OneParam(OnWebRTCDataChannelClosed, const FString&);

	/**
	 * @brief Singleton instance of the manager
	 *
	*/
	static AccelByteNetworkManager& Instance();

	/**
	 * @brief Request connect to peer connection id
	 *
	 * @param PeerId id of the remote connection
	*/
	bool RequestConnect(const FString& PeerId);

	/**
	 * @brief Setup the network manager before use
	 *
	 * @param InApiClientPtr ApiClient to use for the P2P connection
	 */
	void Setup(AccelByte::FApiClientPtr InApiClientPtr);

	/**
	 * @brief Send data to peer id connection
	 *
	 * @param Data to send
	 * @param Count data length to send
	 * @param BytesSent actual data sent
	 * @param PeerId peer id of the remote
	*/
	bool SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FString &PeerId);

	/**
	 * @brief Receive data from any peer when any data available
	 *
	 * @param Data the storage of the data to store to
	 * @param BufferSize length of the data to read
	 * @param BytesRead actual data length read
	 * @param PeerId peer id where the data come from
	*/
	bool RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FString &PeerId);

	/**
	 * @brief Check if any data available to read
	 *
	 * @param PendingDataSize to store the length of available data to read
	 *
	 * @return true if any data available
	*/
	bool HasPendingData(uint32& PendingDataSize);

	/**
	 * @brief Close the peer connection of specific peer id
	 *
	 * @param PeerId peer id to close the connection
	*/
	void ClosePeerConnection(const FString& PeerId);

	/**
	 * @brief Close all peer connection
	*/
	void CloseAllPeerConnections();

	/**
	 * @brief set to indicate the player is hosting the game
	 */
	void EnableHosting();

	/**
	 * @brief set to indicate the player is no longer hosting the game
	 */
	void DisableHosting();

	OnWebRTCDataChannelConnected OnWebRTCDataChannelConnectedDelegate;
	OnWebRTCRequestConnectFinished OnWebRTCRequestConnectFinishedDelegate;
	OnWebRTCDataChannelClosed OnWebRTCDataChannelClosedDelegate;

private:
	// Api client to communicate with Accelbyte services
	AccelByte::FApiClientPtr ApiClientPtr;

	//Instance of the Signaling client
	TSharedPtr<AccelByteSignalingBase> Signaling;

	//Store all ICE connection by its peer id
	TMap<FString, TSharedPtr<AccelByteICEBase>> PeerIdToICEConnectionMap;

	//store the first time peer want to connect
	TMap<FString, FDateTime> PeerRequestConnectTime;

	//store ICE on next tick schedule
	TQueue<FString, EQueueMode::Mpsc> ScheduleToDestroy;

	//for mutext lock
	FCriticalSection LockObject;

	/*
	 * Store the peer data to queue because it has different read mechanism
	 * Most of ice library using callback when any data received
	 * But Unreal read it in every update
	 */
	TQueue<TSharedPtr<ICEData>, EQueueMode::Mpsc> QueueDatas;

	//Cached last read data from peer connection
	TSharedPtr<ICEData> LastReadData;

	//Offset of the data read from cached data (LastReadData)
	int Offset = -1;

	bool bIsRunning = false;

	// Store selected turn manager region for metric purpose, only updated by client
	FString SelectedTurnServerRegion;

	// Store current hosting state of network manager
	bool bIsHosting = false;

	/**
	 * @brief Check whatever there is data ready to read from cached data (LastReadData)
	 *
	 * @return true when any data ready to read
	 */
	bool IsDataReadyToRead() const;

	/**
	 * @brief Handle message when any signaling message from accelbyte lobby service
	 *
	 * @param PeerId Peer id where is this message coming from
	 * @param Message Signaling message in Json
	 */
	void OnSignalingMessage(const FString& PeerId, const FString& Message);

	/**
	 * @brief Incoming data from peer-to-peer remote connection
	 *
	 * @param FromPeerId where the data come from
	 * @param Data date received
	 * @param Count length of the data
	 */
	void IncomingData(const FString &FromPeerId, const uint8* Data, int32 Count);

	/**
	* @brief RTCConnected called when any peer-to-peer connection connected
	*
	* @param PeerId of the remote connection
	* @param P2PConnectionType of the connection (host, srflx, prflx, or host)
	*/
	void RTCConnected(const FString& PeerId, const EP2PConnectionType& P2PConnectionType = EP2PConnectionType::None);

	/**
	* @brief RTCClosed called when any peer-to-peer connection closed
	*
	* @param PeerId of the remote connection
	*/
	void RTCClosed(const FString& PeerId);

	/**
	* @brief CreateNewConnection create a AccelByteICEBase connection instance to the destination
	*
	* @param PeerId of the remote connection
	*/
	TSharedPtr<AccelByteICEBase> CreateNewConnection(const FString& PeerId);

	/**
	 * @brief Ticker for check if connection failed within some seconds
	 *
	 * @param DeltaTime of the loop
	 */
	bool Tick(float DeltaTime);

	/**
	 * @brief Request credential to turn manager
	 *
	 * @param PeerId user id of the peer want to connect to
	 * @param SelectedTurnServer the selected turn server to connect to
	 */
	void RequestCredentialAndConnect(const FString &PeerId, const FAccelByteModelsTurnServer &SelectedTurnServer);

	/**
	 * @brief Callback when any error ICE connection from ICEBase
	 *
	 * @param PeerId user id of the peer
	 * @param Status status of the connection
	 */
	void OnICEConnectionErrorCallback(const FString &PeerId, const EAccelByteP2PConnectionStatus &Status);

	/**
	 * @brief Send info about P2P connection type and selected turn server region to BE
	 *
	 * @param P2PConnectionType the type of P2P connection (host, srflx, prflx, relay)
	 */
	void SendMetricData(const EP2PConnectionType &P2PConnectionType);
};