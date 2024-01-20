// Fill out your copyright notice in the Description page of Project Settings.

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
#include "Components/RevengerSpringArmComponent.h"
#include "Components/RevengerCameraComponent.h"
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
    // Get GameMode 
    RevengerGameMode = Cast<ARevengerGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
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
    if (RevengerGameMode) 
    {

      Possess(RevengerGameMode->Get_RTS_CameraPawn());
        
    }
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
    if (FollowTime <= ShortPressThreshold && ControlledPawn && PlayerCharacter) {
        // WSimpleMoveToLocation is a part of NavigationSystem it will not work untill NavMeshBoundsVolume will be added to map
        // After adding press press 'P' button to check walkable ground
        FHitResult Hit;
        bool bHitSuccessful = false;
        bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

        // If we hit a surface, cache the location
        if (bHitSuccessful) {
            CachedDestination = Hit.Location;
            // Nex goal update only when player Idle
            PlayerCharacter->CharacterState = ECharacterState::IDLE;
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
    
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) 
        {
           /* if (URevengerSpringArmComponent* SpringArm = PlayerCharacter->GetCameraBoom()) 
            {
                SpringArm->PullOrPush(Value.Get<float>());
            }*/
            
        }
    }
}

void ARevengerPlayerController::OnMoveCameraForward(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (UCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {
            //    GEngine->AddOnScreenDebugMessage(4255, 0.5f, FColor::Red, "MoveCameraUp");

            //     // Adjust the movement amount based on your requirements
            //    float ForwardMovementAmount = 2000.0f * GetWorld()->GetDeltaSeconds();

            //    // Get the forward vector of the camera
            //    FVector ForwardVector = CameraComp->GetForwardVector();

            //    // Move the camera forward along its current facing direction
            //    FVector NewLocation = CameraComp->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

            //    // Set the new location with the same Z (height)
            //    CameraComp->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, CameraComp->GetComponentLocation().Z));
            //}
        }
    }
}


void ARevengerPlayerController::OnMoveCameraBackward(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (URevengerCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {

            //    // Adjust the movement amount based on your requirements
            //    float ForwardMovementAmount = 2000.0f * GetWorld()->GetDeltaSeconds();

            //    // Get the forward vector of the camera
            //    FVector ForwardVector = -CameraComp->GetForwardVector();

            //    // Move the camera forward along its current facing direction
            //    FVector NewLocation = CameraComp->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

            //    // Set the new location with the same Z (height)
            //    CameraComp->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, CameraComp->GetComponentLocation().Z));
            //}
        }
    }
}


void ARevengerPlayerController::OnMoveCameraLeft(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (URevengerCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {

            //    // Adjust the movement amount based on your requirements
            //    float ForwardMovementAmount = 1000.0f * GetWorld()->GetDeltaSeconds();

            //    // Get the forward vector of the camera
            //    FVector ForwardVector = -CameraComp->GetRightVector();

            //    // Move the camera forward along its current facing direction
            //    FVector NewLocation = CameraComp->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

            //    // Set the new location with the same Z (height)
            //    CameraComp->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, CameraComp->GetComponentLocation().Z));
            //}
        }
    }
}


void ARevengerPlayerController::OnMoveCameraRight(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (URevengerCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {

            //    // Adjust the movement amount based on your requirements
            //    float ForwardMovementAmount = 1000.0f * GetWorld()->GetDeltaSeconds();

            //    // Get the forward vector of the camera
            //    FVector ForwardVector = CameraComp->GetRightVector();

            //    // Move the camera forward along its current facing direction
            //    FVector NewLocation = CameraComp->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

            //    // Set the new location with the same Z (height)
            //    CameraComp->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, CameraComp->GetComponentLocation().Z));
            //}
        }
    }
}

void ARevengerPlayerController::OnRotateCameraLeft(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (URevengerCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {

            //    // Adjust the rotation amount based on your requirements
            //    float RotationAmount = 100.0f * GetWorld()->GetDeltaSeconds(); // Adjust this value as needed

            //    // Get the rotation of the camera
            //    FRotator CompRotator = CameraComp->GetComponentRotation();

            //    // Rotate the camera left
            //    FRotator NewRotation = CompRotator + FRotator(0.0f, -RotationAmount, 0.0f);

            //    // Set the new rotation with the same Z (height)
            //    CameraComp->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, CameraComp->GetComponentRotation().Roll));
            //}
        }
    }
}

