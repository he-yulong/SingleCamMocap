//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#include "JointOrientationCalculator.h"
#include "MediaPipeConnector.h"



//-------------------------------------------------------------------------------------
JointOrientationCalculator::JointOrientationCalculator(void) :
	isMirror(false)
{

}

//-------------------------------------------------------------------------------------
JointOrientationCalculator::~JointOrientationCalculator(void)
{

}


//-------------------------------------------------------------------------------------

void JointOrientationCalculator::SetConnector(FMediaPipePoseConnector* C) {
	this->Connector = C;
}

// check whether the two vectors lie in the same line (and also parallel)
bool JointOrientationCalculator::AreNearCollinear(FVector v1, FVector v2)
{
	v1.Normalize();
	v2.Normalize();

	float product = FVector::DotProduct(v1, v2);

	return product >= 0.8f;		// [0, 1]
}

//-------------------------------------------------------------------------------------
FVector JointOrientationCalculator::GetDirection(int p1, int p2)
{
	FVector p1Vec = this->Connector->GetLandmarkPosition(p1);
	FVector p2Vec = this->Connector->GetLandmarkPosition(p2);
	

	return p2Vec - p1Vec;
}



//-------------------------------------------------------------------------------------
FQuat JointOrientationCalculator::GetSkeletonJointOrientation(EJointType Joint)
{
	FQuat Orientation;
	
	FVector vx = FVector::XAxisVector;
	FVector vy = FVector::YAxisVector;
	FVector vz = FVector::ZAxisVector;

	switch (Joint)
	{

	case EJointType::SpineBase:
	{

		vx = this->GetDirection(23, 24);
		vx.Normalize();

		Orientation = FRotationMatrix::MakeFromXZ(vx, vz).ToQuat();
	}
	break;

	case EJointType::Head:
	{
		//Eyes
		vx = this->GetDirection(2, 5);
		vx.Normalize();

		
		
		//Nose
		FVector Nose = this->Connector->GetLandmarkPosition(0);
		
		
		//Shoulder Center
		FVector ShoulderLeft = this->Connector->GetLandmarkPosition(11);
		FVector ShoulderRight = this->Connector->GetLandmarkPosition(12);
		FVector ShoulderCenter = (ShoulderLeft + ShoulderRight) / 2;

		vz = Nose - ShoulderCenter;
		vz.Normalize();

		Orientation = FRotationMatrix::MakeFromXZ(vx, vz).ToQuat();
	}
	break;

	case EJointType::SpineShoulder:	
	{
		//Hip Center
		FVector HipLeft = this->Connector->GetLandmarkPosition(23);
		FVector HipRight = this->Connector->GetLandmarkPosition(24);

		FVector HipCenter = (HipLeft + HipRight) / 2;

		//Shoulder Center
		FVector ShoulderLeft = this->Connector->GetLandmarkPosition(11);
		FVector ShoulderRight = this->Connector->GetLandmarkPosition(12);

		FVector ShoulderCenter = (ShoulderLeft + ShoulderRight) / 2;
		
		vz = HipCenter - ShoulderCenter;
		vz.Normalize();
		vx = this->GetDirection(11, 12);
		vx.Normalize();
		
		Orientation = FRotationMatrix::MakeFromXZ(vx, vz).ToQuat();
	}
	break;

	
	case EJointType::ShoulderLeft:
	{
		
		vx = this->GetDirection(11, 12);
		vx.Normalize();

		vz = this->GetDirection(13, 11);
		vz.Normalize();
		
		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();

	}
	break;

	case EJointType::ShoulderRight:
	{
		vx = this->GetDirection(11, 12);
		vx.Normalize();

		vz = this->GetDirection(14, 12);
		vz.Normalize();

		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();

	}
	break;
	

	case EJointType::ElbowLeft:
	{
		vz = this->GetDirection(15, 13);
		vz.Normalize();
		
		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;

	
	case EJointType::ElbowRight:
	{
		vz = this->GetDirection(16, 14);
		vz.Normalize();

		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;

	case EJointType::HipLeft:
	{
		vz = this->GetDirection(25, 23);
		vz.Normalize();
		vx = this->GetDirection(23, 24);
		vx.Normalize();

		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;

	case EJointType::HipRight:
	{
		vz = this->GetDirection(26, 24);
		vz.Normalize();
		vx = this->GetDirection(23, 24);
		vx.Normalize();

		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;

	case EJointType::KneeLeft:			
	{
		vz = this->GetDirection(27, 25);
		vz.Normalize();

		vx = this->GetDirection(23, 24);
		vx.Normalize();
		
		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;

	case EJointType::KneeRight:
	{
		vz = this->GetDirection(28, 26);
		vz.Normalize();

		vx = this->GetDirection(23, 24);
		vx.Normalize();

		Orientation = FRotationMatrix::MakeFromZX(vz, vx).ToQuat();
	}
	break;
}
	

	return Orientation;
}

