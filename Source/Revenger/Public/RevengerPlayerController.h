// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "RevengerPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class ARevengerGameModeBase;
class ARevengerCameraPawn;
class ARevengerCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class REVENGER_API ARevengerPlayerController : public APlayerController {

    GENERATED_BODY()

public:
    ARevengerPlayerController();

    /** RTS camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes,
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ARevengerCameraPawn> RTS_CameraPawn;

    /** Time Threshold to know if it was a short press */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    float ShortPressThreshold;

    /** FX Class that we will spawn when clicking */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UNiagaraSystem* FXCursor;

    /** MappingContext */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* SetDestinationClickAction;

    /** Control top down camera zoom */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* SetCameraZoomAction;

    /** Move camera using forward vector */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveCameraUpAction;

    /** Move camera using negative forward vector  */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveCameraDownAction;

    /** Move camera using negative right vector  */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveCameraLeftAction;

    /** Move camera using right vector  */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveCameraRightAction;

    /** Roatate camera */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* RotateCameraRightAction;

    /** Roatate camera */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* RotateCameraLeftAction;

    virtual void Tick(float DeltaTime) override;

protected:
    /** True if the controlled character should navigate to the mouse cursor. */
    uint32 bMoveToMouseCursor : 1;

    virtual void SetupInputComponent() override;

    // To add mapping context
    virtual void BeginPlay();

    /** Input handlers for SetDestination action. */
    void OnInputStarted();
    void OnSetDestinationTriggered();
    void OnSetDestinationReleased();

    /** Input handlers for camera action. */
    void OnCameraZoom(const FInputActionValue& Value);
    void OnMoveCameraForward(const FInputActionValue& Value);
    void OnMoveCameraBackward(const FInputActionValue& Value);
    void OnMoveCameraLeft(const FInputActionValue& Value);
    void OnMoveCameraRight(const FInputActionValue& Value);
    void OnRotateCameraLeft(const FInputActionValue& Value);
    void OnRotateCameraRight(const FInputActionValue& Value);

private:
    void UpdateGoal();
    void RotateToGoal(const FVector GoalLocation, float DeltaTime);
    void MoveToGoal();
    void SpawnRTSCamera();

    // Root motion related
    // Move to first goal while distance substract

    FVector CachedDestination;
    FVector NextClosestGoal;

    bool bNewGoal;
    float FollowTime; // For how long it has been pressed
    float StateTransitionTime;
    // Pawns
    ARevengerCharacter* DefaultPawn;
    ARevengerCameraPawn* CameraPawn;
};
