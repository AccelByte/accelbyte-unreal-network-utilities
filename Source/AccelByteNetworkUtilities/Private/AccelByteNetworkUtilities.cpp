// Copyright Epic Games, Inc. All Rights Reserved.

#include "AccelByteNetworkUtilities.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "AccelByteNetworkUtilitiesLog.h"
#include "SocketSubsystemModule.h"
#include "Interfaces/IPluginManager.h"
#include "Connection/SocketSubsystemAccelByte.h"
#include "AccelByteNetworkUtilitiesConstant.h"
#include "Networking/AccelByteNetworkManager.h"

#define LOCTEXT_NAMESPACE "FAccelByteNetworkUtilitiesModule"

DEFINE_LOG_CATEGORY(AccelByteNetUtil);
DEFINE_LOG_CATEGORY(AccelByteSignalingMessage);

void* LibICEHandle = nullptr;

void FAccelByteNetworkUtilitiesModule::StartupModule()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("AccelByteNetworkUtilities");
	const FString BaseDir = Plugin->GetBaseDir();
	const FPluginDescriptor &Descriptor = Plugin->GetDescriptor();

	UE_LOG_ABNET(Log, TEXT("AccelByteNetworkUtilities version: %s"), *Descriptor.VersionName);

#ifdef LIBJUICE
#if PLATFORM_SWITCH
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("juice.nro"));
#elif PLATFORM_LINUX
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("libjuice.so"));
#endif //PLATFORM_WINDOWS

	FSocketSubsystemAccelByte* SocketSubsystem = FSocketSubsystemAccelByte::Create();
	FString Error;
	if (SocketSubsystem->Init(Error))
	{
		FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
		SSS.RegisterSocketSubsystem(ACCELBYTE_SUBSYSTEM, SocketSubsystem, false);
		UE_LOG_ABNET(Log, TEXT("AccelByte socket subsystem registered"));
	}
	else
	{
		UE_LOG_ABNET(Log, TEXT("Can not register AccelByte socket subsystem: %s"), *Error);
	}
	
#endif //LIBJUICE
}

void FAccelByteNetworkUtilitiesModule::ShutdownModule()
{
	UE_LOG_ABNET(Log, TEXT("AccelByteNetworkUtilities shutdown"));
	if(LibICEHandle)
	{
		FPlatformProcess::FreeDllHandle(LibICEHandle);
		LibICEHandle = nullptr;		
	}
}

FAccelByteNetworkUtilitiesModule& FAccelByteNetworkUtilitiesModule::Get()
{
	return FModuleManager::LoadModuleChecked<FAccelByteNetworkUtilitiesModule>("AccelByteNetworkUtilities");
}

void FAccelByteNetworkUtilitiesModule::Setup(AccelByte::FApiClientPtr InApiClientPtr)
{
	AccelByteNetworkManager::Instance().Setup(InApiClientPtr);
}

void FAccelByteNetworkUtilitiesModule::RequestConnect(const FString& PeerId)
{
	AccelByteNetworkManager::Instance().RequestConnect(PeerId);
}

void FAccelByteNetworkUtilitiesModule::RegisterICEConnectedDelegate(OnICEConnected Delegate)
{
	AccelByteNetworkManager::Instance().OnWebRTCDataChannelConnectedDelegate = Delegate;
}

void FAccelByteNetworkUtilitiesModule::CloseAllICEConnection()
{
	AccelByteNetworkManager::Instance().CloseAllPeerConnections();
}

void FAccelByteNetworkUtilitiesModule::RegisterDefaultSocketSubsystem()
{
	FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	ISocketSubsystem* SocketSubsystem = SSS.GetSocketSubsystem(ACCELBYTE_SUBSYSTEM);
	if(SocketSubsystem == nullptr)
	{
		UE_LOG_ABNET(Warning, TEXT("AccelByte socket subsystem is null!"));
	}
	else
	{
		SSS.RegisterSocketSubsystem(ACCELBYTE_SUBSYSTEM, SocketSubsystem, true);
		UE_LOG_ABNET(Log, TEXT("AccelByte socket subsystem registered as default"));
	}	
}

void FAccelByteNetworkUtilitiesModule::UnregisterDefaultSocketSubsystem()
{
	FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	ISocketSubsystem* PlatformSocketSubsystem = SSS.GetSocketSubsystem(PLATFORM_SOCKETSUBSYSTEM);
	SSS.RegisterSocketSubsystem(PLATFORM_SOCKETSUBSYSTEM, PlatformSocketSubsystem, true);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAccelByteNetworkUtilitiesModule, AccelByteNetworkUtilities)
