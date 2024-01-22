// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "RevengerSpringArmComponent.generated.h"
/**
 *
 */
UCLASS()
class REVENGER_API URevengerSpringArmComponent : public USpringArmComponent {
    GENERATED_BODY()
public:
    URevengerSpringArmComponent();

    UFUNCTION()
    void ZoomCamera(float Value);

    void BeginPlay() override;

    UFUNCTION()
    void InterpolateTargetArmLengthHandler();

    void TickComponent(float DeltaTime, enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Push or pull spring arm step. Increse camera zoom speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "0.01", ClampMax = "1", UIMin = "0.01", UIMax = "1"))
    float ZoomCameraStep = 0.1;

    /** Push or pull spring arm Interpolation speed */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "5"))
    float ZoomInterpolationSpeed = 1;

    /** Max camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "1000", ClampMax = "5000", UIMin = "1000", UIMax = "5000"))
    float MaxZoomDistance = 2000;

    /** Min camera distance to character */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm Control", meta = (ClampMin = "100", ClampMax = "3000", UIMin = "100", UIMax = "3000"))
    float MinZoomDistance = 200;

    /** Camera distance to pawn from 0 to 1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Control", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float CameraZoom = 0;

private:
    float PreviousDistance = 0;
    // from 0.0 to 1;
    FTimerDelegate InterpolateTargetArmLengthDelegate;
    FTimerHandle ZoomTimerHandler;
    FTimerManager* TimerManager;
};
