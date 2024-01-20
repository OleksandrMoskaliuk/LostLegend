// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevengerGameModeBase.h"
#include "Public/RevengerCharacter.h"
#include "Public/RevengerPlayerController.h"
#include "UObject/ConstructorHelpers.h"

#include "Kismet/GameplayStatics.h"
#include "Components/RevengerCameraPawn.h"

ARevengerGameModeBase::ARevengerGameModeBase()
{
    // use our custom PlayerController class
    PlayerControllerClass = ARevengerPlayerController::StaticClass();

    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/BP_RevengerCharacter.BP_RevengerCharacter"));
    if (PlayerPawnBPClass.Class != nullptr) {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // set default controller to our Blueprinted controller
    static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/PlayerController/BP_RevengerPlayerController.BP_RevengerPlayerController"));
    if (PlayerControllerBPClass.Class != NULL) {
        PlayerControllerClass = PlayerControllerBPClass.Class;
    }

}

void ARevengerGameModeBase::StartPlay()
{
    Super::StartPlay();
    SpawnSecondPawn();
}

void ARevengerGameModeBase::SpawnSecondPawn()
{
    if (UWorld* World = GetWorld()) {
        // Get the player controller
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);

        if (PlayerController) {
            // Get the location and rotation of the player controller
            FVector SpawnLocation;
            FRotator SpawnRotation;
            PlayerController->GetPlayerViewPoint(SpawnLocation, SpawnRotation);

            // Set the spawn parameters
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = PlayerController;
            SpawnParams.Instigator = NULL;

            // Spawn the second pawn at the calculated location
            CameraPawn = World->SpawnActor<ARevengerCameraPawn>(RTS_CameraPawn, SpawnLocation, SpawnRotation, SpawnParams);
        }
    }
}