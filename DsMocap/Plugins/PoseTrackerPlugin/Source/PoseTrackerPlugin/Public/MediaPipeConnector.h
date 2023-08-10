//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once
#include <vector>
#include <memory>
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "MediaTexture.h"
#include "JointOrientationCalculator.h"



//MediaPipe log
DECLARE_LOG_CATEGORY_EXTERN(MediaPipe, Log, All);


class FMediaPipePoseConnector
{

	
public:
	FMediaPipePoseConnector();
	virtual ~FMediaPipePoseConnector();

	/** Startup the device, and do any initialization that may be needed */
	bool StartupConnector();

	/** Tear down the device */
	void ShutdownConnector();


	uint8* Texture2DToUint8(UTexture2D* Texture);
	uint8* MediaTextureToUint8(UMediaTexture* Texture);
	uint8* TextureRenderTarget2DToUint8(UTextureRenderTarget2D* CameraRenderTarget);
	void ProcessFrame(int rows, int cols, uint8* rawFrame);
	bool InitGraph();
	bool ShutdownGraph();
	float GetAspectRatio();
	
	FVector GetLandmarkPosition(int ID, bool bDepth=true);
	FVector2D GetLandmarkPosition2D(int ID);
	FRotator GetLandmarkRotation(EJointType Joint);
	float GetLandmarkVisibility(int ID);
	float GetLandmarkPresence(int ID);
	float GetSkeletonHealth();

	bool bIsInitiated;

	//Camera parameters
	float HFoV;		
	float VFoV;
	float RefDistance; 
	float ShoulderWidthReference;
	float HipWidthReference;
	float Distance;
	float BodyYaw;
	
	//Need for fInterpTo
	float DeltaTime;
	
	

private:
	void FixDeadlock();

	/** Handle to the test dll we will load */
	void* MediaPipeLibraryHandle;
	void* OpenCVLibraryHandle;
	void* ProcessFrameFunctionHandle;
	void* InitGraphFunctionHandle;
	void* ShutdownGraphFunctionHandle;

	float AspectRatio;

	std::vector<std::vector<double>> pose_landmarks_vector;

	JointOrientationCalculator Calculator;
	
	
	
};
