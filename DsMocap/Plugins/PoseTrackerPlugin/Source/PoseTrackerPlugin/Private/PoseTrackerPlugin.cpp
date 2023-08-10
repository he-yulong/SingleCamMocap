//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Windows.h"
#include "IPoseTrackerPlugin.h"

class FPoseTrackerPlugin : public IPoseTrackerPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


IMPLEMENT_MODULE(FPoseTrackerPlugin, PoseTrackerPlugin)

void FPoseTrackerPlugin::StartupModule()
{
	TSharedPtr<FMediaPipePoseConnector> MediaPipeStartup(new FMediaPipePoseConnector);
	if (MediaPipeStartup->StartupConnector())
	{
		MediaPipeConnector = MediaPipeStartup;
	}

}

void FPoseTrackerPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (MediaPipeConnector.IsValid())
	{
		MediaPipeConnector->ShutdownConnector();
		MediaPipeConnector = nullptr;
	}


	
}

#undef LOCTEXT_NAMESPACE
	

