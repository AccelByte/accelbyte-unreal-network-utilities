// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SocketAccelByte.h"

#include "AccelByteNetworkUtilities.h"
#include "Networking/AccelByteNetworkManager.h"
#include "IpAddressAccelByte.h"
#include "AccelByteNetworkUtilitiesConstant.h"

FSocketAccelByte::FSocketAccelByte()
{
}

bool FSocketAccelByte::Shutdown(ESocketShutdownMode Mode)
{
	return true;
}

bool FSocketAccelByte::Close()
{
	// ICE connection closed when game session is done
	return true;
}

bool FSocketAccelByte::Bind(const FInternetAddr& Addr)
{
	return true;
}

bool FSocketAccelByte::Connect(const FInternetAddr& Addr)
{
	//WebRTC does not support connect TCP
	return false;
}

bool FSocketAccelByte::Listen(int32 MaxBacklog)
{
	//WebRTC not listening
	return false;
}

bool FSocketAccelByte::WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime)
{
	//not implemented as the connection is WebRTC
	return false;
}

bool FSocketAccelByte::HasPendingData(uint32& PendingDataSize)
{
	if (!NetId.IsEmpty()) 
	{
		return AccelByteNetworkManager::Instance().HasPendingData(PendingDataSize);
	}
	PendingDataSize = 0;
	return false;
}

class FSocket* FSocketAccelByte::Accept(const FString& InSocketDescription)
{
	//Not implemented using webRTC, only support UDP
	return nullptr;
}

class FSocket* FSocketAccelByte::Accept(FInternetAddr& OutAddr, const FString& InSocketDescription)
{
	//Not implemented using webRTC, only support UDP
	return nullptr;
}

bool FSocketAccelByte::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
	const FInternetAddrAccelByte& Dest = static_cast<const FInternetAddrAccelByte&>(Destination);
	NetId = Dest.NetId;
	Channel = Dest.GetPort();
	return AccelByteNetworkManager::Instance().SendTo(Data, Count, BytesSent, Dest.ToString(true));
}

bool FSocketAccelByte::Send(const uint8* Data, int32 Count, int32& BytesSent) {
	//Not implemented using webRTC, only support UDP
	return false;
}

bool FSocketAccelByte::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags)
{
	FString SourcePeer;
	int32 SourceChannel;
	if (AccelByteNetworkManager::Instance().RecvFrom(Data, BufferSize, BytesRead, SourcePeer, SourceChannel)) 
	{
		static_cast<FInternetAddrAccelByte&>(Source).SetPeerChannel(SourcePeer, SourceChannel);
		return true;
	}
	return false;
}

bool FSocketAccelByte::Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags)
{
	//not implemented on TCP
	return false;
}

bool FSocketAccelByte::RecvMulti(FRecvMulti& MultiData, ESocketReceiveFlags::Type Flags)
{
	//not implemented using webRTC
	return false;
}

bool FSocketAccelByte::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
	//not implemented as listen method
	return false;
}

ESocketConnectionState FSocketAccelByte::GetConnectionState()
{
	return ESocketConnectionState::SCS_NotConnected;
}
void FSocketAccelByte::GetAddress(FInternetAddr& /*OutAddr*/)
{
	//not implemented
}

bool FSocketAccelByte::GetPeerAddress(FInternetAddr& /*OutAddr*/)
{
	//not implemented
	return false;
}

bool FSocketAccelByte::SetNonBlocking(bool bIsNonBlocking)
{
	//non blocking by default
	return true;
}

bool FSocketAccelByte::SetBroadcast(bool bAllowBroadcast)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::JoinMulticastGroup(const FInternetAddr& GroupAddress)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::LeaveMulticastGroup(const FInternetAddr& GroupAddress)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::SetMulticastLoopback(bool bLoopback)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::SetMulticastTtl(uint8 TimeToLive)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::SetMulticastInterface(const FInternetAddr& InterfaceAddress)
{
	//not implemented broadcast on webRTC connection
	return false;
}

bool FSocketAccelByte::SetReuseAddr(bool bAllowReuse)
{
	//true by default
	return true;
}

bool FSocketAccelByte::SetNoDelay(bool bIsNoDelay)
{
	//enabled by default
	return true;
}

bool FSocketAccelByte::SetLinger(bool bShouldLinger, int32 Timeout)
{
	//enabled by default
	return true;
}

bool FSocketAccelByte::SetRecvErr(bool bUseErrorQueue)
{
	//TODO: check with webRTC send error
	return true;
}

bool FSocketAccelByte::SetSendBufferSize(int32 Size, int32& NewSize)
{
	//always success because directly send data to webrtc data channel
	return true;
}

bool FSocketAccelByte::SetReceiveBufferSize(int32 Size, int32& NewSize)
{
	//always success because directly read data from webrtc data channel
	return true;
}

int32 FSocketAccelByte::GetPortNo()
{
	return Channel;
}