// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/MediaBlueprintFunctionLibrary.h"
#include "MediaPlayer.h"
#include "WebcamController.generated.h"

// TODO: OnWebcamStarted
// TODO: OnWebcamStopped
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWebcamStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWebcamStopped);

UCLASS()
class AWebcamController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWebcamController();
	void Stop();
	void Start();
	void Play();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Test")
	FWebcamStarted OnWebcamStarted;
	UPROPERTY(BlueprintAssignable, Category = "Test")
	FWebcamStopped OnWebcamStopped;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config");
	FString VisualizationMode = TEXT("Avatar");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config");
	UMediaPlayer* MediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Format");
	FIntPoint SelectedResolution;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Format");
	FVector2D PreferedResolution = FVector2D(1280, 720);  //  1280.0 720.0
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Format");
	float PreferedFPS = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Format");
	float SelectedFPS = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Format");
	int32 SelectedFormat = 0;

private:
	void UpdateDeviceName();
	void OpenMediaPlayer(FString Url);
	UFUNCTION()
	void UpdateFormatsAndPlay();
	void UpdateTracks();
	void UpdateResolutions();
	void UpdateFPS();
	void SelectCamFormat();
	int32 FindFormatIndex();
private:
	TArray<FMediaCaptureDevice> CameraDevices;
	TArray<FText> WebcamNames;
	int32 WebcamID = 0;
	int32 TrackID = 0;
	TArray<FVector2D> AvailableResolutions;
	TArray<float> AvailableFPS;
	int32 ResolutionID = 0;
	int32 FPSID = 0;

};
