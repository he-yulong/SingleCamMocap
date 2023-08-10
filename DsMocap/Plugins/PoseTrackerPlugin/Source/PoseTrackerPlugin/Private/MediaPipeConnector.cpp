//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//


#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
//#define WITH_CONSOLE

#include "MediaPipeConnector.h"
#include "Interfaces/IPluginManager.h"
#include "Components/LineBatchComponent.h"
#include "Runtime/Launch/Resources/Version.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"

#include "Windows.h"
#include "Core.h"
#include <string>
#include "stdio.h"
#include <iostream>

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif 

#include "Runtime/MediaAssets/Private/Misc/MediaTextureResource.h"


#include "MediaPipePose.h"
#include "DeadLockFix.h"




extern "C" {
	typedef bool (*process_frame_ptr)(int rows, int cols, uint8* raw_frame, std::vector<std::vector<double>>& pose_landmarks_vector);
	typedef bool (*init_graph_ptr)(LPCWSTR base_dir);
	typedef bool (*shutdown_graph_ptr)();
}

#define LOCTEXT_NAMESPACE "FMediaPipePluginModule"
DEFINE_LOG_CATEGORY(MediaPipe);

FMediaPipePoseConnector::FMediaPipePoseConnector()
{
	//TODO: Set camera params from blueprint
	HFoV = 90.0f;
	VFoV = 59.0f;
	RefDistance = 270.0f; //Distance where the width of shoulders are equal to the avatar width of shoulders in 2D, using this for reference to measure depth //270
	ShoulderWidthReference = 35.4f;  //Distance between shoulder_left and shoulder_right of the standard UE4 Mannequin 
	HipWidthReference = 18.01f;  //Distance between thigh_left and thigh_right of the standard UE4 Mannequin 
	bIsInitiated = false;
	Distance = RefDistance;
	

	// Console windows for debug
#ifdef WITH_CONSOLE
	AllocConsole();
	
	// std::cout, std::clog, std::cerr, std::cin
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	// std::wcout, std::wclog, std::wcerr, std::wcin
	HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
	SetStdHandle(STD_ERROR_HANDLE, hConOut);
	SetStdHandle(STD_INPUT_HANDLE, hConIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();
#endif

	//Class for calculating joint orientations from positions
	this->Calculator = JointOrientationCalculator();
	Calculator.SetConnector(this);

	AspectRatio = 1.0f;

	//Init the vector with some empty values
	for (int i = 0; i < 33; i++) {
		pose_landmarks_vector.push_back(std::vector<double>{0, 0, 0, 0 ,0});
	}

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("PoseTrackerPlugin")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
	FString OpenCVLibraryPath;

	FixDeadlock();

#if PLATFORM_WINDOWS
	
	#if WITH_EDITOR
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/PoseTrackerPluginLibrary/Win64/mediapipe_pose.dll"));
		OpenCVLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/PoseTrackerPluginLibrary/Win64/opencv_world3410.dll"));
	
    #else
		LibraryPath = TEXT("mediapipe_pose.dll");
		OpenCVLibraryPath = TEXT("opencv_world3410.dll");
	#endif //WITH EDITOR

#endif // PLATFORM_WINDOWS

	OpenCVLibraryHandle = nullptr;
	OpenCVLibraryHandle = FPlatformProcess::GetDllHandle(*OpenCVLibraryPath);

	if (OpenCVLibraryHandle)
	{
		//Do something here on module loaded
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "OpenCV loaded"));
	}
	else {
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "OpenCV NOT loaded"));
	}
	
	MediaPipeLibraryHandle = nullptr;
	MediaPipeLibraryHandle = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (MediaPipeLibraryHandle)
	{
		//Do something here on module loaded
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "Third party library loaded"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load MediaPipe third party library"));
	}
	
	
	UE_LOG(MediaPipe, Log, TEXT("Connector created."));
}

FMediaPipePoseConnector::~FMediaPipePoseConnector()
{
	/*
	FreeConsole();
	*/

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(MediaPipeLibraryHandle);
	MediaPipeLibraryHandle = nullptr;

	FPlatformProcess::FreeDllHandle(OpenCVLibraryHandle);
	OpenCVLibraryHandle = nullptr;

	UE_LOG(MediaPipe, Log, TEXT("Connector shutdown"));

	
}

uint8* FMediaPipePoseConnector::TextureRenderTarget2DToUint8(UTextureRenderTarget2D* CameraRenderTarget) {
	// Read the pixels from the RenderTarget and store them in a FColor array
	TArray<FColor> ColorData;
	FRenderTarget* RenderTarget = CameraRenderTarget->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(ColorData);
	return (uint8*)ColorData.GetData();
}


