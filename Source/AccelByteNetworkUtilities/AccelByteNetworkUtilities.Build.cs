// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;

public class AccelByteNetworkUtilities : ModuleRules
{
	public AccelByteNetworkUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDefinitions.Add("ACCELBYTE_NETWORK_UTILITIES_PACKAGE=1");
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_5_2_OR_LATER
		IWYUSupport = IWYUSupport.Full;
#else
		bEnforceIWYU = true;
#endif
		bAllowConfidentialPlatformDefines = true;

		/*
		 * Use platform string because Platform_PS5 not available on Unreal installed from Epic Launcher
		 */
		String PlatformString = Target.Platform.ToString().ToUpper();

		if (PlatformString == "PS5")
		{
			bAllowConfidentialPlatformDefines = true;
		}
		
		/*
		 * LibJuice is library that handle the nat punch connection.
		 *
		 * Linux is gated to UE5+. The prebuilt linux64/libjuice.so is built against
		 * GLIBC 2.34, which the toolchain bundled with 4.27 cannot link:
		 *   ld.lld: error: libjuice.so: undefined reference to pthread_create@GLIBC_2.34
		 * Keep this in step with the matching gate in LibJuice.Build.cs, and drop both
		 * once the .so is rebuilt against an older glibc.
		 */
		bool bLibJuiceLinuxSupported = PlatformString == "LINUX" && Target.Version.MajorVersion >= 5;

		if (PlatformString == "WIN64" ||
		    PlatformString == "XBOXONEGDK" ||
		    PlatformString == "XB1" ||
		    PlatformString == "XSX" ||
		    PlatformString == "PS4" ||
		    PlatformString == "PS5" ||
		    PlatformString == "SWITCH" ||
		    bLibJuiceLinuxSupported ||
		    PlatformString == "MAC")
		{
			PrivateDefinitions.Add("LIBJUICE");
		}

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"OnlineSubsystemUtils"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"LibJuice",
				"CoreUObject",
				"Projects",
				"WebSockets",
				"NetCore",
				"Engine",
				"Sockets",
				"OnlineSubsystem",
				"PacketHandler",
				"Json",
				"JsonUtilities",
				"AccelByteUe4Sdk"
			}
		);
	}
}
