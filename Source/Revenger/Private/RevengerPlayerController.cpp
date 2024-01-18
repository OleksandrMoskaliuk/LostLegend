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
// vectors calculation
#include "Kismet/KismetMathLibrary.h"
// Find path
#include "GameFramework/PawnMovementComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

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
}

void ARevengerPlayerController::Tick(float DeltaTime)
{
    UpdateGoal();
    RotateToGoal(NextClosestGoal, DeltaTime * 2.f);
    MoveToGoal(DeltaTime);
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
    // APawn* ControlledPawn = GetPawn();
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
    APawn* ControlledPawn = GetPawn();
    ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn);
    // If it was a short press
    // GEngine->AddOnScreenDebugMessage(0, 1.2f, FColor::Red, "Released");
    if (FollowTime <= ShortPressThreshold) {
        // WSimpleMoveToLocation is a part of NavigationSystem it will not work untill NavMeshBoundsVolume will be added to map
        // After adding press press 'P' button to check walkable ground
        // UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
        //

        FHitResult Hit;
        bool bHitSuccessful = false;

        bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

        // If we hit a surface, cache the location
        if (bHitSuccessful) {
            CachedDestination = Hit.Location;
        }
        // Spawn cursor effect
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, FXCursor, CachedDestination, FRotator::ZeroRotator,
            FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
    }
    FollowTime = 0.f;
}

void ARevengerPlayerController::UpdateGoal()
{
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn != nullptr) {
        FVector PlayerLocation = ControlledPawn->GetActorLocation();
        ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn);
        // Player reach last goal location
        float IsGoalReachedDistance = 50;
        if (FVector::DistXY(PlayerLocation, CachedDestination) < IsGoalReachedDistance || CachedDestination == FVector::ZeroVector && PlayerCharacter) {
            PlayerCharacter->CharacterState = ECharacterState::IDLE;
            CachedDestination = FVector::ZeroVector;
            return;
        } else if (PlayerCharacter) {

            // Draw path
            UNavigationPath* Path = UNavigationSystemV1::FindPathToActorSynchronously(this, CachedDestination, ControlledPawn, 100.f);
            for (FVector Ph : Path->PathPoints) {
                DrawDebugSphere(GetWorld(), Ph, 20.f, 10, FColor::Red, false, 1.f);
            }            
            // Firs point if navigation is last point of array, so next point to follow will be:
            NextClosestGoal = Path->PathPoints[Path->PathPoints.Max() - 2];
            // Draw closest goal
            DrawDebugSphere(GetWorld(), NextClosestGoal, 50.f, 10, FColor::Green, false, 1.f);
        }
    }
}

void ARevengerPlayerController::MoveToGoal(float DeltaTime)
{
    if (APawn* ControlledPawn = GetPawn()) 
    {
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) 
        {
            float DistanceToClosestGoal = FVector::DistXY(ControlledPawn->GetActorLocation(), NextClosestGoal); 
            // Slow down when goal is close
            bool RunState = PlayerCharacter->CharacterState == ECharacterState::SLOW_RUN
                || PlayerCharacter->CharacterState == ECharacterState::MEDIUM_RUN
                || PlayerCharacter->CharacterState == ECharacterState::FAST_RUN;
            if (PlayerCharacter
                && CachedDestination != FVector::ZeroVector
                && DistanceToClosestGoal < 120.f
                && RunState) 
            {
                PlayerCharacter->CharacterState = ECharacterState::WALK;
            }

            // Start moving
            if (PlayerCharacter 
             && CachedDestination != FVector::ZeroVector
             && PlayerCharacter->CharacterState != ECharacterState::ROTATE
             && DistanceToClosestGoal > PlayerCharacter->MinimalDistanceToMove
             && PlayerCharacter->CharacterState == ECharacterState::IDLE
             || PlayerCharacter->CharacterState == ECharacterState::SLOW_RUN)
            {
                PlayerCharacter->CharacterState = ECharacterState::WALK;
            }

            // Slow run 
            if (PlayerCharacter 
             && CachedDestination != FVector::ZeroVector
             && DistanceToClosestGoal > 400.f
             && PlayerCharacter->CharacterState == ECharacterState::WALK
             && StateTransitionTime > 2.5f)
            {
                PlayerCharacter->CharacterState = ECharacterState::SLOW_RUN;
            }
            if (PlayerCharacter->CharacterState == ECharacterState::IDLE) 
            {
                StateTransitionTime = 0.f;
            }
            StateTransitionTime += DeltaTime;
            GEngine->AddOnScreenDebugMessage(12, 1.5f, FColor::Green, "StateTransitionTime = " + FString::SanitizeFloat(StateTransitionTime));
        }

    }
    return;
}

