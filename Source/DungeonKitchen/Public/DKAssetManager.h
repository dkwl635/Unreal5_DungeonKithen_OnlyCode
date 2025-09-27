// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "DKAssetManager.generated.h"

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API UDKAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UDKAssetManager& Get();

private:
	virtual void StartInitialLoading() override;
};
