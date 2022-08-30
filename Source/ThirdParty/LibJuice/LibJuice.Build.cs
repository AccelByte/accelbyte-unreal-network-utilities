// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using System;
using System.Diagnostics;
using System.IO;
using UnrealBuildTool;

public class LibJuice : ModuleRules
{
	public LibJuice(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		/*
		 * Use platform string because Platform_PS5 not available on Unreal installed from Epic Launcher
		 */
		String PlatformString = Target.Platform.ToString().ToUpper();
		
		if (PlatformString == "WIN64")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64/static/relwithdebinfo/juice-static.lib"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64/static/release/juice-static.lib"));
			}
			PublicAdditionalLibraries.Add("bcrypt.lib");
		}
        else if (PlatformString == "XBOXONEGDK")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xbox/static/debug/juice.lib"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xbox/static/release/juice.lib"));
			}
		} 
		else if (PlatformString == "XSX")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xsx/static/debug/juice.lib"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xsx/static/release/juice.lib"));
			}
		}
		else if (PlatformString == "PS4")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			PrivateDefinitions.Add("ORBIS_BUILD");
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps4/debug/juice.a"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps4/release/juice.a"));
			}
		} 
		else if (PlatformString == "PS5")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			PrivateDefinitions.Add("ORBIS_BUILD");
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
				Target.Configuration == UnrealTargetConfiguration.Development ||
				Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps5/debug/juice.a"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps5/release/juice.a"));
			}
		}
		else if (PlatformString == "SWITCH")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
				Target.Configuration == UnrealTargetConfiguration.Development ||
				Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.nro",Path.Combine(ModuleDirectory, "switch/debug/juice.nro"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "switch/debug/juice.nrs"));
			}
			else
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.nro",Path.Combine(ModuleDirectory, "switch/release/juice.nro"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "switch/release/juice.nrs"));
			}
		}
		else if (PlatformString == "LINUX")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "linux64/debug/libjuice.so"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "linux64/debug/libjuice.so"));
			}
			else
			{
				RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "linux64/release/libjuice.so"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "linux64/release/libjuice.so"));
			}
			PublicDelayLoadDLLs.Add("libjuice.so");
		}
	}
}
