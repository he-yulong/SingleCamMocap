//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#include "MediaPipeAnimInstance.h"
#include "MediaPipeConnector.h"
#include "IPoseTrackerPlugin.h"

UMediaPipeAnimInstance::UMediaPipeAnimInstance(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MediaPipeConnector = IPoseTrackerPlugin::GetMediaPipeConnectorSafe();

}

FVector UMediaPipeAnimInstance::GetLandmarkPosition(int ID) {
	if (!MediaPipeConnector|| !MediaPipeConnector->bIsInitiated) return FVector::ZeroVector;

	FVector Position = MediaPipeConnector->GetLandmarkPosition(ID);
	if (!Position.ContainsNaN()) return Position;
	return FVector::ZeroVector;
}

FVector2D UMediaPipeAnimInstance::GetLandmarkPosition2D(int ID) {
	if (!MediaPipeConnector || !MediaPipeConnector->bIsInitiated) return FVector2D::ZeroVector;

	FVector2D Position = MediaPipeConnector->GetLandmarkPosition2D(ID);
	if (!Position.ContainsNaN()) return Position;
	return FVector2D::ZeroVector;
}

FRotator UMediaPipeAnimInstance::GetLandmarkRotation(EJointType Joint) {
	
	if (!MediaPipeConnector|| !MediaPipeConnector->bIsInitiated) return FRotator::ZeroRotator;


	FRotator Rot =  MediaPipeConnector->GetLandmarkRotation(Joint);
	if (!Rot.ContainsNaN()) return Rot;
	return FRotator::ZeroRotator;
}

float UMediaPipeAnimInstance::GetDistance() {
	if (!MediaPipeConnector || !MediaPipeConnector->bIsInitiated) return 0.0f;
	return MediaPipeConnector->Distance;
}

void UMediaPipeAnimInstance::SetHipWidthReference(float Value) {
	MediaPipeConnector->HipWidthReference = Value;
}

void UMediaPipeAnimInstance::SetShoulderWidthReference(float Value) {
	MediaPipeConnector->ShoulderWidthReference = Value;
}

void UMediaPipeAnimInstance::SetBodyYaw(float Value) {
	MediaPipeConnector->BodyYaw = Value;
}





