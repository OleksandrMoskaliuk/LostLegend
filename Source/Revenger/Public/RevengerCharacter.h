// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RevengerCharacter.generated.h"

UCLASS()
class REVENGER_API ARevengerCharacter : public ACharacter {
  GENERATED_BODY()

 public:
  // Sets default values for this character's properties
  ARevengerCharacter();

 private:
  /** Top down camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  class UCameraComponent* TopDownCameraComponent;

  /** Camera boom positioning the camera above the character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  class USpringArmComponent* CameraBoom;

 protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

 public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  // Called to bind functionality to input
  virtual void SetupPlayerInputComponent(
      class UInputComponent* PlayerInputComponent) override;

  /** Returns TopDownCameraComponent subobject **/
  FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const {
    return TopDownCameraComponent;
  }
  /** Returns CameraBoom subobject **/
  FORCEINLINE class USpringArmComponent* GetCameraBoom() const {
    return CameraBoom;
  }
};
