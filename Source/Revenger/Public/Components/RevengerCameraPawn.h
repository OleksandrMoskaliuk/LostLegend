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

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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
