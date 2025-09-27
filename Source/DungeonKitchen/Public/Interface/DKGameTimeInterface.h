// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Game/Time/DKTimeManager.h"
#include "DKGameTimeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UDKGameTimeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DUNGEONKITCHEN_API IDKGameTimeInterface
{
	GENERATED_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameTime")
	UDKTimeManager* GetDKTimeManager();
};
