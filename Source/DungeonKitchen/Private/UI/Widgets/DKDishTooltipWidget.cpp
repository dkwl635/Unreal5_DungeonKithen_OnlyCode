// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKDishTooltipWidget.h"
#include "Components/TextBlock.h"


void UDKDishTooltipWidget::SetTooltipFromDishData(const FDKDishData& DishData)
{
	if (!Txt_ItemName || !Txt_ItemDesc || !Txt_EffectDesc) return;

	const FString NameStr = DishData.ItemName.ToString(); // FName → FString
	const FString EffectStr = DishData.DishEffect;        // 이펙트 설명
	const FString DescStr = DishData.Description;         // 일반 설명

	Txt_ItemName->SetText(FText::FromString(NameStr));
	Txt_EffectDesc->SetText(FText::FromString(EffectStr));
	Txt_ItemDesc->SetText(FText::FromString(DescStr));
}
