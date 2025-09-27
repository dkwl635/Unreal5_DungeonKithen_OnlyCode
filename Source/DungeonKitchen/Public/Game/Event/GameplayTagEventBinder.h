// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTagEventBinder.generated.h"
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagEventDelegate  , const FGameplayTag , EventTag);

UCLASS(Blueprintable, BlueprintType)
class DUNGEONKITCHEN_API UGameplayTagEventBinder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FGameplayTagEventDelegate EventDelegate;


};
