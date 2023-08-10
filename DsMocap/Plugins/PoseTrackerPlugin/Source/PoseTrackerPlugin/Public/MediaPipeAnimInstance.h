//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once

#include "Animation/AnimInstance.h"
#include "MediaPipeAnimInstance.generated.h"

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class UMediaPipeAnimInstance : public UAnimInstance
{

	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker" )
	FVector GetLandmarkPosition(int ID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
	FVector2D GetLandmarkPosition2D(int ID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
	FRotator GetLandmarkRotation(EJointType JointType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pose tracker")
	float GetDistance();

	UFUNCTION(BlueprintCallable, Category = "Pose tracker")
	void SetShoulderWidthReference(float Value);

	UFUNCTION(BlueprintCallable, Category = "Pose tracker")
	void SetHipWidthReference(float Value);

	UFUNCTION(BlueprintCallable, Category = "Pose tracker")
	void SetBodyYaw(float Value);


protected:

private:

	FMediaPipePoseConnector* MediaPipeConnector;



};


