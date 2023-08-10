//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//


#include "MediaPipePoseTrackerComponent.h"
#include "MediaPipeConnector.h"


#include "IPoseTrackerPlugin.h"


UMediaPipePoseTrackerComponent::UMediaPipePoseTrackerComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Make sure this component ticks
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bAutoActivate = true;
	bTickInEditor = true;
	bAutoActivate = true;
	
}

void UMediaPipePoseTrackerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction){
	IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->DeltaTime = DeltaTime;
}


void UMediaPipePoseTrackerComponent::GetPoseLandmarksFromRenderTarget2D(UTextureRenderTarget2D* TextureRenderTarget) {
	if (TextureRenderTarget==nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->TextureRenderTarget2DToUint8(TextureRenderTarget);

	//Process the raw frame
	int32 SizeX = TextureRenderTarget->SizeX;
	int32 SizeY = TextureRenderTarget->SizeY;

	IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}

void UMediaPipePoseTrackerComponent::GetPoseLandmarksFromTexture2D(UTexture2D* Texture) {
	if (Texture == nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->Texture2DToUint8(Texture);

	//Process the raw frame
	int32 SizeX = Texture->GetSizeX();
	int32 SizeY = Texture->GetSizeY();

	IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}

void UMediaPipePoseTrackerComponent::GetPoseLandmarksFromMediaTexture(UMediaTexture* Texture) {
	if (Texture == nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->MediaTextureToUint8(Texture);

	//Process the raw frame
	int32 SizeX = Texture->GetWidth();
	int32 SizeY = Texture->GetHeight();

	IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}




bool UMediaPipePoseTrackerComponent::InitGraph() {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->InitGraph();
}

bool UMediaPipePoseTrackerComponent::ShutdownGraph() {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->ShutdownGraph();
}

FVector UMediaPipePoseTrackerComponent::GetLandmarkPosition(int ID, bool bDepth) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkPosition(ID, bDepth);
}

FVector2D UMediaPipePoseTrackerComponent::GetLandmarkPosition2D(int ID) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkPosition2D(ID);
}

FRotator UMediaPipePoseTrackerComponent::GetLandmarkRotation(EJointType Joint) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkRotation(Joint);
}

float UMediaPipePoseTrackerComponent::GetLandmarkVisibility(int ID) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkVisibility(ID);
}

float UMediaPipePoseTrackerComponent::GetLandmarkPresence(int ID) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkPresence(ID);
}

float UMediaPipePoseTrackerComponent::GetSkeletonHealth() {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->GetSkeletonHealth();
}

void UMediaPipePoseTrackerComponent::DrawLine(FVector StartLocation, FVector EndLocation, FLinearColor Color, uint8 DepthPriority, float Thickness, float LifeTime) {
	GetWorld()->LineBatcher->DrawLine(StartLocation, EndLocation, Color, 1, Thickness, LifeTime);
}

void UMediaPipePoseTrackerComponent::DrawPoint(FVector Position, FLinearColor Color, float PointSize, uint8 DepthPriority, float LifeTime) {
	GetWorld()->LineBatcher->DrawPoint(Position, Color, PointSize, DepthPriority, LifeTime);
}

void UMediaPipePoseTrackerComponent::SetHFoV(float Degrees) {
	IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->HFoV = Degrees;
}

float UMediaPipePoseTrackerComponent::GetHFoV(float Degrees) {
	return IPoseTrackerPlugin::GetMediaPipeConnectorSafe()->HFoV;
}


void UMediaPipePoseTrackerComponent::DrawCylinder(FVector const& Start, FVector const& End, float Radius, int32 Segments, FLinearColor const& Color, float LifeTime, uint8 DepthPriority, float Thickness) {
	// From "/Engine/Source/Runtime/Engine/Private/DrawDebugHelpers.cpp"

	if (ULineBatchComponent* const LineBatcher = GetWorld()->LineBatcher)
	{
		// Need at least 4 segments
		Segments = FMath::Max(Segments, 4);

		// Rotate a point around axis to form cylinder segments
		FVector Segment;
		FVector P1, P2, P3, P4;
		const float AngleInc = 360.f / Segments;
		float Angle = AngleInc;

		// Default for Axis is up
		FVector Axis = (End - Start).GetSafeNormal();
		if (Axis.IsZero())
		{
			Axis = FVector(0.f, 0.f, 1.f);
		}

		FVector Perpendicular;
		FVector Dummy;

		Axis.FindBestAxisVectors(Perpendicular, Dummy);

		Segment = Perpendicular.RotateAngleAxis(0, Axis) * Radius;
		P1 = Segment + Start;
		P3 = Segment + End;

		while (Segments--)
		{
			Segment = Perpendicular.RotateAngleAxis(Angle, Axis) * Radius;
			P2 = Segment + Start;
			P4 = Segment + End;

			LineBatcher->DrawLine(P2, P4, Color, DepthPriority, Thickness, LifeTime);
			LineBatcher->DrawLine(P1, P2, Color, DepthPriority, Thickness, LifeTime);
			LineBatcher->DrawLine(P3, P4, Color, DepthPriority, Thickness, LifeTime);

			P1 = P2;
			P3 = P4;
			Angle += AngleInc;
		}
	}
}


