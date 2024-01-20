// Fill out your copyright notice in the Description page of Project Settings.

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
