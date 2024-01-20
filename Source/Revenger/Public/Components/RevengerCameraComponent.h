// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "RevengerCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class REVENGER_API URevengerCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
  public:
        void MoveCameraForward(float Amount);
        void MoveCameraBackward(float Amount);
        void MoveCameraLeft(float Amount);
        void MoveCameraRight(float Amount);
        void RotateCameraLeft(float Amount);
        void RotateCameraRight(float Amount);
};
