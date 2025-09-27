// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DKInteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDKInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class DUNGEONKITCHEN_API IDKInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION()
	virtual void Interact() = 0;
};
