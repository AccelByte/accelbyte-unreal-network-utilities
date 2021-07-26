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

		String PlatformString = Target.Platform.ToString().ToUpper();
		
		if (PlatformString == "WIN64")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "x64/debug/juice.dll"), StagedFileType.NonUFS);
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64/debug/juice.lib"));
			}
			else
			{
				RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "x64/release/juice.dll"), StagedFileType.NonUFS);
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64/release/juice.lib"));
			}
			PublicDelayLoadDLLs.Add("juice.dll");
        } 
        else if (PlatformString == "XBOXONEGDK")
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			    Target.Configuration == UnrealTargetConfiguration.Development ||
			    Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				//RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "xbox/debug/juice.dll"), StagedFileType.NonUFS);
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xbox/static/debug/juice.lib"));
			}
			else
			{
				//RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "xbox/release/juice.dll"), StagedFileType.NonUFS);
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
				//RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "xsx/debug/juice.dll"), StagedFileType.NonUFS);
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "xsx/static/debug/juice.lib"));
			}
			else
			{
				//RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "xsx/release/juice.dll"), StagedFileType.NonUFS);
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
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.prx",Path.Combine(ModuleDirectory, "ps4/debug/juice.prx"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps4/debug/juice_stub.a"));
			}
			else
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.prx",Path.Combine(ModuleDirectory, "ps4/release/juice.prx"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps4/release/juice_stub.a"));
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
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.prx",Path.Combine(ModuleDirectory, "ps5/debug/juice.prx"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps5/debug/juice_stub.a"));
			}
			else
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/juice.prx",Path.Combine(ModuleDirectory, "ps5/release/juice.prx"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ps5/release/juice_stub.a"));
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
				PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "linux64/debug/"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "linux64/debug/libjuice.so"));
			}
			else
			{
				RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "linux64/release/libjuice.so"));
				PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "linux64/release/"));
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "linux64/release/libjuice.so"));
			}
			PublicDelayLoadDLLs.Add("libjuice.so");
		}
	}
}
