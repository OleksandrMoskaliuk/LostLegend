// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.

#include "RevengerGameModeBase.h"
#include "Public/RevengerCharacter.h"
#include "Public/RevengerPlayerController.h"
#include "UObject/ConstructorHelpers.h"

#include "Components/RevengerCameraPawn.h"
#include "Kismet/GameplayStatics.h"

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
}

