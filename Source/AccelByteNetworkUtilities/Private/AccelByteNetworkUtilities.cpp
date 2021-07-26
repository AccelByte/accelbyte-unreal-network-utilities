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
	UE_LOG_ABNET(Log, TEXT("AccelByteNetworkUtilities startup"));
	
	const FString BaseDir = IPluginManager::Get().FindPlugin("AccelByteNetworkUtilities")->GetBaseDir();

#ifdef LIBJUICE
#if PLATFORM_WINDOWS
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	const auto LibraryPath = FPaths::Combine(*BaseDir, TEXT("source/ThirdParty/LibJuice/x64/debug/juice.dll"));
#else
	const auto LibraryPath = FPaths::Combine(*BaseDir, TEXT("source/ThirdParty/LibJuice/x64/release/juice.dll"));
#endif
	LibICEHandle = FPlatformProcess::GetDllHandle(*LibraryPath);
#elif PLATFORM_SWITCH
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("juice.nro"));
#elif PLATFORM_XSX
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	const auto LibraryPath = FPaths::Combine(*BaseDir, TEXT("source/ThirdParty/LibJuice/xsx/debug/juice.dll"));
#else
	const auto LibraryPath = FPaths::Combine(*BaseDir, TEXT("source/ThirdParty/LibJuice/xsx/release/juice.dll"));
#endif
	//TODO: dynamic library not working for now
	//LibICEHandle = FPlatformProcess::GetDllHandle(LibraryPath);
#elif PLATFORM_PS4
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("juice.prx"));
#elif PLATFORM_PS5
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("juice.prx"));
#elif PLATFORM_LINUX
	LibICEHandle = FPlatformProcess::GetDllHandle(TEXT("libjuice.so"));
#endif //PLATFORM_WINDOWS

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

void FAccelByteNetworkUtilitiesModule::Run()
{
	AccelByteNetworkManager::Instance().Run();
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
	FSocketSubsystemAccelByte* SocketSubsystem = FSocketSubsystemAccelByte::Create();
	FString Error;
	if (SocketSubsystem->Init(Error))
	{
		FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
		SSS.RegisterSocketSubsystem(ACCELBYTE_SUBSYSTEM, SocketSubsystem, true);
		UE_LOG_ABNET(Log, TEXT("AccelByte socket subsystem registered"));
	}
	else
	{
		UE_LOG_ABNET(Log, TEXT("Can not register AccelByte socket subsystem: %s"), *Error);
	}
}

void FAccelByteNetworkUtilitiesModule::UnregisterDefaultSocketSubsystem()
{
	FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	SSS.UnregisterSocketSubsystem(ACCELBYTE_SUBSYSTEM);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAccelByteNetworkUtilitiesModule, AccelByteNetworkUtilities)
