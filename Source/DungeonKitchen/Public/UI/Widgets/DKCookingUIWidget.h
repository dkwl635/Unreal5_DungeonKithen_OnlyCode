// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DKCookingUIWidget.generated.h"

class UButton;

UCLASS()
class DUNGEONKITCHEN_API UDKCookingUIWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_Close;

	UFUNCTION()
	void OnCloseButtonClicked();

	void AutoReturnPrepToPantry();
};
