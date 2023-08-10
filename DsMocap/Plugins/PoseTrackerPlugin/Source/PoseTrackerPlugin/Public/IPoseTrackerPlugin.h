//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once

#include "Modules/ModuleManager.h"


/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IPoseTrackerPlugin : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IPoseTrackerPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< IPoseTrackerPlugin >( "PoseTrackerPlugin" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "PoseTrackerPlugin" );
	}

	FORCEINLINE TSharedPtr<class FMediaPipePoseConnector> GetMediaPipeConnector() const
	{
		return MediaPipeConnector;
	}

	/**
	* Simple helper function to get the device currently active.
	* @return	Pointer to the MediaPipeConnector, or nullptr if Device is not available.
	*/
	static FMediaPipePoseConnector* GetMediaPipeConnectorSafe()
	{
#if WITH_EDITOR
		FMediaPipePoseConnector* MediaPipeConnector = IPoseTrackerPlugin::IsAvailable() ? IPoseTrackerPlugin::Get().GetMediaPipeConnector().Get() : nullptr;
#else
		FMediaPipePoseConnector* MediaPipeConnector = IPoseTrackerPlugin::Get().GetMediaPipeConnector().Get();
#endif
		return MediaPipeConnector;
	}


protected:
	/**
	* Reference to the actual MediaPipeConnector, grabbed through the GetMediaPipeConnector() interface, and created and destroyed in Startup/ShutdownModule
	*/
	TSharedPtr<class FMediaPipePoseConnector> MediaPipeConnector;
};


