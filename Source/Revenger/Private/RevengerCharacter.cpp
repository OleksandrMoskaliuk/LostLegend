// Fill out your copyright notice in the Description page of Project Settings.

#include "RevengerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

// get game mode
// #include "Kismet/GameplayStatics.h"
// #include "Revenger/RevengerGameModeBase.h"
// // get game mode
// ARevengerGameModeBase *GameMode = Cast<ARevengerGameModeBase>(
//    UGameplayStatics::GetGameMode(GetWorld()->GetAuthGameMode()));

// // get player
// UGameplayStatics::GetPlayerCharacter(GetWorld(),0);

// Sets default values
ARevengerCharacter::ARevengerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to
    // improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Don't rotate character to camera direction
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    // Movement
    MovingRotationSpeed = 50;
    CharacterState = ECharacterState::IDLE;
    MinimalDistanceToMove = 10.f;
    MinimalTransitionStateTime = 1.2f;
}

// Called when the game starts or when spawned
void ARevengerCharacter::BeginPlay() { Super::BeginPlay(); }

// Called every frame
void ARevengerCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

// Called to bind functionality to input
void ARevengerCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
