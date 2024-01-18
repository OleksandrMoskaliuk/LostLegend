// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTopDownCppImport, Log, All);

UENUM(BlueprintType)
enum class ECharacterState : uint8 {
    IDLE UMETA(DisplayName = "Idle"),
    // Moving
    CROUCHING UMETA(DisplayName = "Crouching"),
    WALK UMETA(DisplayName = "Walk"),
    SLOW_RUN UMETA(DisplayName = "Slow_Run"),
    MEDIUM_RUN UMETA(DisplayName = "Medium_Run"),
    FAST_RUN UMETA(DisplayName = "Fast_Run"),
};