// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DKConsumedStripWidget.generated.h"

class UCanvasPanel;
class UPlayerInventoryManager;
class UDKFood;

UCLASS()
class DUNGEONKITCHEN_API UDKConsumedStripWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* StripCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Strip")
	int32 ItemsPerRow = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Strip")
	float IconSize = 48.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Strip")
	float OverlapX = 12.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Strip")
	float RowSpacing = 3.f;

	UPROPERTY()
	UPlayerInventoryManager* Inv;

	UFUNCTION()
	void OnFoodCame(const TArray<UDKFood*>& Foods);

	UFUNCTION(BlueprintCallable)
	void PushIcon(UTexture2D* Icon);

	UFUNCTION(BlueprintCallable)
	void ClearStrip();

private:
	UPROPERTY()
	TArray<UWidget*> IconWidgets;
};
