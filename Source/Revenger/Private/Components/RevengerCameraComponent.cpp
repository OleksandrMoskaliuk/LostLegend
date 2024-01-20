// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RevengerCameraComponent.h"

void URevengerCameraComponent::MoveCameraForward(float Amount)
{
    float ForwardMovementAmount = Amount * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = this->GetForwardVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, this->GetComponentLocation().Z));
}

void URevengerCameraComponent::MoveCameraBackward(float Amount)
{
    // Adjust the movement amount based on your requirements
    float ForwardMovementAmount = Amount * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = -this->GetForwardVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, this->GetComponentLocation().Z));
}

void URevengerCameraComponent::MoveCameraLeft(float Amount)
{

    // Adjust the movement amount based on your requirements
    float ForwardMovementAmount = Amount * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = -this->GetRightVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, this->GetComponentLocation().Z));
}

void URevengerCameraComponent::MoveCameraRight(float Amount)
{
    // Adjust the movement amount based on your requirements
    float ForwardMovementAmount = Amount * GetWorld()->GetDeltaSeconds();

    // Get the forward vector of the camera
    FVector ForwardVector = this->GetRightVector();

    // Move the camera forward along its current facing direction
    FVector NewLocation = this->GetComponentLocation() + ForwardVector * ForwardMovementAmount;

    // Set the new location with the same Z (height)
    this->SetWorldLocation(FVector(NewLocation.X, NewLocation.Y, this->GetComponentLocation().Z));
}

void URevengerCameraComponent::RotateCameraLeft(float Amount)
{
    // Adjust the rotation amount based on your requirements
    float RotationAmount = Amount * GetWorld()->GetDeltaSeconds(); // Adjust this value as needed

    // Get the rotation of the camera
    FRotator CompRotator = this->GetComponentRotation();

    // Rotate the camera left
    FRotator NewRotation = CompRotator + FRotator(0.0f, -RotationAmount, 0.0f);

    // Set the new rotation with the same Z (height)
    this->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, this->GetComponentRotation().Roll));
}

void URevengerCameraComponent::RotateCameraRight(float Amount)
{
    // Adjust the rotation amount based on your requirements
    float RotationAmount = Amount * GetWorld()->GetDeltaSeconds(); // Adjust this value as needed

    // Get the rotation of the camera
    FRotator CompRotator = this->GetComponentRotation();

    // Rotate the camera right
    FRotator NewRotation = CompRotator + FRotator(0.0f, RotationAmount, 0.0f);

    // Set the new rotation with the same Z (height)
    this->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, this->GetComponentRotation().Roll));
}
