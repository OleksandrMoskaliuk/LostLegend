// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RevengerCameraPawn.generated.h"

class URevengerCameraComponent;
class URevengerSpringArmComponent;

UCLASS()
class REVENGER_API ARevengerCameraPawn : public APawn {
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ARevengerCameraPawn();

private:
    /** Top down camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
        meta = (AllowPrivateAccess = "true"))
    class URevengerCameraComponent* TopDownCameraComponent;

    /** Camera boom positioning the camera above the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
        meta = (AllowPrivateAccess = "true"))
    class URevengerSpringArmComponent* CameraBoom;

    /** Max camera speed if CameraZoom equal CameraZoomMax */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Controls", meta = (ClampMin = "100", ClampMax = "5000", UIMin = "100", UIMax = "5000"), meta = (AllowPrivateAccess = "true"))
    float MaxCameraSpeed = 1000.f;

    /** Min camera speed if CameraZoom equal CameraZoomMin */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Controls", meta = (ClampMin = "100", ClampMax = "5000", UIMin = "100", UIMax = "5000"), meta = (AllowPrivateAccess = "true"))
    float MinCameraSpeed = 200.f;

    /** Camera rotation speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Controls", meta = (ClampMin = "100", ClampMax = "1000", UIMin = "100", UIMax = "1000"), meta = (AllowPrivateAccess = "true"))
    float CameraRotationSpeed = 100.f;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Move pawn left
    void MoveCameraForward();
    void MoveCameraBackward();
    void MoveCameraLeft();
    void MoveCameraRight();
    void RotateCameraLeft();
    void RotateCameraRight();

    /** Returns TopDownCameraComponent subobject **/
    FORCEINLINE class URevengerCameraComponent* GetTopDownCameraComponent() const
    {
        return TopDownCameraComponent;
    }
    /** Returns CameraBoom subobject **/
    FORCEINLINE class URevengerSpringArmComponent* GetCameraBoom() const
    {
        return CameraBoom;
    }
};
