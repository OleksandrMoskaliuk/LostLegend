// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "GameFramework/SpringArmComponent.h" 
#include "RevengerSpringArmComponent.generated.h"
/**
 * 
 */
UCLASS()
class REVENGER_API URevengerSpringArmComponent : public USpringArmComponent
{
    GENERATED_BODY()
public:
    UFUNCTION()
    void PullOrPush(float Value);
    URevengerSpringArmComponent();
    void BeginPlay() override;
    void InterpolateTargetArmLength(float From, float To);
    UFUNCTION()
    void InterpolateTargetArmLengthHandler();
    void TickComponent(float DeltaTime, enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    float MaxCameraDistanceToCharacter = 2000;
    float MinCameraDistanceToCharacter = 100;

private:
    float TargetArmLengthStartDistance;
    float NewTargetArmLengthDistance;
    // from 0.0 to 1;
    FTimerManager* TimerManager;
    float LerpTargetArmLengthTimerAlpha;
    FTimerHandle InterpolateTargetArmLengthTimer;
    FTimerDelegate InterpolateTargetArmLengthDelegate;
    TArray<float> ZoomSequence;
	
};
