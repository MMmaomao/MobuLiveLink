// Copyright Epic Games, Inc. All Rights Reserved.

#include "RequiredProgramMainCPPInclude.h"
#include "MobuLiveLinkCommon.h"
#include "MobuLiveLinkDevice.h"

DEFINE_LOG_CATEGORY_STATIC(LogMoBuPlugin, Log, All);

IMPLEMENT_APPLICATION(MobuLiveLinkPlugin, "MobuLiveLinkPlugin");

//--- Library declaration
FBLibraryDeclare( FMobuLiveLink )
{
	FBLibraryRegister( FMobuLiveLink		);
	FBLibraryRegister( FMobuLiveLinkLayout	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	
{
	GEngineLoop.PreInit(TEXT("MobuLiveLinkPlugin -Messaging"));

	// ensure target platform manager is referenced early as it must be created on the main thread
	GetTargetPlatformManager();

	ProcessNewlyLoadedUObjects();

	// Tell the module manager that it may now process newly-loaded UObjects when new C++ modules are loaded
	FModuleManager::Get().StartProcessingNewlyLoadedObjects();
	FModuleManager::Get().LoadModule(TEXT("UdpMessaging"));

	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::PreDefault);
	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::Default);
	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::PostDefault);
	
	// Initialize Python bindings early so 'import livelink' works even before device is created
	InitMobuLiveLinkPythonBindings();
	
	FBTrace("MobuLiveLink Library Initialized\n");
	FBTrace("Python module 'livelink' is now available for import\n");
	return true;
}

bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
