// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DKPrepTrayWidget.generated.h"

class UDKFood;
class UDKPantrySlotWidget;
class UPlayerInventoryManager;
class UButton;
class UDKBlockPaletteWidget;
class UCookingSystem;
class UTexture2D;
class UHorizontalBox;

/**
 * 준비 트레이 위젯을 관리하는 클래스.
 * 음식을 드래그 앤 드롭으로 받고, '요리하기' 버튼을 통해 블록 팔레트를 갱신하는 역할을 합니다.
 */
UCLASS()
class DUNGEONKITCHEN_API UDKPrepTrayWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* HBox_Slots;

	UPROPERTY(EditAnywhere, Category="PrepTray")
	TSubclassOf<UDKPantrySlotWidget> TraySlotClass;

	TArray<TObjectPtr<UDKPantrySlotWidget>> TraySlots;

	UPROPERTY(meta=(BindWidget))
	UButton* Button_Cooking;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// 드래그 앤 드롭 작업이 위젯 위에 드롭될 때 호출
	virtual bool NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;

private:
	UPROPERTY()
	UPlayerInventoryManager* Inv = nullptr;

	UPROPERTY()
	UCookingSystem* CookingSystem = nullptr;


public:
	// 요리 버튼 클릭 시 이 포인터를 통해 팔레트 위젯의 함수 호출
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDKBlockPaletteWidget* BlockPaletteWidget;

	// 요리하기 버튼 클릭 이벤트에 바인딩
	// 버튼 클릭 시 블록 팔레트에 갱신
	UFUNCTION()
	void OnCookingButtonClicked();

	// 쿠킹시스템 슬롯 데이터에 따라 UI 반영
	UFUNCTION()
	void RefreshFromCookingSystem();

	void BuildTraySlots(int32 NumSlots);     // 생성/부착

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumSlots = 3;
};