void ARevengerPlayerController::RotateToGoal(const FVector GoalLocation, float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn);
    float DistanceToGoal = 0.f;
    float AllowedDistance;
    float PlayerSpeed = 0.f;

    if (ControlledPawn) {
        DistanceToGoal = FVector::DistXY(GoalLocation, ControlledPawn->GetActorLocation());
        UPawnMovementComponent* MoveComponent = ControlledPawn->GetMovementComponent();
        PlayerSpeed = MoveComponent->Velocity.Size();
    }
    if (true) {
        GEngine->AddOnScreenDebugMessage(8, 2.f, FColor::Red, "Distance to goal = " + FString::SanitizeFloat(DistanceToGoal));
        GEngine->AddOnScreenDebugMessage(10, 2.f, FColor::Blue, "Speed = " + FString::SanitizeFloat(PlayerSpeed));
    }
    // Lambda for precise Pawn rotation
    auto PreciseRotate = [&](float AcceptableRotationThreashold = 30) {
        // FIND CLOSEST ROTATION TO GOAL
        // Get the current location of the character
        FVector CurrentLocation = ControlledPawn->GetActorLocation();

        // DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 10, FColor::Yellow, false, 5.f);

        // Calculate the direction vector to the goal
        FVector DirectionToGoal = GoalLocation - CurrentLocation;
        DirectionToGoal.Normalize();

        // Get the forward vector of the character
        FVector ForwardVector = ControlledPawn->GetActorForwardVector();

        // Calculate the angle between the current forward vector and the direction to the goal
        float AngleToGoal = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToGoal)));

        // Check if the rotation is close enough to the goal
        if (FMath::Abs(AngleToGoal) > AcceptableRotationThreashold) {
            // Determine if the rotation is towards the left or right
            FVector CrossProduct = FVector::CrossProduct(ForwardVector, DirectionToGoal);
            float Sign = FMath::Sign(CrossProduct.Z);
            // Root motion rotation relative
            UPawnMovementComponent* MoveComponent = ControlledPawn->GetMovementComponent();
            float PlayerSpeed = MoveComponent->Velocity.Size();
            float MovingRotationSpeed = 40;
            if (PlayerCharacter != nullptr) {
                MovingRotationSpeed = PlayerCharacter->MovingRotationSpeed;
            }
            if (Sign > 0) {
                FQuat Rotattion = FQuat(FRotator(0, MovingRotationSpeed * DeltaTime, 0));
                ControlledPawn->AddActorLocalRotation(Rotattion, false, 0);
            } else if (Sign < 0) {
                FQuat Rotattion = FQuat(FRotator(0, -MovingRotationSpeed * DeltaTime, 0));
                ControlledPawn->AddActorLocalRotation(Rotattion, false, 0);
            }
        }
        if (PlayerCharacter != nullptr) {
            PlayerCharacter->TurnLeft = false;
            PlayerCharacter->TurnRight = false;
            PlayerCharacter->CharacterState = ECharacterState::IDLE;
        }
    };

    // Rotation using root motion
    auto RootMotionRotation = [&](float AcceptableRotationThreashold = 60) {
        if (ControlledPawn != nullptr) {
            // FIND CLOSEST ROTATION TO GOAL
            // Get the current location of the character
            FVector CurrentLocation = ControlledPawn->GetActorLocation();

            // DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 10, FColor::Yellow, false, 5.f);

            // Calculate the direction vector to the goal
            FVector DirectionToGoal = GoalLocation - CurrentLocation;
            DirectionToGoal.Normalize();

            // Get the forward vector of the character
            FVector ForwardVector = ControlledPawn->GetActorForwardVector();

            // Calculate the angle between the current forward vector and the direction to the goal
            float AngleToGoal = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToGoal)));

            // Check if the rotation is close enough to the goal
            if (FMath::Abs(AngleToGoal) > AcceptableRotationThreashold) {
                // Determine if the rotation is towards the left or right
                FVector CrossProduct = FVector::CrossProduct(ForwardVector, DirectionToGoal);
                float Sign = FMath::Sign(CrossProduct.Z);
                FString RotationDirection = (Sign > 0) ? TEXT("Right") : TEXT("Left");
                // Root motion rotation relative
                if (PlayerCharacter != nullptr && PlayerSpeed <= 0) {
                    PlayerCharacter->CharacterState = ECharacterState::ROTATE;
                    // turn right
                    if (Sign > 0) {
                        PlayerCharacter->TurnRight = true;
                        // turn left
                    } else if (Sign < 0) {
                        PlayerCharacter->TurnLeft = true;
                    }
                }
            } else if (PlayerCharacter != nullptr) {
                // Update turn variables
                PlayerCharacter->TurnLeft = false;
                PlayerCharacter->TurnRight = false;
                PlayerCharacter->CharacterState = ECharacterState::IDLE;
            }
        }
    };

    // When player moving , use only precise rotation
    if (PlayerSpeed > 100.f) {
        PreciseRotate(10);
        return;
    }

    // Goal is close to player
    // Turn off root motion rotation if goal is close
    AllowedDistance = 50;
    if (DistanceToGoal <= AllowedDistance) {
        if (PlayerCharacter != nullptr) {
            // Update turn variables
            PlayerCharacter->TurnLeft = false;
            PlayerCharacter->TurnRight = false;
        }
        return;
    }

    // Goal is near to player, use root motion
    AllowedDistance = 300;
    if (DistanceToGoal <= AllowedDistance) {
        RootMotionRotation(60);
        return;
    }

    // Goal is far from player, use root motion
    AllowedDistance = 600;
    if (DistanceToGoal >= AllowedDistance) {
        RootMotionRotation(30);
        return;
    } 
    else {
        RootMotionRotation(30);
        return; 
    }
}