uint8* FMediaPipePoseConnector::Texture2DToUint8(UTexture2D* Texture) {
	uint8* raw = NULL;
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_ONLY);
	raw = (uint8*)Data;
	Mip.BulkData.Unlock();
	return raw;
}

uint8* FMediaPipePoseConnector::MediaTextureToUint8(UMediaTexture* Texture) {
	TArray<FColor> ColorData;
	FMediaTextureResource* TexResource = static_cast<FMediaTextureResource*>(Texture->Resource);
	TexResource->ReadPixels(ColorData);
	return (uint8*)ColorData.GetData();
}

bool FMediaPipePoseConnector::InitGraph() {
	bool result = false;
	InitGraphFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("initGraph"));
	auto init_graph_func = reinterpret_cast<init_graph_ptr>(InitGraphFunctionHandle);

	if (InitGraphFunctionHandle != nullptr) {
		FString ProjectDir;
#if WITH_EDITOR

		ProjectDir = FPaths::ProjectDir()+FString("Binaries/Win64");
		UE_LOG(MediaPipe, Log, TEXT("Binaries: %s"), *ProjectDir);
#else
		ProjectDir = "";
#endif //WITH_EDITOR
		
		
		char* dir = TCHAR_TO_ANSI(*ProjectDir);
		wchar_t wtext[255];
		mbstowcs(wtext, dir, strlen(dir) + 1);//Plus null
		LPWSTR ptr = wtext;

		result = init_graph_func(wtext);

		if (result == true) {
			UE_LOG(MediaPipe, Log, TEXT("Graph initialized"));
			bIsInitiated = true;
			return 1;
		}
	}
	else {
		UE_LOG(MediaPipe, Error, TEXT("Init graph function not found"));
	}
	return 0;
}

bool FMediaPipePoseConnector::ShutdownGraph() {
	bIsInitiated = false;
	bool result = false;
	
	ShutdownGraphFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("shutdownGraph"));
	auto shutdown_graph_func = reinterpret_cast<shutdown_graph_ptr>(ShutdownGraphFunctionHandle);

	if (ShutdownGraphFunctionHandle != nullptr) {
		result = shutdown_graph_func();

		if (result == true) {
			UE_LOG(MediaPipe, Log, TEXT("Graph shutdown completed."));
			return 1;
		}
	}
	else {
		UE_LOG(MediaPipe, Error, TEXT("shutdown graph function not found"));
	}
	return 0;
}


void FMediaPipePoseConnector::ProcessFrame(int rows, int cols, uint8* rawFrame) {

		//compute aspect ratio
	    AspectRatio = (float)cols / (float)rows;

		bool result = false;
		ProcessFrameFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("processFrame"));
		auto process_frame_func = reinterpret_cast<process_frame_ptr>(ProcessFrameFunctionHandle);



		


		if (ProcessFrameFunctionHandle != nullptr) {
			result = process_frame_func(rows, cols, rawFrame, pose_landmarks_vector);

			

		}
		else {
			UE_LOG(MediaPipe, Error, TEXT("Process frame function not found"));
		}

}



