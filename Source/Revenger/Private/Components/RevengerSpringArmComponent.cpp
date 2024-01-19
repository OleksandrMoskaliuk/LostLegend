// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RevengerSpringArmComponent.h"

// Push value to array for processing arm length
void URevengerSpringArmComponent::PullOrPush(float Value)
{
    // GEngine->AddOnScreenDebugMessage(-134, 3.f, FColor::White, "Elements = " + FString::SanitizeFloat(ZoomSequence.Num()));
    ZoomSequence.Push(Value);
    if (TimerManager && !(TimerManager->IsTimerActive(InterpolateTargetArmLengthTimer))) {
        TimerManager->ClearTimer(InterpolateTargetArmLengthTimer);
        TimerManager->SetTimer(InterpolateTargetArmLengthTimer,
            InterpolateTargetArmLengthDelegate, 0.010f, true);
    } 
}

URevengerSpringArmComponent::URevengerSpringArmComponent()
    : TargetArmLengthStartDistance(0.f)
    , NewTargetArmLengthDistance(0.f)
    , LerpTargetArmLengthTimerAlpha(0.f)
{
    PrimaryComponentTick.bCanEverTick = true;
    TimerManager = nullptr;
}

void URevengerSpringArmComponent::BeginPlay()
{
    Super::BeginPlay();
    TimerManager = &(GetWorld()->GetTimerManager());
    // Only work through delegate
    InterpolateTargetArmLengthDelegate.BindUFunction(
        this, "InterpolateTargetArmLengthHandler");
    TargetArmLengthStartDistance = this->TargetArmLength;
    NewTargetArmLengthDistance = this->TargetArmLength;
}

void URevengerSpringArmComponent::InterpolateTargetArmLengthHandler()
{
    if (LerpTargetArmLengthTimerAlpha >= 0.99f && ZoomSequence.Num() == 0) {
        TimerManager->ClearTimer(InterpolateTargetArmLengthTimer);
        return;
    }

    if (LerpTargetArmLengthTimerAlpha >= 0.99f && ZoomSequence.Num() > 0) {
        LerpTargetArmLengthTimerAlpha = 0.f;
        float From = this->TargetArmLength;
        float To = this->TargetArmLength + (PushOrPullArmStep * -ZoomSequence.Pop());
        TargetArmLengthStartDistance = From;
        NewTargetArmLengthDistance = To;
        return;
    }

    if (LerpTargetArmLengthTimerAlpha < 0.99f) {
        NewTargetArmLengthDistance = FMath::Clamp(NewTargetArmLengthDistance, MinCameraDistanceToCharacter, MaxCameraDistanceToCharacter);
        this->TargetArmLength = FMath::Lerp(TargetArmLengthStartDistance, NewTargetArmLengthDistance,
            LerpTargetArmLengthTimerAlpha);
        LerpTargetArmLengthTimerAlpha += InterpolationSpeed * GetWorld()->GetDeltaSeconds();
        return;
    }
}

void URevengerSpringArmComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
