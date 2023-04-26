// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AccelByteSignalingModels.h"

/*
 * AccelByteSignalingBase is the base interface for WebRTC ICE connection
 */

class AccelByteSignalingBase
{
public:
	/**
	 * @brief Delegate when there is data from signaling server
	 * @param Param 1 is the PeerId where it came from
	 * @param Param 2 is the message in Json String
	 */
	DECLARE_DELEGATE_TwoParams(OnWebRTCSignalingMessage, const FString&, const FString &);

	virtual ~AccelByteSignalingBase() 
	{
	}

	/**
	* @brief Init the signaling
	*/
	virtual void Init() = 0;

	/**
	* @brief Check if signaling is connected
	*
	* @return true if connected;
	*/
	virtual bool IsConnected() const = 0;

	/**
	* @brief Connect to signaling server
	*
	*/
	virtual void Connect() = 0;

	/**
	* @brief SendMessage to signaling server
	*
	* @param PeerId destination of peer to sent
	* @param Message to send
	*/
	virtual void SendMessage(const FString &PeerId, const FAccelByteSignalingMessage &Message) = 0;

	/**
	* @brief Set the delegate when any data from signaling message
	*
	* @param Delegate with type OnWebRTCSignalingMessage
	*/
	void SetOnWebRTCSignalingMessage(OnWebRTCSignalingMessage Delegate) 
	{ 
		OnWebRtcSignalingMessageDelegate = Delegate;
	}

protected:
	OnWebRTCSignalingMessage OnWebRtcSignalingMessageDelegate;
};