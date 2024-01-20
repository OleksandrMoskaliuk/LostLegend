// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RevengerSpringArmComponent.h"

// Push value to array for processing arm length
// Value resonable range from -1 to 1;
void URevengerSpringArmComponent::ZoomCamera(float Value)
{
    CameraZoom = FMath::Clamp(CameraZoom + (Value * ZoomCameraStep * -1.f), 0.f, 1.f);
    if (TimerManager && !(TimerManager->IsTimerActive(ZoomTimerHandler))) {
        TimerManager->ClearTimer(ZoomTimerHandler);
        TimerManager->SetTimer(ZoomTimerHandler,
            InterpolateTargetArmLengthDelegate, 0.015f, true);
    }
}

URevengerSpringArmComponent::URevengerSpringArmComponent()
{
    TimerManager = nullptr;
}

void URevengerSpringArmComponent::BeginPlay()
{
    Super::BeginPlay();
    TimerManager = &(GetWorld()->GetTimerManager());

    InterpolateTargetArmLengthDelegate.BindUFunction(
        this, "InterpolateTargetArmLengthHandler");

    // Calculate initial CameraZoom based on initial TargetArmLength
    float InitialTargetArmLength = this->TargetArmLength;
    CameraZoom = FMath::GetMappedRangeValueClamped(FVector2D(MinZoomDistance, MaxZoomDistance), FVector2D(0.0f, 1.0f), InitialTargetArmLength);
}

void URevengerSpringArmComponent::InterpolateTargetArmLengthHandler()
{
    PreviousDistance = this->TargetArmLength;
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float From = this->TargetArmLength;
    float To = FMath::Lerp(MinZoomDistance, MaxZoomDistance,
        CameraZoom);
    this->TargetArmLength = FMath::Lerp(From, To,
        ZoomInterpolationSpeed * DeltaTime);

    // Stop timer if distance is same as in previous time
    if (FMath::IsNearlyEqual(PreviousDistance, TargetArmLength, KINDA_SMALL_NUMBER)) {
        TimerManager->ClearTimer(ZoomTimerHandler);
    }
}

void URevengerSpringArmComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
