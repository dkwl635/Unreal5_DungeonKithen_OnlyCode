// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStructures.h"
#include "DKDishTooltipWidget.generated.h"

//struct UDKDish;
//class FDKDishData;

UCLASS()
class DUNGEONKITCHEN_API UDKDishTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetTooltipFromDishData(const FDKDishData& DishData);

    //void SetEffectTooltip(const FDKDishEffectData& EffectData);


protected:
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Txt_ItemName;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Txt_EffectDesc;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Txt_ItemDesc;


	UPROPERTY(BlueprintReadOnly)
	int32 DishId;
};
