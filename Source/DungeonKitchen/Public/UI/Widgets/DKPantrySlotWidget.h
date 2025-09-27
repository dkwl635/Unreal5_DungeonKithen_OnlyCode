// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStructures.h"
#include "DKPantrySlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPantrySlotClicked, int32, SlotIndex);

class UButton;
class UImage;
class UDKFood;
class UTexture2D;
class UDragDropOperation;
class UDKDragDropOperation;



UCLASS()
class DUNGEONKITCHEN_API UDKPantrySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // 생성자에서 드래그 비주얼 클래스 로드
    UDKPantrySlotWidget(const FObjectInitializer& ObjectInitializer);

    // 슬롯에 음식 아이템을 설정
	UFUNCTION(BlueprintCallable)
	void SetItem(UDKFood* Item);
    // 슬롯을 비움
	UFUNCTION(BlueprintCallable)
	void SetEmpty();

    // 슬롯 인덱스 설정
	/*UFUNCTION(BlueprintCallable)
	void SetupIndex(int32 InIndex) { SlotIndex = InIndex; }*/
    
    // 블록 모양 데이터를 슬롯에 설정
	//void SetBlockShape(const FBlockShapeDef& Shape);
	// void SetBlockCell(const FBlockShapeDef& Def, const FIntPoint& Cell);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDKInventoryType InventoryType = EDKInventoryType::Pantry;


protected:
	virtual void NativeConstruct() override;
	
    // 마우스 버튼 클릭 시 드래그 감지
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
    // 드래그가 감지되면 호출
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation) override;

    // 아이템이 드롭될 때 호출
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;
    
    // 블루프린트에서 드래그 비주얼을 생성하는 함수
	//UFUNCTION(BlueprintImplementableEvent, Category="UI")
	UWidget* GetDragVisualWidget();

protected:
    UPROPERTY(meta=(BindWidget))
    UButton* ClickButton;
    UPROPERTY(meta=(BindWidget))
    UImage* IconImage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slot")
    float IconSize = 85.f;

private:
	UPROPERTY()
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY()
    UDKFood* FoodItem = nullptr;
    
    // 클릭 이벤트 핸들러
	UFUNCTION()
	void HandleClicked();

public:
    // 외부에 노출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnPantrySlotClicked OnPantrySlotClicked;

	// 부모 위젯(팬트리/프렙)에서 슬롯 만들 때 꼭 호출
	UFUNCTION(BlueprintCallable)
	void SetupContext(EDKInventoryType InType, int32 InIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGuid BlockId;

};
