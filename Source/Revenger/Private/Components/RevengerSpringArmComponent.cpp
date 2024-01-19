// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RevengerSpringArmComponent.h"

// Push value to array for processing arm length
void URevengerSpringArmComponent::PullOrPush(float Value)
{
    ZoomSequence.Push(Value);
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
    TargetArmLength = 1000;
    TargetArmLengthStartDistance = TargetArmLength;
}

void URevengerSpringArmComponent::InterpolateTargetArmLength(float From, float To)
{

    if (TimerManager && TimerManager->IsTimerActive(InterpolateTargetArmLengthTimer)) {
        LerpTargetArmLengthTimerAlpha = 0.f;
        TimerManager->ClearTimer(InterpolateTargetArmLengthTimer);
        //PrimaryComponentTick.bCanEverTick = true;
    }

    if (TimerManager) {
        TargetArmLengthStartDistance = From;
        NewTargetArmLengthDistance = To;
        // Do not work
        /*TimerManager->SetTimer(
            InterpolateTargetArmLengthTimer, this,
            &UDEV_SpringArmComponent::InterpolateTargetArmLengthHandler, 60.f,
            true);*/
        TimerManager->SetTimer(InterpolateTargetArmLengthTimer,
            InterpolateTargetArmLengthDelegate, 0.016f, true);
       
    } else {
        GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Red, "No timer in spring arm component!");
    }
}

void URevengerSpringArmComponent::InterpolateTargetArmLengthHandler()
{
    if (LerpTargetArmLengthTimerAlpha >= 1.f) {
        LerpTargetArmLengthTimerAlpha = 0.f;
        if (TimerManager) {
            TimerManager->ClearTimer(InterpolateTargetArmLengthTimer);
        }
        // SetComponentTickEnabled(false);
    } else {
        // If set to false TargetArmLength can't be changed in runtime
        // SetComponentTickEnabled(true);
        this->TargetArmLength = FMath::Lerp(TargetArmLengthStartDistance, NewTargetArmLengthDistance,
            LerpTargetArmLengthTimerAlpha);
        LerpTargetArmLengthTimerAlpha += 0.01f;
        // PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

void URevengerSpringArmComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    GEngine->AddOnScreenDebugMessage(-134, 3.f, FColor::White, "Elements = " + FString::SanitizeFloat(ZoomSequence.Num()));
    if (TimerManager 
        && !(TimerManager->IsTimerActive(InterpolateTargetArmLengthTimer))
        && (ZoomSequence.Num() > 0))
    {     
        float From = this->TargetArmLength;
        float To = this->TargetArmLength + (120 * ZoomSequence.Pop());
        // Clamp distance to player
        if (To <= MaxCameraDistanceToCharacter || To >= MinCameraDistanceToCharacter) {
           // GEngine->AddOnScreenDebugMessage(145, 0.5f, FColor::Red, "Input = " + FString::SanitizeFloat(To));
            this->InterpolateTargetArmLength(From, To);
        }
    }
}
