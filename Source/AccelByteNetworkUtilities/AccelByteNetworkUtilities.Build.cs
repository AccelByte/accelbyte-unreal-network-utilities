// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;

public class AccelByteNetworkUtilities : ModuleRules
{
	public AccelByteNetworkUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDefinitions.Add("ACCELBYTE_NETWORK_UTILITIES_PACKAGE=1");
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		String PlatformString = Target.Platform.ToString().ToUpper();
		
		/*
		 * The LibJuice precompiled library only for Win64, XboxGDK, XSX, PS4, and PS5
		 * Other platform still not supported yet.
		 * LibJuice is library that handle the nat punch connection.
		 */
		if (PlatformString == "WIN64" || 
		    PlatformString == "XBOXONEGDK" ||
		    PlatformString == "XSX" ||
		    PlatformString == "PS4" ||
		    PlatformString == "PS5" ||
		    PlatformString == "SWITCH" ||
		    PlatformString == "LINUX")
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
