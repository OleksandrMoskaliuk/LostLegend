// Fill out your copyright notice in the Description page of Project Settings.

#include "RevengerPlayerController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "RevengerCharacter.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ARevengerPlayerController::ARevengerPlayerController()
{
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
    CachedDestination = FVector::ZeroVector;
    FollowTime = 0.f;
    bNewGoal = false;
}

void ARevengerPlayerController::BeginPlay()
{
    // Call the base class
    Super::BeginPlay();

    // Add Input Mapping Context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
            GetLocalPlayer())) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void ARevengerPlayerController::Tick(float DeltaTime)
{
    /*APawn* ControlledPawn = GetPawn();
    if (ControlledPawn != nullptr && bNewGoal) {
      ControlledPawn->AddMovementInput(CachedDestination, 1.0, false);
    }
    if (bNewGoal) {
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
        }*/
}

void ARevengerPlayerController::SetupInputComponent()
{
    // set up gameplay key bindings
    Super::SetupInputComponent();

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
        // Setup mouse input events
        EnhancedInputComponent->BindAction(
            SetDestinationClickAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnInputStarted);
        EnhancedInputComponent->BindAction(
            SetDestinationClickAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnSetDestinationTriggered);
        EnhancedInputComponent->BindAction(
            SetDestinationClickAction, ETriggerEvent::Completed, this,
            &ARevengerPlayerController::OnSetDestinationReleased);
        EnhancedInputComponent->BindAction(
            SetDestinationClickAction, ETriggerEvent::Canceled, this,
            &ARevengerPlayerController::OnSetDestinationReleased);
    } else {
        UE_LOG(
            LogTemplateCharacter, Error,
            TEXT("'%s' Failed to find an Enhanced Input Component! This template "
                 "is built to use the Enhanced Input system. If you intend to use "
                 "the legacy system, then you will need to update this C++ file."),
            *GetNameSafe(this));
    }
}

void ARevengerPlayerController::OnInputStarted()
{
    StopMovement();
}

// Triggered every frame when the input is held down
void ARevengerPlayerController::OnSetDestinationTriggered()
{
    // We flag that the input is being pressed
    FollowTime += GetWorld()->GetDeltaSeconds();

    GEngine->AddOnScreenDebugMessage(-2, 1.2f, FColor::Magenta, FString::SanitizeFloat(FollowTime));

    // We look for the location in the world where the player has pressed the
    // input
    FHitResult Hit;
    bool bHitSuccessful = false;

    bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

    // If we hit a surface, cache the location
    if (bHitSuccessful) {
        CachedDestination = Hit.Location;
        bNewGoal = true;
    }

    // Move towards mouse pointer or touch
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn != nullptr) {
        FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation())
                                     .GetSafeNormal();
        ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
    }
}

void ARevengerPlayerController::OnSetDestinationReleased()
{
    // If it was a short press
    GEngine->AddOnScreenDebugMessage(0, 1.2f, FColor::Red, "Released");
    if (FollowTime <= ShortPressThreshold) {
        // WSimpleMoveToLocation is a part of NavigationSystem it will not work untill NavMeshBoundsVolume will be added to map
        // After adding press press 'P' button to check walkable ground
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
        // Spawn cursor effect
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, FXCursor, CachedDestination, FRotator::ZeroRotator,
            FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
    }
    FollowTime = 0.f;
    GEngine->AddOnScreenDebugMessage(-2, 1.2f, FColor::Magenta, FString::SanitizeFloat(this->FollowTime));
}
