//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once
#include "Engine/Texture2D.h"
#include "MediaPipePoseTrackerComponent.generated.h"


UCLASS(ClassGroup = MediaPipe, BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UMediaPipePoseTrackerComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()


public:
	
		void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
		UFUNCTION(BlueprintCallable,  Category = "Pose tracker")
		bool InitGraph();

		UFUNCTION(BlueprintCallable,  Category = "Pose tracker")
		bool ShutdownGraph();
		
		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void GetPoseLandmarksFromRenderTarget2D(UTextureRenderTarget2D* TextureRenderTarget);

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void GetPoseLandmarksFromTexture2D(UTexture2D* Texture);

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void GetPoseLandmarksFromMediaTexture(UMediaTexture* Texture);

		UFUNCTION(BlueprintCallable, BlueprintPure,  Category = "Pose tracker")
		FVector GetLandmarkPosition(int ID, bool bDepth=true);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
		FVector2D GetLandmarkPosition2D(int ID);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
		FRotator GetLandmarkRotation(EJointType Joint);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
		float GetLandmarkVisibility(int ID);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
		float GetLandmarkPresence(int ID);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
		float GetSkeletonHealth();

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void SetHFoV(float Degrees);

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		float GetHFoV(float Degrees);


		//DRAW HELPER Function TODO: Move to a separate component

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void DrawLine(FVector StartLocation, FVector EndLocation, FLinearColor Color, uint8 DepthPriority, float Thickness, float LifeTime);

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void DrawPoint(FVector Position, FLinearColor Color, float PointSize, uint8 DepthPriority, float LifeTime);

		UFUNCTION(BlueprintCallable, Category = "Pose tracker")
		void DrawCylinder(FVector const& Start, FVector const& End, float Radius, int32 Segments, FLinearColor const& Color, float LifeTime, uint8 DepthPriority, float Thickness);

};