void ARevengerPlayerController::OnRotateCameraRight(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn()) {
        // Update goal for using move and other logick only when player idle
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            //if (URevengerCameraComponent* CameraComp = PlayerCharacter->GetTopDownCameraComponent()) {

            //    // Adjust the rotation amount based on your requirements
            //    float RotationAmount = 100.0f * GetWorld()->GetDeltaSeconds(); // Adjust this value as needed

            //    // Get the rotation of the camera
            //    FRotator CompRotator = CameraComp->GetComponentRotation();

            //    // Rotate the camera right
            //    FRotator NewRotation = CompRotator + FRotator(0.0f, RotationAmount, 0.0f);

            //    // Set the new rotation with the same Z (height)
            //    CameraComp->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, CameraComp->GetComponentRotation().Roll));
            //}
        }
    }
}



void ARevengerPlayerController::UpdateGoal()
{
    if (APawn* ControlledPawn = GetPawn()) 
    {  
        // Update goal for using move and other logick only when player idle
       if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) 
       {
            FVector PlayerLocation = ControlledPawn->GetActorLocation();
            UNavigationPath* Path = UNavigationSystemV1::FindPathToActorSynchronously(this, CachedDestination, ControlledPawn, PlayerCharacter->MinimalDistanceToMove);
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

void ARevengerPlayerController::MoveToGoal()
{
    if (APawn* ControlledPawn = GetPawn()) {
        if (ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn)) {
            float DistanceToClosestGoal = FVector::DistXY(ControlledPawn->GetActorLocation(), NextClosestGoal);
            float DistanceToMainGoal = FVector::DistXY(PlayerCharacter->GetActorLocation(), CachedDestination);

            // Main player stop
            if (DistanceToMainGoal <= PlayerCharacter->MinimalDistanceToMove) {
                    CachedDestination = FVector::ZeroVector;
                    PlayerCharacter->CharacterState = ECharacterState::IDLE;
                    return;
            }

            // Handle acceleration
            if (PlayerCharacter->CharacterState == ECharacterState::SLOW_RUN
                || PlayerCharacter->CharacterState == ECharacterState::MEDIUM_RUN
                || PlayerCharacter->CharacterState == ECharacterState::FAST_RUN
                || PlayerCharacter->CharacterState == ECharacterState::WALK) {
                    StateTransitionTime += GetWorld()->GetDeltaSeconds();
            } else {
                    StateTransitionTime = 0.f;
            }

            // Slow down when goal is close
            if (PlayerCharacter->CharacterState == ECharacterState::SLOW_RUN
                || PlayerCharacter->CharacterState == ECharacterState::MEDIUM_RUN
                || PlayerCharacter->CharacterState == ECharacterState::FAST_RUN
                    && DistanceToClosestGoal < 120.f) {
                    PlayerCharacter->CharacterState = ECharacterState::WALK;
            }

            // Start moving
            if (DistanceToClosestGoal > PlayerCharacter->MinimalDistanceToMove
                    && PlayerCharacter->CharacterState == ECharacterState::IDLE
                || PlayerCharacter->CharacterState == ECharacterState::SLOW_RUN) {
                    PlayerCharacter->CharacterState = ECharacterState::WALK;
            }

            // Slow run
            if (DistanceToClosestGoal > 250.f
                && PlayerCharacter->CharacterState == ECharacterState::WALK
                && StateTransitionTime > PlayerCharacter->MinimalTransitionStateTime) {
                    PlayerCharacter->CharacterState = ECharacterState::SLOW_RUN;
            }
        }
    }
    return;
}

void ARevengerPlayerController::RotateToGoal(const FVector GoalLocation, float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    ARevengerCharacter* PlayerCharacter = Cast<ARevengerCharacter>(ControlledPawn);
    float DistanceToGoal = 0.f;
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
    // Lambda for precise Pawn rotation. Used only when player move
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
                    // turn right
                    if (Sign > 0) {
                        PlayerCharacter->CharacterState = ECharacterState::TURN_RIGHT;
                        // turn left
                    } else if (Sign < 0) {
                        PlayerCharacter->CharacterState = ECharacterState::TURN_LEFT;
                    }
                }
            } else if (PlayerCharacter != nullptr) {
                // Update turn variables
                PlayerCharacter->CharacterState = ECharacterState::IDLE;
            }
        }
    };

    // When player moving , use only precise rotation
    if (PlayerCharacter && PlayerSpeed > 100.f) {
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

    if (PlayerCharacter && DistanceToGoal < PlayerCharacter->MinimalDistanceToMove) 
    {
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