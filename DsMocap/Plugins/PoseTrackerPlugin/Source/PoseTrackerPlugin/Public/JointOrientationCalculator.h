//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once


UENUM(BlueprintType)
enum class EJointType : uint8
{
	//Basic joints
	Head  UMETA(DisplayName = "Head"),
	Neck  UMETA(DisplayName = "Neck"),
	ShoulderLeft  UMETA(DisplayName = "Shoulder Left"),
	ShoulderRight  UMETA(DisplayName = "Shoulder Right"),
	ElbowLeft  UMETA(DisplayName = "Elbow Left"),
	ElbowRight  UMETA(DisplayName = "Elbow Right"),
	HandLeft  UMETA(DisplayName = "Hand Left"),
	HandRight  UMETA(DisplayName = "Hand Right"),
	SpineBase  UMETA(DisplayName = "Spine Base"),
	SpineShoulder  UMETA(DisplayName = "Spine Shoulder"),
	HipLeft  UMETA(DisplayName = "Hip Left"),
	HipRight  UMETA(DisplayName = "Hip Right"),
	KneeLeft  UMETA(DisplayName = "Knee Left"),
	KneeRight  UMETA(DisplayName = "Knee Right"),
	FootLeft  UMETA(DisplayName = "Foot Left"),
	FootRight  UMETA(DisplayName = "Foot Right"),

};

class FMediaPipePoseConnector;
class JointOrientationCalculator
{
public:
	JointOrientationCalculator(void);
	virtual ~JointOrientationCalculator(void);

	
	FQuat GetSkeletonJointOrientation(EJointType Joint);
	void SetConnector(FMediaPipePoseConnector* C);

	
protected:
	
	bool isMirror;
	FMediaPipePoseConnector* Connector;

protected:
	bool AreNearCollinear(FVector v1, FVector v2);
	FVector GetDirection(int p1, int p2);

	

};