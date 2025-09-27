// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStructures.h"
#include "DKBlockPaletteWidget.generated.h"


class UUniformGridPanel;
class UDKPantrySlotWidget;
class UPlayerInventoryManager;
class UTexture2D;
class UCookingSystem;
class UDKDish;


UCLASS()
class DUNGEONKITCHEN_API UDKBlockPaletteWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 드래그 시작 훅
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
	virtual void   NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, class UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:


public:
	// UMG 디자이너에서 UniformGridPanel에 접근하기 위한 변수
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* Grid_Palette = nullptr;

	// 슬롯 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDKPantrySlotWidget> SlotClass;

	UPROPERTY()
	UPlayerInventoryManager* Inv = nullptr;

	UPROPERTY()
	UCookingSystem* CookingSystem = nullptr;

	/** 팔레트에 표시할 ShapeId 목록 (보통 Dish 하나만) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Palette")
    TArray<int32> PaletteShapeIDs;

	UPROPERTY()
	UTexture2D* DishIcon;


	UPROPERTY() int32 CachedShapeId = INDEX_NONE;
	UPROPERTY() UTexture2D* CachedDishIcon = nullptr;
	// 데이터 테이블/매니저가 소유하므로 포인터 캐시해도 보통 문제 없음(원한다면 복사)
	const FBlockShapeDef* CachedDef = nullptr;
   
	/** Dish가 생겼을 때 팔레트 재구성 (CookingSystem 델리게이트에서 바인딩) */
	UFUNCTION(BlueprintCallable)
	void SetPaletteByDish(UDKDish* Dish);
	/** 현재 Dish/ShapeId를 기반으로 팔레트 그리드 재빌드 */
	UFUNCTION()
	void RebuildPaletteFromManager();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasBlockInPalette() const;

	UWidget* GetDragVisualWidget(UTexture2D* Icon, const FBlockShapeDef* Def, float CellPx = 90.f);

	// const FBlockShapeDef* CachedDef = nullptr;

	UDKDish* CachedDish;

	// ★ 팔레트 전체 호버 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Palette|Hover")
    bool bPaletteHovered = false;

    // ★ 호버 틴트 색
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Palette|Hover")
    FLinearColor PaletteHoverTint = FLinearColor::Red;

    // ★ 마우스 진입/이탈
    virtual void NativeOnMouseEnter(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // ★ 모든 슬롯에 호버 비주얼 적용/해제
    UFUNCTION(BlueprintCallable, Category="Palette|Hover")
    void ApplyPaletteHover(bool bEnable);

	UPROPERTY(BlueprintReadOnly, Category="Palette")
	bool bIsPreview = false;

	UFUNCTION(BlueprintCallable, Category="Palette")
	void ShowIconPreview(UDKDish* Dish);

	void HandleDishPreviewCreated_Delayed(UDKDish* Dish);

	UPROPERTY() class USizeBox* PreviewBox = nullptr;
	UPROPERTY() class UImage*   PreviewImg = nullptr;

	void EnsurePreviewLayer();   // 프리뷰용 컨테이너(Overlay/SizeBox/Image) 미리 구성
	void PrewarmPreview();       // 더미로 한 번 레이아웃을 통과시킴

	UFUNCTION(BlueprintCallable, Category="Palette")
	void ClearIconPreview();

	void ClearPaletteVisuals();

	// 미리보기용 내부 핸들
	UPROPERTY(Transient)
	class UOverlay* PreviewOverlay = nullptr;

protected:
    // 다른 위젯 속성들...
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block")
    FBlockShapeDef BlockShape;
};