FVector FMediaPipePoseConnector::GetLandmarkPosition(int ID, bool bDepth) {
	if (pose_landmarks_vector.size() != 0 && pose_landmarks_vector.size() > ID) {

		//Projecting from 0-1 to -1 , 1 


		float x = (this->pose_landmarks_vector.at(ID).at(0) - 0.5f) * 2.0f;
		float y = (this->pose_landmarks_vector.at(ID).at(1) - 0.5f) * 2.0f;
		float z = this->pose_landmarks_vector.at(ID).at(2);
		
		//Ignore z if requested
		if (!bDepth) z = 0;
		
		float MultiplierRefH = FMath::Tan(FMath::DegreesToRadians(HFoV / 2.0f)) * RefDistance / 2.0f;
		
		

		
		
		
		//Distance based on shoulders
		float InterpSpeed=1.0f;
		

		if (FMath::Abs(BodyYaw) >= 175) InterpSpeed = 0.1;
		if (FMath::Abs(BodyYaw) < 175) InterpSpeed = 0.05;
		
		if (GetLandmarkVisibility(23) > 0.75 && GetLandmarkVisibility(24) > 0.75) {
			float HipWidthUser = (GetLandmarkPosition2D(23) - GetLandmarkPosition2D(24)).Size() * MultiplierRefH * 4.0f;
			if (HipWidthUser > 0.0f && FMath::Abs(BodyYaw) > 170 ) {
				Distance = FMath::FInterpTo(Distance, HipWidthReference / HipWidthUser * RefDistance, DeltaTime, InterpSpeed);
			}
		}
		//Distance based on hips
		else if(GetLandmarkVisibility(12) > 0.75 && GetLandmarkVisibility(11) > 0.75) {
			float ShoulderWidthUser = (GetLandmarkPosition2D(12) - GetLandmarkPosition2D(11)).Size() * MultiplierRefH * 4.0f;
			if (ShoulderWidthUser > 0.0f && FMath::Abs(BodyYaw) > 170 ) {
				Distance = FMath::FInterpTo(Distance, ShoulderWidthReference / ShoulderWidthUser * RefDistance, DeltaTime, InterpSpeed);
			}
		}
		
		
		float HipWidthUser = (GetLandmarkPosition2D(23) - GetLandmarkPosition2D(24)).Size() * MultiplierRefH * 4.0f;
		//UE_LOG(MediaPipe, Log, TEXT("BodyYaw: %f"), BodyYaw);
		
		
		VFoV = FMath::RadiansToDegrees(2*FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HFoV) / 2) * 1.0f/AspectRatio));
		//UE_LOG(MediaPipe, Log, TEXT("VFoV: %f"), VFoV);
		//UE_LOG(MediaPipe, Log, TEXT("Aspect Ratio: %f"), AspectRatio);
		//UE_LOG(MediaPipe, Log, TEXT("Distance: %f"), Distance);
		
		
		float Multiplier = FMath::Tan(FMath::DegreesToRadians(HFoV / 2.0f)) * Distance;
		float MultiplierV = FMath::Tan(FMath::DegreesToRadians(VFoV / 2.0f)) * Distance;

		if (Distance == 0) Distance = RefDistance;
			
		return FVector(x * Multiplier, -z * MultiplierV/2.0f, -y * MultiplierV) + FVector(0, -Distance, 0); 
	}

	return FVector(0, 0, 0);
}

FVector2D FMediaPipePoseConnector::GetLandmarkPosition2D(int ID) {
	if (pose_landmarks_vector.size() != 0 && pose_landmarks_vector.size() > ID) {

		//Range from 0-1
		float x = this->pose_landmarks_vector.at(ID).at(0);
		float y = this->pose_landmarks_vector.at(ID).at(1);
		

		//TO DO: Need to convert texture space to real 3D space based on camera fov, and other parameters.
		return FVector2D(x, y);

	}
	return FVector2D(0, 0);
}

FRotator FMediaPipePoseConnector::GetLandmarkRotation(EJointType Joint) {
	return Calculator.GetSkeletonJointOrientation(Joint).Rotator();
}

float FMediaPipePoseConnector::GetLandmarkVisibility(int ID) {
	if (pose_landmarks_vector.size() != 0 && pose_landmarks_vector.size() > ID) {
		return (float)this->pose_landmarks_vector.at(ID).at(3);
	}
	return 0.0f;
}

float FMediaPipePoseConnector::GetLandmarkPresence(int ID) {
	if (pose_landmarks_vector.size() != 0 && pose_landmarks_vector.size() > ID) {
		return (float)this->pose_landmarks_vector.at(ID).at(4);
	}
	return 0.0f;
}

float FMediaPipePoseConnector::GetSkeletonHealth() {
	float health = 0;;
	for (int i = 0; i < pose_landmarks_vector.size(); i++) {
		health = health + GetLandmarkVisibility(i);
	}
	health = health / pose_landmarks_vector.size();
	return health;
}


float FMediaPipePoseConnector::GetAspectRatio() {
	return this->AspectRatio;
}


bool FMediaPipePoseConnector::StartupConnector() {
	
	//return InitGraph();
	return 1;
}

void FMediaPipePoseConnector::ShutdownConnector() {
	
	//Need this otherwise the module won't close, and the editor remains in the memory. 
	ShutdownGraph();
}

void FMediaPipePoseConnector::FixDeadlock()
{
#if (ENGINE_MAJOR_VERSION == 5)

	const char* Variants[] = {
		// 5.0.0
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 B5 C6 B5 00", // dev editor (unrealeditor-core.dll)
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 CC CC CC CC 48 33 C4 48 89 45 17 4D 63 68 3C 33 C0", // dev game
	};
	const int NumVariants = sizeof(Variants) / sizeof(Variants[0]);

	auto Process = GetCurrentProcess();

	for (int i = 0; i < NumVariants; ++i)
	{
		auto Pattern = CkParseByteArray(Variants[i]);
		std::vector<uint8_t*> Loc;
		auto Status = CkFindPatternIntern<CkWildcardCC>(Process, Pattern, 2, Loc);
		
		if (Status == 0 && Loc.size() == 1)
		{
			auto Fix = CkParseByteArray("C3");
			Status = CkProtectWriteMemory(Process, Fix, Loc[0], 0);
			return;
		}
	}

	

#endif
}



