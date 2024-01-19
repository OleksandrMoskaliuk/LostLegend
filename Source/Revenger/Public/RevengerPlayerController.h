// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "RevengerPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class REVENGER_API ARevengerPlayerController : public APlayerController {
    GENERATED_BODY()

public:
    ARevengerPlayerController();

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
    void CameraZoom(const FInputActionValue& Value);

private:

    void UpdateGoal();
    void RotateToGoal(const FVector GoalLocation, float DeltaTime);
    void MoveToGoal(); 

    // Root motion related
    // Move to first goal while distance substract

    FVector CachedDestination;
    FVector NextClosestGoal;

    bool bNewGoal;
    float FollowTime; // For how long it has been pressed
    float StateTransitionTime;
};
