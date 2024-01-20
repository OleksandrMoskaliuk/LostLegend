// Copyright (c) [2024] [Alex Mercer]

// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions, and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions, and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// For inquiries or more information, contact: @Dru9Dealer on Twitter.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "Components/RevengerCameraPawn.h"
#include "Components/RevengerCameraComponent.h"
#include "Components/RevengerSpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values
ARevengerCameraPawn::ARevengerCameraPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create a camera boom...
    CameraBoom = CreateDefaultSubobject<URevengerSpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->SetUsingAbsoluteRotation(
        true); // Don't want arm to rotate when character does
    CameraBoom->TargetArmLength = 1200.f;
    CameraBoom->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraLagSpeed = 5.f;
    CameraBoom->CameraRotationLagSpeed = 5.f;

    // Create a camera...
    TopDownCameraComponent = CreateDefaultSubobject<URevengerCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom,
        USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called when the game starts or when spawned
void ARevengerCameraPawn::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ARevengerCameraPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARevengerCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARevengerCameraPawn::MoveCameraForward()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float Speed = FMath::Lerp(MinCameraSpeed, MaxCameraSpeed, CameraBoom->CameraZoom) * 100.f * DeltaTime;

    float ForwardMovementAmount = Speed * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = this->GetActorForwardVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetActorLocation() + ForwardVector * ForwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, this->GetActorLocation().Z));
}

void ARevengerCameraPawn::MoveCameraBackward()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float Speed = FMath::Lerp(MinCameraSpeed, MaxCameraSpeed, CameraBoom->CameraZoom) * 100.f * DeltaTime;
    float BackwardMovementAmount = Speed * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = -this->GetActorForwardVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetActorLocation() + ForwardVector * BackwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, this->GetActorLocation().Z));
}

void ARevengerCameraPawn::MoveCameraLeft()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float Speed = FMath::Lerp(MinCameraSpeed, MaxCameraSpeed, CameraBoom->CameraZoom) * 100 * DeltaTime;
    float LeftMovementAmount = Speed * GetWorld()->GetDeltaSeconds();

    // Get the right vector of the camera
    FVector RightVector = -this->GetActorRightVector();

    // Move the camera left along its current facing direction
    FVector NewLocation = this->GetActorLocation() + RightVector * LeftMovementAmount;

    // Set the new location with the same Z (height)
    this->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, this->GetActorLocation().Z));
}

void ARevengerCameraPawn::MoveCameraRight()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float Speed = FMath::Lerp(MinCameraSpeed, MaxCameraSpeed, CameraBoom->CameraZoom) * 100.f * DeltaTime;
    float RightMovementAmount = Speed * GetWorld()->GetDeltaSeconds();

    // Get the right vector of the camera
    FVector RightVector = this->GetActorRightVector();

    // Move the camera right along its current facing direction
    FVector NewLocation = this->GetActorLocation() + RightVector * RightMovementAmount;

    // Set the new location with the same Z (height)
    this->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, this->GetActorLocation().Z));
}

void ARevengerCameraPawn::RotateCameraLeft()
{
    float RotationAmount = CameraRotationSpeed * GetWorld()->GetDeltaSeconds();

    // Get the rotation of the camera
    FRotator CompRotator = this->GetActorRotation();

    // Rotate the camera left
    FRotator NewRotation = CompRotator + FRotator(0.0f, -RotationAmount, 0.0f);

    // Set the new rotation with the same Z (height)
    this->SetActorRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, this->GetActorRotation().Roll));
}

void ARevengerCameraPawn::RotateCameraRight()
{
    float RotationAmount = CameraRotationSpeed * GetWorld()->GetDeltaSeconds();

    // Get the rotation of the camera
    FRotator CompRotator = this->GetActorRotation();

    // Rotate the camera right
    FRotator NewRotation = CompRotator + FRotator(0.0f, RotationAmount, 0.0f);

    // Set the new rotation with the same Z (height)
    this->SetActorRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, this->GetActorRotation().Roll));
}