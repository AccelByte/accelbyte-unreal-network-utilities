﻿// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#ifdef LIBJUICE

#include "AccelByteICEBase.h"
#include "juice/juice.h"
#include "Dom/JsonObject.h"
#include "Containers/Queue.h"
#include "Models/AccelByteTurnModels.h"
#include "Core/AccelByteDefines.h"

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

class AccelByteJuice : public AccelByteICEBase, public TSharedFromThis<AccelByteJuice, ESPMode::ThreadSafe>
{
public:
	AccelByteJuice(const FString &PeerId);
	virtual ~AccelByteJuice() override;

	/**
	* @brief Process signaling message
	*
	* @param Message from signaling service (AccelByte Lobby)
	*/
	virtual void OnSignalingMessage(const FAccelByteSignalingMessage &Message) override;

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

	/**
	 * Schedule reconnection to host in the future, this is to make sure libjuice agent in host already closed before
	 * initiating new connection.
	 *
	 * @param TurnServerCredential Turn server credential tobe used in reconnection
	 * @param InNextSeconds the reconnection will start after this seconds from the time peer inactive timeout
	 */
	virtual void ScheduleReconnection(const FAccelByteModelsTurnServerCredential& TurnServerCredential, float InNextSeconds = 1) override;

	
	/**
	 * [FOR GAUNTLET TEST ONLY] simulate network switching by ignoring update to LastPeerActivityTime.
	 * Later it will trigger connection lost and reconnection.
	 */
	virtual void SimulateNetworkSwitching() override;

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

	AccelByte::NetworkUtilities::EAccelBytePeerStatus PeerStatus = AccelByte::NetworkUtilities::EAccelBytePeerStatus::NotHosting;

	FDateTime InitiateConnectionTime;

	/*
	 * flag to indicate if still waiting for host to reply
	 */
	bool bIsCheckingHost = false;

	/*
	 * Will save the remote Juice SDP when bIsDescriptionReady still false. And apply it when bIsDescriptionReady is true
	 */
	TQueue<FAccelByteSignalingMessage> SdpQueue;

	/*
	* Will save the remote Juice Candidate when bIsDescriptionReady still false. And apply it when bIsDescriptionReady is true
	*/
	TQueue<FAccelByteSignalingMessage> CandidateQueue;

	/*
	 * This variable is to save the temporary value from FString
	 */
	TurnConfigHelper ConfigHelper;

	/*
	 * Store the latest juice state
	 */
	juice_state_t LastJuiceState = JUICE_STATE_DISCONNECTED;

	/*
	 * Flag indicating reconnection for replacing libjuice agent with a new one.
	 * When this flag set to true the on connected delegate shouldn't be triggered to avoid messing up OSS.
	*/
	bool bReconnection = false;

	// Ping payload
	static constexpr int32 PingDataLength = 1;
	static constexpr uint8 PingData[PingDataLength] = { 32 };

	// This time indicate the last time this agent received a data from its peer
	TAtomic<double> LastPeerActivityTime{0};

	// Interval for checking peer last activity ticker in seconds
	float PeerLastActivityTickerIntervalInSeconds = 1.0f;

	// Maximum peer inactive time, counted from the last time the agent received data.
	double PeerInactiveTimeoutInSeconds = 10.0;

	// Host initial check delegate and delegate handle
	FTickerDelegate HostInitialCheckDelegate;
	FDelegateHandleAlias HostInitialCheckDelegateHandle;

	// Peer last activity delegate and delegate handle
	FTickerDelegate PeerLastActivityTickerDelegate;
	FDelegateHandleAlias PeerLastActivityTickerDelegateHandle;

	// Schedule reconnection delegate and delegate handle
	FTickerDelegate ScheduleReconnectionDelegate;
	FDelegateHandleAlias ScheduleReconnectionDelegateHandle;

	bool bSimulateNetworkSwitchingEnabled = false; 

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
	void HandleMessage(const FAccelByteSignalingMessage &Message);

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
	bool HostInitialCheckTick(float DeltaTime);

	/**
	 * @brief Update peer status based on message received from signaling
	 *
	 * @param InStatus Peer status, true if hosting.
	 */
	void UpdatePeerStatus(bool InStatus);

	/**
	 * Update LastPeerActivityTime to current time
	 */
	void UpdatePeerLastActivity();

	/**
	 * Method attached to core ticker for initiating reconnection to host.
	 *
	 * @param DeltaTime elapsed delta time in seconds.
	 * @return return false because only used as one shot timer.
	 */
	bool ReconnectOnNextTick(float DeltaTime);

	/**
	 * Method attached to core ticker for monitoring peer last activity time. This method start to run regularly from
	 * the time its connected to peer. It will stop when peer inactive timeout detected and schedule next attempt to
	 * reconnect to host. In host side it only make sure the libjuice agent closed and destroyed.
	 *
	 * @param DeltaTime elapsed delta time in seconds.
	 * @return return true to restart the timer and false to stop it.
	 */
	bool PeerLastActivityTick(float DeltaTime);

	/**
	 * Start a ticker to monitor peer last activity time, called after peer connected.
	 * The interval is set by PeerLastActivityTickerIntervalInSeconds
	 */
	void StartPeerLastActivityWatcher();

	static bool IsPingData(const char* InData, size_t InSize);
};

#endif