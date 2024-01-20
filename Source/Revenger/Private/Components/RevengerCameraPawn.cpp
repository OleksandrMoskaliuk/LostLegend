// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RevengerCameraPawn.h"

// Sets default values
ARevengerCameraPawn::ARevengerCameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

