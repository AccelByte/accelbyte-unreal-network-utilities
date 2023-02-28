// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#ifdef LIBJUICE

#include "AccelByteICEBase.h"
#include "juice/juice.h"
#include "Dom/JsonObject.h"
#include "Containers/Queue.h"
#include "Models/AccelByteTurnModels.h"

using namespace AccelByte::NetworkUtilities;

/*
 * Currently max char host name, username, and password is 64.
 * The example host will be: us-turn1.accelbyte.net, eu-turn2.accelbyte.net.
 */
#define TURN_MAX_LENGTH 64

/*
 * Helper to store TURN config server that handle its own memory
 */
struct TurnConfigHelper
{
	char Host[TURN_MAX_LENGTH];
	char Username[TURN_MAX_LENGTH];
	char Password[TURN_MAX_LENGTH];
};

/*
 * AccelByteJuice implement AccelByteICEBase using third party library LibJuice for ICE connection
 */

class AccelByteJuice: public AccelByteICEBase
{
public:
	AccelByteJuice(const FString &PeerId);

	/**
	* @brief Process signaling message
	*
	* @param Message from signaling service (AccelByte Lobby)
	*/
	virtual void OnSignalingMessage(const FString& Message) override;

	/**
	* @brief Request connect to PeerId
	*/
	virtual bool RequestConnect(const FString &ServerUrl, int ServerPort, const FString &Username, const FString &Password) override;

	/**
	* @brief Send data to connected peer data channel
	*
	* @param Data to sent
	* @param Count of the data to be sent
	* @param BytesSent notify byte sent.
	*
	* @return true when success
	*/
	virtual bool Send(const uint8* Data, int32 Count, int32& BytesSent) override;

	/**
	* @brief Disconnect peer connection
	*
	*/
	virtual void ClosePeerConnection() override;

	/**
	* @brief Check if peer instance requirement met
	*
	* @return true if requirement met
	*/
	virtual bool IsPeerReady() const override;

private:
	// instance of the Juice agent, will handle the ICE connection
	juice_agent_t *JuiceAgent = nullptr;
	// config for the Juice instance
	juice_config_t JuiceConfig;

	bool bIsForceIceUsingRelay = false;

	FCriticalSection DescriptionReadyMutex;
	/*
	 * flag when the juice local description is ready or not. Remote description will not apply when local description
	 * is not ready yet
	 */
	bool bIsDescriptionReady = false;

	/**
	 * Stores host check timeout in seconds, this value is from DefaultEngine.ini
	 * [AccelByteNetworkUtilities] HostCheckTimeout
	 * Default to 10 seconds if config not found
	 */
	int32 HostCheckTimeout = 10;

	FAccelByteModelsTurnServerCredential SelectedTurnServerCred;

	EAccelBytePeerStatus PeerStatus = EAccelBytePeerStatus::NotHosting;

	FDateTime InitiateConnectionTime;

	/*
	 * flag to indicate if still waiting for host to reply
	 */
	bool bIsCheckingHost = false;

	/*
	 * Will save the remote Juice SDP when bIsDescriptionReady still false. And apply it when bIsDescriptionReady is true
	 */
	TQueue<TSharedPtr<FJsonObject>> SdpQueue;

	/*
	* Will save the remote Juice Candidate when bIsDescriptionReady still false. And apply it when bIsDescriptionReady is true
	*/
	TQueue<TSharedPtr<FJsonObject>> CandidateQueue;

	/*
	 * This variable is to save the temporary value from FString
	 */
	TurnConfigHelper ConfigHelper;

	/*
	 * Store the latest juice state
	 */
	juice_state_t LastJuiceState = JUICE_STATE_DISCONNECTED;

	/**
	 * @brief Create peer connection using defined Turn server
	 *
	 * @param Host address of the turn server
	 * @param Username of the turn server
	 * @param Password of the turn server
	 * @param Port of the turn server
	 */
	void CreatePeerConnection(const FString& Host, const FString &Username, const FString &Password, int Port);

	// Setup all callback for juice library
	void SetupCallback();

	// Handle signaling message
	void HandleMessage(TSharedPtr<FJsonObject> Json);

	//Setup local description
	void SetupLocalDescription();

	// juice callback
	void JuiceStateChanged(juice_state_t State);
	void JuiceCandidate(const char *Candidate);
	void JuiceGatheringDone();
	void JuiceDataRecv(const char *data, size_t size);

	/**
	 * @brief Return the connection type based on selected remote candidate
	 */
	EP2PConnectionType GetP2PConnectionType() const;

	/**
	 * @brief Ticker for checking if peer is still hosting game or not before creating connection and trigger timeout
	 * if there is no response from host after INITIATE_CONN_TIMEOUT_S. This ticker will shutdown after receiving
	 * reply from host or after timeout
	 *
	 * @param DeltaTime of the loop
	 */
	bool Tick(float DeltaTime);

	/**
	 * @brief Update peer status based on message received from signaling
	 *
	 * @param Message message string from signaling
	 */
	void UpdatePeerStatus(const FString &Message);
};

#endif