// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "RevengerSpringArmComponent.generated.h"
/**
 *
 */
UCLASS()
class REVENGER_API URevengerSpringArmComponent : public USpringArmComponent {
    GENERATED_BODY()
public:
    URevengerSpringArmComponent();

    UFUNCTION()
    void ZoomCamera(float Value);

    void BeginPlay() override;

    UFUNCTION()
    void InterpolateTargetArmLengthHandler();

    void TickComponent(float DeltaTime, enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Push or pull spring arm step. Increse camera zoom speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "0.01", ClampMax = "1", UIMin = "0.01", UIMax = "1"))
    float ZoomCameraStep = 0.1;

    /** Push or pull spring arm Interpolation speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "5"))
    float ZoomInterpolationSpeed = 1;

    /** Max camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "1000", ClampMax = "5000", UIMin = "1000", UIMax = "5000"))
    float MaxZoomDistance = 2000;

    /** Min camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "100", ClampMax = "3000", UIMin = "100", UIMax = "3000"))
    float MinZoomDistance = 200;

    /** Camera distance to pawn from 0 to 1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Control", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float CameraZoom = 0;

private:
    float PreviousDistance = 0;
    // from 0.0 to 1;
    FTimerDelegate InterpolateTargetArmLengthDelegate;
    FTimerHandle ZoomTimerHandler;
    FTimerManager* TimerManager;
};
