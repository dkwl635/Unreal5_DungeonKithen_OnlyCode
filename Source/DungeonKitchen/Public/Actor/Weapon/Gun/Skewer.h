// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Actor/Weapon/GunBase.h"
#include "Skewer.generated.h"

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API ASkewer : public AGunBase
{
	GENERATED_BODY()
protected:
	virtual void Attack() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "fire")
	float Angle = 15;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "fire")
	bool bATK1 = true;
	FTimerHandle SkewerTimerHandle;
	float RoundCount = 0;
};
