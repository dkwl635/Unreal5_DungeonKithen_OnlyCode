// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Actor/Weapon/GunBase.h"
#include "PepperGrinder.generated.h"

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API APepperGrinder : public AGunBase
{
	GENERATED_BODY()

public:
	//APepperGrinder();

	virtual void Attack() override;

	UPROPERTY(EditDefaultsOnly, Category="Fire")
	float Spread;

	UPROPERTY(EditDefaultsOnly, Category="Fire")
	float Range;

protected:

};
