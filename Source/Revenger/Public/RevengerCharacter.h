// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "CoreMinimal.h"
#include "Revenger\Revenger.h"
#include "GameFramework/Character.h"
#include "RevengerCharacter.generated.h"

class URevengerSpringArmComponent;
class URevengerCameraComponent;

UCLASS()
class REVENGER_API ARevengerCharacter : public ACharacter {
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ARevengerCharacter();

public:
    /** Current player state */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
    ECharacterState CharacterState;

    /** Minimal Distance to move */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
    float MinimalDistanceToMove;

    /** Minimal time between movements transition state */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
    float MinimalTransitionStateTime;

    /** Player rotation speed while moving */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
    float MovingRotationSpeed;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent) override;

};
