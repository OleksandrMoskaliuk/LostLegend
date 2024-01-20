//Copyright (c) [2024] [Alex Mercer]
//
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//1. Redistributions of source code must retain the above copyright notice,
//   this list of conditions, and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions, and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
//For inquiries or more information, contact: @Dru9Dealer on Twitter.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.

#include "RevengerPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "RevengerCharacter.h"
// vectors calculation
#include "Kismet/KismetMathLibrary.h"
// Get GameMode instance
#include "Kismet/GameplayStatics.h"
// Find path
#include "GameFramework/PawnMovementComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
// Handle camera movement
#include "Components/RevengerCameraComponent.h"
#include "Components/RevengerCameraPawn.h"
#include "Components/RevengerSpringArmComponent.h"
#include "Revenger/RevengerGameModeBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ARevengerPlayerController::ARevengerPlayerController()
{
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
    CachedDestination = FVector::ZeroVector;
    NextClosestGoal = FVector::ZeroVector;
    FollowTime = 0.f;
    bNewGoal = false;
    StateTransitionTime = 0;
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
    // Get pawns
    DefaultPawn = Cast<ARevengerCharacter>(GetPawn());
    SpawnRTSCamera();
    if (CameraPawn) 
    {
        this->Possess(CameraPawn);
    }
}

void ARevengerPlayerController::Tick(float DeltaTime)
{
    if (CachedDestination != FVector::ZeroVector) {
        UpdateGoal();
        RotateToGoal(NextClosestGoal, DeltaTime);
        MoveToGoal();
    }
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
        // Camera zoom
        EnhancedInputComponent->BindAction(
            SetCameraZoomAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnCameraZoom);
        // Camera move
        EnhancedInputComponent->BindAction(
            MoveCameraUpAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnMoveCameraForward);
        EnhancedInputComponent->BindAction(
            MoveCameraUpAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnMoveCameraForward);

        EnhancedInputComponent->BindAction(
            MoveCameraDownAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnMoveCameraBackward);
        EnhancedInputComponent->BindAction(
            MoveCameraDownAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnMoveCameraBackward);

        EnhancedInputComponent->BindAction(
            MoveCameraLeftAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnMoveCameraLeft);
        EnhancedInputComponent->BindAction(
            MoveCameraLeftAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnMoveCameraLeft);

        EnhancedInputComponent->BindAction(
            MoveCameraRightAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnMoveCameraRight);
        EnhancedInputComponent->BindAction(
            MoveCameraRightAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnMoveCameraRight);
        // Camera rotate
        EnhancedInputComponent->BindAction(
            RotateCameraLeftAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnRotateCameraLeft);
        EnhancedInputComponent->BindAction(
            RotateCameraLeftAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnRotateCameraLeft);

        EnhancedInputComponent->BindAction(
            RotateCameraRightAction, ETriggerEvent::Started, this,
            &ARevengerPlayerController::OnRotateCameraRight);
        EnhancedInputComponent->BindAction(
            RotateCameraRightAction, ETriggerEvent::Triggered, this,
            &ARevengerPlayerController::OnRotateCameraRight);

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

    // TO DO: Stop prevoius movement using this function
}

// Triggered every frame when the input is held down
void ARevengerPlayerController::OnSetDestinationTriggered()
{
    // We flag that the input is being pressed
    FollowTime += GetWorld()->GetDeltaSeconds();

    // GEngine->AddOnScreenDebugMessage(-2, 1.2f, FColor::Magenta, FString::SanitizeFloat(FollowTime));

    // We look for the location in the world where the player has pressed the
    // input
    // FHitResult Hit;
    // bool bHitSuccessful = false;

    // bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

    //// If we hit a surface, cache the location
    // if (bHitSuccessful) {
    //     CachedDestination = Hit.Location;
    //     bNewGoal = true;
    //     // get navigation goals using navigation volume
    //
    // }
    //
    //// Move towards mouse pointer or touch
    // APawn* DefaultPawn = GetPawn();
    // if (ControlledPawn != nullptr && bHitSuccessful) {
    //     FVector PlayerLocation = ControlledPawn->GetActorLocation();
    //    UNavigationPath *Path =  UNavigationSystemV1::FindPathToActorSynchronously(this, CachedDestination, ControlledPawn);
    //     for (FVector Ph : Path->PathPoints)
    //     {
    //        DrawDebugSphere(GetWorld(), Ph, 20.f, 10, FColor::Red);
    //     }
    //     FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation())
    //                                  .GetSafeNormal();
    //    // ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
    // }
}

void ARevengerPlayerController::OnSetDestinationReleased()
{
    // If it was a short press
    if (FollowTime <= ShortPressThreshold && DefaultPawn) {
        // WSimpleMoveToLocation is a part of NavigationSystem it will not work untill NavMeshBoundsVolume will be added to map
        // After adding press press 'P' button to check walkable ground
        FHitResult Hit;
        bool bHitSuccessful = false;
        bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

        // If we hit a surface, cache the location
        if (bHitSuccessful) {
            CachedDestination = Hit.Location;
            // Nex goal update only when player Idle
            DefaultPawn->CharacterState = ECharacterState::IDLE;
        }
        // Spawn cursor effect
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, FXCursor, CachedDestination, FRotator::ZeroRotator,
            FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
    }
    FollowTime = 0.f;
}

void ARevengerPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
    if (CameraPawn) {
        CameraPawn->GetCameraBoom()->ZoomCamera(Value.Get<float>());
    }
}

void ARevengerPlayerController::OnMoveCameraForward(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::OnMoveCameraBackward(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::OnMoveCameraLeft(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::OnMoveCameraRight(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::OnRotateCameraLeft(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::OnRotateCameraRight(const FInputActionValue& Value)
{
}

void ARevengerPlayerController::UpdateGoal()
{
    // Update goal for using move and other logick only when player idle
    if (DefaultPawn) {
        FVector PlayerLocation = DefaultPawn->GetActorLocation();
        UNavigationPath* Path = UNavigationSystemV1::FindPathToActorSynchronously(this, CachedDestination, DefaultPawn, DefaultPawn->MinimalDistanceToMove);
        for (FVector Ph : Path->PathPoints) {
            DrawDebugSphere(GetWorld(), Ph, 20.f, 10, FColor::Red, false, 1.f);
        }
        // Firs point if navigation is last point of array, so next point to follow will be:
        NextClosestGoal = Path->PathPoints[Path->PathPoints.Max() - 2];
        // Draw closest goal
        DrawDebugSphere(GetWorld(), NextClosestGoal, 50.f, 10, FColor::Green, false, 1.f);
    }
}

void ARevengerPlayerController::MoveToGoal()
{
    if (DefaultPawn) {
        float DistanceToClosestGoal = FVector::DistXY(DefaultPawn->GetActorLocation(), NextClosestGoal);
        float DistanceToMainGoal = FVector::DistXY(DefaultPawn->GetActorLocation(), CachedDestination);

        // Main player stop
        if (DistanceToMainGoal <= DefaultPawn->MinimalDistanceToMove) {
            CachedDestination = FVector::ZeroVector;
            DefaultPawn->CharacterState = ECharacterState::IDLE;
            return;
        }

        // Handle acceleration
        if (DefaultPawn->CharacterState == ECharacterState::SLOW_RUN
            || DefaultPawn->CharacterState == ECharacterState::MEDIUM_RUN
            || DefaultPawn->CharacterState == ECharacterState::FAST_RUN
            || DefaultPawn->CharacterState == ECharacterState::WALK) {
            StateTransitionTime += GetWorld()->GetDeltaSeconds();
        } else {
            StateTransitionTime = 0.f;
        }

        // Slow down when goal is close
        if (DefaultPawn->CharacterState == ECharacterState::SLOW_RUN
            || DefaultPawn->CharacterState == ECharacterState::MEDIUM_RUN
            || DefaultPawn->CharacterState == ECharacterState::FAST_RUN
                && DistanceToClosestGoal < 120.f) {
            DefaultPawn->CharacterState = ECharacterState::WALK;
        }

        // Start moving
        if (DistanceToClosestGoal > DefaultPawn->MinimalDistanceToMove
                && DefaultPawn->CharacterState == ECharacterState::IDLE
            || DefaultPawn->CharacterState == ECharacterState::SLOW_RUN) {
            DefaultPawn->CharacterState = ECharacterState::WALK;
        }

        // Slow run
        if (DistanceToClosestGoal > 250.f
            && DefaultPawn->CharacterState == ECharacterState::WALK
            && StateTransitionTime > DefaultPawn->MinimalTransitionStateTime) {
            DefaultPawn->CharacterState = ECharacterState::SLOW_RUN;
        }
    }

    return;
}

void ARevengerPlayerController::RotateToGoal(const FVector GoalLocation, float DeltaTime)
{
    if (DefaultPawn) {
        // Prevent acces violation during posses another pawn
        float DistanceToGoal = 0.f;
        float PlayerSpeed = 0.f;
        if (DefaultPawn) {
            DistanceToGoal = FVector::DistXY(GoalLocation, DefaultPawn->GetActorLocation());
            UPawnMovementComponent* MoveComponent = DefaultPawn->GetMovementComponent();
            PlayerSpeed = MoveComponent->Velocity.Size();
        }
        if (true) {
            GEngine->AddOnScreenDebugMessage(8, 2.f, FColor::Red, "Distance to goal = " + FString::SanitizeFloat(DistanceToGoal));
            GEngine->AddOnScreenDebugMessage(10, 2.f, FColor::Blue, "Speed = " + FString::SanitizeFloat(PlayerSpeed));
        }
        // Lambda for precise Pawn rotation. Used only when player move
        auto PreciseRotate = [&](float AcceptableRotationThreashold = 30) {
            // FIND CLOSEST ROTATION TO GOAL
            // Get the current location of the character
            FVector CurrentLocation = DefaultPawn->GetActorLocation();

            // DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 10, FColor::Yellow, false, 5.f);

            // Calculate the direction vector to the goal
            FVector DirectionToGoal = GoalLocation - CurrentLocation;
            DirectionToGoal.Normalize();

            // Get the forward vector of the character
            FVector ForwardVector = DefaultPawn->GetActorForwardVector();

            // Calculate the angle between the current forward vector and the direction to the goal
            float AngleToGoal = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToGoal)));

            // Check if the rotation is close enough to the goal
            if (FMath::Abs(AngleToGoal) > AcceptableRotationThreashold) {
                // Determine if the rotation is towards the left or right
                FVector CrossProduct = FVector::CrossProduct(ForwardVector, DirectionToGoal);
                float Sign = FMath::Sign(CrossProduct.Z);
                // Root motion rotation relative
                UPawnMovementComponent* MoveComponent = DefaultPawn->GetMovementComponent();
                float PlayerSpeed = MoveComponent->Velocity.Size();
                float MovingRotationSpeed = 40;
                if (DefaultPawn != nullptr) {
                    MovingRotationSpeed = DefaultPawn->MovingRotationSpeed;
                }
                if (Sign > 0) {
                    FQuat Rotattion = FQuat(FRotator(0, MovingRotationSpeed * DeltaTime, 0));
                    DefaultPawn->AddActorLocalRotation(Rotattion, false, 0);
                } else if (Sign < 0) {
                    FQuat Rotattion = FQuat(FRotator(0, -MovingRotationSpeed * DeltaTime, 0));
                    DefaultPawn->AddActorLocalRotation(Rotattion, false, 0);
                }
            }
        };

        // Rotation using root motion
        auto RootMotionRotation = [&](float AcceptableRotationThreashold = 60) {
            if (DefaultPawn != nullptr) {
                // FIND CLOSEST ROTATION TO GOAL
                // Get the current location of the character
                FVector CurrentLocation = DefaultPawn->GetActorLocation();

                // DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 10, FColor::Yellow, false, 5.f);

                // Calculate the direction vector to the goal
                FVector DirectionToGoal = GoalLocation - CurrentLocation;
                DirectionToGoal.Normalize();

                // Get the forward vector of the character
                FVector ForwardVector = DefaultPawn->GetActorForwardVector();

                // Calculate the angle between the current forward vector and the direction to the goal
                float AngleToGoal = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToGoal)));

                // Check if the rotation is close enough to the goal
                if (FMath::Abs(AngleToGoal) > AcceptableRotationThreashold) {
                    // Determine if the rotation is towards the left or right
                    FVector CrossProduct = FVector::CrossProduct(ForwardVector, DirectionToGoal);
                    float Sign = FMath::Sign(CrossProduct.Z);
                    FString RotationDirection = (Sign > 0) ? TEXT("Right") : TEXT("Left");
                    // Root motion rotation relative
                    if (DefaultPawn != nullptr && PlayerSpeed <= 0) {
                        // turn right
                        if (Sign > 0) {
                            DefaultPawn->CharacterState = ECharacterState::TURN_RIGHT;
                            // turn left
                        } else if (Sign < 0) {
                            DefaultPawn->CharacterState = ECharacterState::TURN_LEFT;
                        }
                    }
                } else if (DefaultPawn != nullptr) {
                    // Update turn variables
                    DefaultPawn->CharacterState = ECharacterState::IDLE;
                }
            }
        };

        // When player moving , use only precise rotation
        if (DefaultPawn && PlayerSpeed > 100.f) {
            // Fix pawn rotation flickering
            if (DistanceToGoal > 400) {
                PreciseRotate(10);
                return;
            }
            if (DistanceToGoal > 300) {
                PreciseRotate(20);
                return;
            }
            if (DistanceToGoal > 200) {
                PreciseRotate(30);
                return;
            }
            if (DistanceToGoal > 100) {
                PreciseRotate(40);
                return;
            }
        }

        if (DefaultPawn && DistanceToGoal < DefaultPawn->MinimalDistanceToMove) {
            return;
        }

        // Goal is near to player, use root motion
        float AllowedDistance = 300;
        if (DistanceToGoal <= AllowedDistance) {
            RootMotionRotation(60);
            return;
        }

        // Goal is far from player, use root motion
        AllowedDistance = 600;
        if (DistanceToGoal >= AllowedDistance) {
            RootMotionRotation(30);
            return;
        } else {
            RootMotionRotation(20);
            return;
        }
    }
}

void ARevengerPlayerController::SpawnRTSCamera()
{
    if (UWorld* World = GetWorld()) {
        // Get the player controller
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);

        if (PlayerController) {
            // Get the location and rotation of the player controller
            FVector SpawnRTSCameraLocation;
            FRotator SpawnRTSCameraRotation;
            PlayerController->GetPlayerViewPoint(SpawnRTSCameraLocation, SpawnRTSCameraRotation);

            // Set the spawn parameters
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = PlayerController;
            SpawnParams.Instigator = NULL;

            // Spawn the second pawn at the calculated location
            CameraPawn = World->SpawnActor<ARevengerCameraPawn>(RTS_CameraPawn, SpawnRTSCameraLocation, SpawnRTSCameraRotation, SpawnParams);
        }
    }
}
