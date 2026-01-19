// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public abstract class MobuLiveLinkPluginBase : ModuleRules
{
	public MobuLiveLinkPluginBase(ReadOnlyTargetRules Target, string MobuVersionString) : base(Target)
	{
		IWYUSupport = IWYUSupport.None;
		bUseRTTI = true;

		PrivateIncludePathModuleNames.Add("Launch");

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "StreamObjects/Public"),
		});

		// Unreal dependency modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Messaging",
			"Projects",
			"UdpMessaging",
			"LiveLinkInterface",
			"LiveLinkMessageBusFramework",
			"IntelTBB"
		});

		// Mobu SDK setup
		{
			//UE_MOTIONBUILDER2017_INSTALLATIONFOLDER
			string MobuInstallFolder = System.Environment.GetEnvironmentVariable("UE_MOTIONBUILDER" + MobuVersionString + "_INSTALLATIONFOLDER");
			if (string.IsNullOrEmpty(MobuInstallFolder))
			{
				MobuInstallFolder = @"C:\Program Files\Autodesk\MotionBuilder " + MobuVersionString;
			}
			MobuInstallFolder = Path.Combine(MobuInstallFolder, "OpenRealitySDK");

			if (!Directory.Exists(MobuInstallFolder))
			{
				// Try with build machine setup
				string SDKRootEnvVar = System.Environment.GetEnvironmentVariable("UE_SDKS_ROOT");
				if (!string.IsNullOrEmpty(SDKRootEnvVar))
				{
					MobuInstallFolder = Path.Combine(SDKRootEnvVar, "HostWin64", "Win64", "MotionBuilder", MobuVersionString);
				}
			}

			// Make sure this version of Mobu is actually installed
			if (Directory.Exists(MobuInstallFolder))
			{
				// Then add Mobu SDK include path
				PrivateIncludePaths.Add(Path.Combine(MobuInstallFolder, "include"));
				
				// Add Python support for Boost.Python bindings
				// Use PublicSystemIncludePaths to force compiler to find Python headers
				string PythonInclude = Path.Combine(MobuInstallFolder, "include", "python-2.7.11", "Include");
				if (Directory.Exists(PythonInclude))
				{
					System.Console.WriteLine("=== Adding Python Include Path: " + PythonInclude);
					PublicSystemIncludePaths.Add(PythonInclude);
					// Also try adding it multiple ways to ensure it works
					PublicIncludePaths.Add(PythonInclude);
					PrivateIncludePaths.Add(PythonInclude);
				}
				else
				{
					System.Console.WriteLine("=== ERROR: Python Include Path NOT FOUND: " + PythonInclude);
				}

				if (Target.Platform == UnrealTargetPlatform.Win64)  // @todo: Support other platforms?
				{
					string LibDir = Path.Combine(MobuInstallFolder, "lib/x64");

					// Mobu library we're depending on
					PublicAdditionalLibraries.Add(Path.Combine(LibDir, "fbsdk.lib"));
					
					// Add Python 2.7 library for Python C API
					string PythonLib = Path.Combine(LibDir, "python27.lib");
					if (File.Exists(PythonLib))
					{
						PublicAdditionalLibraries.Add(PythonLib);
					}
					else
					{
						System.Console.WriteLine("WARNING: Python27.lib not found at: " + PythonLib);
					}
				}
			}

			PublicDefinitions.Add("PRODUCT_VERSION=" + MobuVersionString);
		}
	}
}

public class MobuLiveLinkPlugin2017 : MobuLiveLinkPluginBase
{
	public MobuLiveLinkPlugin2017(ReadOnlyTargetRules Target) : base(Target, "2017")
	{
		// NOTE: A spot fix for an SDK header is required for include/fbsdk/fbproperties.h
		// Line 783:
		// 	From:	inline void operator=(tType pValue)					{ SetData( &pValue ); }
		// 	To:		inline void operator=(tType pValue)					{ this->SetData( &pValue ); }
		// Line 790:
		// 	From:	inline operator tType() const						{ tType Value; GetData( &Value,sizeof(Value) ); return Value; } 
		// 	To:		inline operator tType() const						{ tType Value; this->GetData( &Value,sizeof(Value) ); return Value; } 
	}
}
