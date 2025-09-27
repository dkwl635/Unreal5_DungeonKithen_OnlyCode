// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/ItemDataStructures.h"
#include "DKBlockDragDropOp.generated.h"

class DKInventoryWidget;

UCLASS()
class DUNGEONKITCHEN_API UDKBlockDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
    TObjectPtr<class UDKDish> Dish = nullptr;
	
	UPROPERTY()
	FBlockShapeDef DragBlockDef;

	UPROPERTY(BlueprintReadWrite)
	int32 DragRotation = 0;
	/** 사용자가 클릭해 잡은 셀(도형 로컬 좌표계) */
	UPROPERTY(BlueprintReadWrite)
	FIntPoint GrabCell = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadWrite)
	EDKInventoryType SourceInventory = EDKInventoryType::Cooking;

	/** 회전 적용된 셀과 그랩 오프셋을 리턴 (인벤토리 드롭 계산에서 사용) */
	UFUNCTION(BlueprintCallable)
	void GetRotatedCellsAndGrab(TArray<FIntPoint>& OutCellsR, FIntPoint& OutGrabR) const;
	/** 회전 보조: 90도 단위로 DragBlockDef.Cells 회전 */
	static void RotateCells(const TArray<FIntPoint>& In, int32 Rot90, TArray<FIntPoint>& Out);

	// 초기화(드래그 시작 시 호출)
	UFUNCTION(BlueprintCallable, Category="BlockDrag")
	void InitFromShape(const FBlockShapeDef& InDef, int32 InRotation = 0);

	// 회전이 적용된 셀 목록을 반환 (X=row, Y=col)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="BlockDrag")
	TArray<FIntPoint> GetRotatedCells() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Data")
	TSoftObjectPtr<UTexture2D> BlockIconTexture;

	UPROPERTY(BlueprintReadWrite)
    FGuid OriginalBlockId;

	UPROPERTY() TWeakObjectPtr<class UDKInventoryWidget> SourceInventoryWidget;



private:
	/** 단일 셀 회전 유틸(도 단위, 0/90/180/270) */
	//static FORCEINLINE FIntPoint RotateCell(const FIntPoint& P, int32 RotDeg);
	
};
