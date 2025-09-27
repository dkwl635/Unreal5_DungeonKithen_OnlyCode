// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStructures.h"
#include "DKPantryInventoryWidget.generated.h"

class UScrollBox;
class UUniformGridPanel;
class UDKPantrySlotWidget;
class UDKFood;


UCLASS()
class DUNGEONKITCHEN_API UDKPantryInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Columns = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 DefaultRows = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TSubclassOf<UDKPantrySlotWidget> SlotClass;

	// 전체 갱신
	UFUNCTION(BlueprintCallable)
	void RefreshFromPantry(const TArray<UDKFood*>& Foods);

private:
	UPROPERTY()
	TArray<UDKPantrySlotWidget*> Slots;
	UPROPERTY()
	class UPlayerInventoryManager* Inv = nullptr;

	UDKPantrySlotWidget* PreCreate();
	
	UFUNCTION()
    void HandleSlotClicked(int32 SlotIndex);
};
