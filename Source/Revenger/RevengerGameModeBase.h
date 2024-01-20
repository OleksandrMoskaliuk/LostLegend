// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RevengerGameModeBase.generated.h"

class ARevengerCameraPawn;

/**
 *
 */
UCLASS()
class REVENGER_API ARevengerGameModeBase : public AGameModeBase {
    GENERATED_BODY()

public:
    ARevengerGameModeBase();

private:
    /** RTS camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes,
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ARevengerCameraPawn> RTS_CameraPawn;

public:

    virtual void StartPlay() override;

    FORCEINLINE class APawn* Get_RTS_CameraPawn() const
    {
        return CameraPawn;
    }

private:
    void SpawnSecondPawn();
    APawn * CameraPawn;
};
