// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.

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
