// Fill out your copyright notice in the Description page of Project Settings.
#include "WebcamController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#pragma optimize("", off)
// Sets default values
AWebcamController::AWebcamController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWebcamController::BeginPlay()
{
	Super::BeginPlay();
	Start();
}

void AWebcamController::Start()
{
	UpdateDeviceName();

	OpenMediaPlayer(CameraDevices[0].Url);  // TODO: error prone
}

void AWebcamController::Stop()
{
	MediaPlayer->Pause();
}

void AWebcamController::Play()
{
	if (MediaPlayer->IsPlaying())
	{
		MediaPlayer->Close();
		OnWebcamStopped.Broadcast();
		Start();
	}
	else
	{
		bool Status = MediaPlayer->Play();
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Cyan, FString::Printf(TEXT("play: %d"), Status));
		OnWebcamStarted.Broadcast();
	}
}

void AWebcamController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();
}

void AWebcamController::UpdateDeviceName()
{
	UMediaBlueprintFunctionLibrary::EnumerateVideoCaptureDevices(CameraDevices, -1);

	for (FMediaCaptureDevice& CameraDevice : CameraDevices)
	{
		WebcamNames.Add(CameraDevice.DisplayName);
	}
}

// Called every frame
void AWebcamController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWebcamController::OpenMediaPlayer(FString Url)
{
	if (MediaPlayer->OpenUrl(Url)) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, TEXT("Open Camera successfully!"));

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "UpdateFormatsAndPlay";
	LatentInfo.Linkage = 1;
	LatentInfo.UUID = 100;
	UKismetSystemLibrary::Delay(this, 0.4f, LatentInfo);
}

void AWebcamController::UpdateFormatsAndPlay()
{
	UpdateTracks();
	UpdateResolutions();
	UpdateFPS();

	UE_LOG(LogTemp, Warning, TEXT("Track Index: %d, Selected Resolution: (%d, %d), SelectedFPS: %f"), TrackID, SelectedResolution.X, SelectedResolution.Y, SelectedFPS);
	SelectCamFormat();
	Play();
}

void AWebcamController::UpdateTracks()
{
	TrackID = MediaPlayer->GetNumTracks(EMediaPlayerTrack::Video) - 1;  // TODO: error prone
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Track Num: %d"), TrackID + 1));
}

void AWebcamController::UpdateResolutions()
{
	int32 FormatsNum = MediaPlayer->GetNumTrackFormats(EMediaPlayerTrack::Video, TrackID);
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Formats Num: %d"), FormatsNum));

	for (int32 i = 0; i < FormatsNum; ++i)
	{
		FIntPoint Resolution = MediaPlayer->GetVideoTrackDimensions(TrackID, i);
		AvailableResolutions.AddUnique(Resolution);
	}

	for (int i = 0; i < AvailableResolutions.Num(); ++i)
	{
		if (UKismetMathLibrary::EqualEqual_Vector2DVector2D(PreferedResolution, AvailableResolutions[i], 1e-4))
		{
			ResolutionID = i;
		}
	}

	SelectedResolution = FIntPoint(AvailableResolutions[ResolutionID].X, AvailableResolutions[ResolutionID].Y);
}

void AWebcamController::UpdateFPS()
{
	int32 FormatsNum = MediaPlayer->GetNumTrackFormats(EMediaPlayerTrack::Video, TrackID);
	for (int32 i = 0; i < FormatsNum; ++i)
	{
		FIntPoint Resolution = MediaPlayer->GetVideoTrackDimensions(TrackID, i);
		
		if (UKismetMathLibrary::EqualEqual_Vector2DVector2D(Resolution, SelectedResolution, 1e-4))
		{
			AvailableFPS.AddUnique(MediaPlayer->GetVideoTrackFrameRate(TrackID, i));
		}
	}
	for (int i = 0; i < AvailableFPS.Num(); ++i)
	{
		if (PreferedFPS == AvailableFPS[i]) FPSID = i;
	}
	SelectedFPS = AvailableFPS[FPSID];
}

void AWebcamController::SelectCamFormat()
{
	MediaPlayer->SelectTrack(EMediaPlayerTrack::Video, TrackID);
	SelectedFormat = FindFormatIndex();
	MediaPlayer->SetTrackFormat(EMediaPlayerTrack::Video, TrackID, SelectedFormat);
}

int32 AWebcamController::FindFormatIndex()
{
	int32 FormatsNum = MediaPlayer->GetNumTrackFormats(EMediaPlayerTrack::Video, TrackID);
	for (int i = 0; i < FormatsNum; ++i)
	{
		bool MatchResolution = SelectedResolution == MediaPlayer->GetVideoTrackDimensions(TrackID, i);
		bool MatchFPS = SelectedFPS == MediaPlayer->GetVideoTrackFrameRate(TrackID, i);
		if (MatchResolution && MatchFPS) return i;
	}

	return 0;
}

#pragma optimize("", on)
