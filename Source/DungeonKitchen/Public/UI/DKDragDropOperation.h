// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/ItemDataStructures.h"
#include "DKDragDropOperation.generated.h"



UCLASS()
class DUNGEONKITCHEN_API UDKDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	EDKInventoryType SourceInventory = EDKInventoryType::Pantry;
    UPROPERTY(BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;
    UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UDKFood> Food = nullptr;
};
