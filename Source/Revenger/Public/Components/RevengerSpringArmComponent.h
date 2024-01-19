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
    URevengerSpringArmComponent();

    UFUNCTION()
    void PullOrPush(float Value);

    void BeginPlay() override;

    UFUNCTION()
    void InterpolateTargetArmLengthHandler();

    void TickComponent(float DeltaTime, enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Push or pull spring arm step. Increse camera zoom speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "10", ClampMax = "100", UIMin = "10", UIMax = "100"))
    float PushOrPullArmStep = 80;

    /** Push or pull spring arm Interpolation speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "0", ClampMax = "10", UIMin = "0", UIMax = "10"))
    float InterpolationSpeed = 5;

    /** Max camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control")
    float MaxCameraDistanceToCharacter = 2000;

    /** Min camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control")
    float MinCameraDistanceToCharacter = 100;

private:
    float TargetArmLengthStartDistance;
    float NewTargetArmLengthDistance;
    // from 0.0 to 1;
    float LerpTargetArmLengthTimerAlpha;
    // Array with all player saved inputs
    TArray<float> ZoomSequence;
    FTimerDelegate InterpolateTargetArmLengthDelegate;
    FTimerHandle InterpolateTargetArmLengthTimer;
    FTimerManager* TimerManager;
	
};
