// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevengerGameModeBase.h"
#include "Public/RevengerCharacter.h"
#include "Public/RevengerPlayerController.h"
#include "UObject/ConstructorHelpers.h"

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