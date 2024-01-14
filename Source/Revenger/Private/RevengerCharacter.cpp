// Fill out your copyright notice in the Description page of Project Settings.


#include "RevengerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"

// Sets default values
ARevengerCharacter::ARevengerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	// Don't rotate character to camera direction
        bUseControllerRotationPitch = false;
        bUseControllerRotationYaw = false;
        bUseControllerRotationRoll = false;

        // Configure character movement
        GetCharacterMovement()->bOrientRotationToMovement =
            true;  // Rotate character to moving direction
        GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
        GetCharacterMovement()->bConstrainToPlane = true;
        GetCharacterMovement()->bSnapToPlaneAtStart = true;

        // Create a camera boom...
        CameraBoom =
            CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
        CameraBoom->SetupAttachment(RootComponent);
        CameraBoom->SetUsingAbsoluteRotation(
            true);  // Don't want arm to rotate when character does
        CameraBoom->TargetArmLength = 800.f;
        CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
        CameraBoom->bDoCollisionTest =
            false;  // Don't want to pull camera in when it collides with level

        // Create a camera...
        TopDownCameraComponent =
            CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
        TopDownCameraComponent->SetupAttachment(
            CameraBoom, USpringArmComponent::SocketName);
        TopDownCameraComponent->bUsePawnControlRotation =
            false;  // Camera does not rotate relative to arm

        // Activate ticking in order to update the cursor every frame.
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.bStartWithTickEnabled = true;

}

// Called when the game starts or when spawned
void ARevengerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARevengerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARevengerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

