// Copyright © 2025 Tartare Studio

#pragma once
#include "CoreMinimal.h"
#include "DKTime.generated.h" 

USTRUCT(BlueprintType,Blueprintable)
struct FDKTime
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Hour = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Minute = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Season = 0;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Day = 1;



};