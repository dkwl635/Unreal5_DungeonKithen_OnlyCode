// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStructures.h"
#include "DKInventoryWidget.generated.h"

class UUniformGridPanel;
class UDKPantrySlotWidget;
class UOverlay;
class UImage;
class UDKDishTooltipWidget;

/**
 * 인벤토리 위젯을 관리하는 클래스.
 * 블록을 그리드에 배치하고, 회전하고, 드래그 앤 드롭을 처리하는 로직 포함
 */

UCLASS()
class DUNGEONKITCHEN_API UDKInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 위젯 생성 시 호출
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	// 드래그 프리뷰 & 드롭
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;
	virtual bool NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp) override;
	virtual void   NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 인벤토리 매니저 서브시스템
	UPROPERTY()
	class UPlayerInventoryManager* Inv = nullptr;

	// 마지막으로 칠했던 프리뷰 슬롯 인덱스들 (지울 때 사용)
	TArray<int32> PreviewIndices;

	// 프리뷰 색상
	//FLinearColor GridIdleColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
	FLinearColor GridIdleColor = FLinearColor::Gray;
	FLinearColor PaletteBlockTint = FLinearColor(0.75f, 0.75f, 0.78f, 1.f); // 아주 미세하게 푸른 톤
    FLinearColor PreviewOK   = FLinearColor(0.24f, 0.92f, 0.35f, 0.90f);
    FLinearColor PreviewBAD  = FLinearColor(1.00f, 0.25f, 0.25f, 0.90f);
    //FLinearColor ClearColor  = FLinearColor(0.f, 0.f, 0.f, 0.f);

public:
	// 변수
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* Grid_Camouflage = nullptr;

	// 블록 전체 이미지를 얹을 오버레이(코드에서 구성)
	UPROPERTY()
	UOverlay* Overlay_Blocks = nullptr;
	UPROPERTY()
	UOverlay* Overlay_Drag = nullptr;      // 새로 추가
    UPROPERTY()
	UOverlay* Overlay_Preview = nullptr;   // 새로 추가

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TSubclassOf<UDKPantrySlotWidget> SlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Rows = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Cols = 0;

	UPROPERTY()
	TArray<UWidget*> CellWidgets;  // Rows*Cols 크기, [r*Cols+c]로 접근

	UWidget* GetCell(int32 R, int32 C) const;
	

	// 그리드 셀의 점유 상태를 나타내는 배열
	// true->점유 false->비어있음
	TArray<bool> CamOcc;

	// 그리드에 배치된 블록 인스턴스 목록
	// 블록 모양, 위치, 회전 정보 저장
	UPROPERTY()
	TArray<FPlacedBlock> CamPlaced;

	UPROPERTY()
	UTexture2D* Icon;

	UPROPERTY()
	UImage* DragGhostImg = nullptr;

	FVector2D DragGhostSize;
	//int32 DragGhostMinR = 0, DragGhostMinC = 0; // 회전된 셀 bounding min
	TArray<FIntPoint> DragGhostCellsR; // 회전된 셀 캐시

	// 드래그 고스트(팔레트 스타일) 메타
	/*UPROPERTY() UWidget*
	DragGhost = nullptr;*/
	// 고스트의 바운딩 정보
	int32 DragGhostMinR = 0, DragGhostMinC = 0;
	int32 DragGhostRows = 1, DragGhostCols = 1;

	// 프리뷰 타일(재사용 가능하도록 보관)
    UPROPERTY()
	TArray<TWeakObjectPtr<UImage>> PreviewTiles;
	// 드래그 고스트 루트(팔레트 스타일 비주얼)
    UPROPERTY()
	TWeakObjectPtr<UWidget> DragGhost;

	// 마지막 마우스 위치(회전 직후 재배치용)
	FVector2D LastDragScreenPos = FVector2D::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category="Inventory|Tooltip")
    TSubclassOf<UDKDishTooltipWidget> DishTooltipClass;



	

	// 함수

	// 초기화/유틸
	// 인벤토리 그리드 초기화
	void InitCamouflageGrid();
	// 그리드의 모든 슬롯 투명하게
	void PaintClearCamouflage();
	// 그리드 크기 업데이트 및 슬롯 재구성
	void UpdateGridDimensions(int32 inRows, int32 inCols);

	// 블록 회전/배치/검사
	// 블록 회전 후 정규화된 셀 좌표 반환
	TArray<FIntPoint> GetRotatedCells(const FBlockShapeDef& Def, int32 Rot) const;
	// 특정 위치에 블록 배치할 수 있는지 판단
	bool CanPlaceCamouflage(int32 r0, int32 c0, const TArray<FIntPoint>& Cells) const;
	// 그리드 스캔하고 블록 배치할 수 있는 첫 번째 빈 공간 찾아서 배치
	FLinearColor GetBaseColorAtCell(int32 R, int32 C) const;
	// 프리뷰 그리기/지우기 유틸
	void ClearPreview();
	void PaintPreview(int32 r0, int32 c0, const TArray<FIntPoint>& Cells, bool bPlaceable);
	// 배치에 성공하면 true 반환, Out 변수에 배치된 블록 정보 채움
	bool PlaceCamouflage_FirstFit(const FBlockShapeDef& Def, int32 Rot, FPlacedBlock& Out);
	// 주어진 위치와 셀 정보 기반으로 블록 배치하고, CamOcc오ㅏ CamPlaced 배열 업데이트
	void PlaceCamouflage_At(int32 r0, int32 c0, const TArray<FIntPoint>& Cells, int32 ShapeId, int32 Rot, FPlacedBlock& Out);

	// 좌표 변환(마우스→그리드 셀; 드롭 시 근처 셀부터 우선 시도)
	// 스크린 좌표를 인벤토리 그리드의 셀 좌표로 변환
	bool ScreenToCamouflageCell(const FGeometry& InGeo, const FVector2D& ScreenPos, int32& OutRow, int32& OutCol) const;
	// FPlacedBlock 정보를 기반으로 그리드에 블록 시각화
	void PaintBlockCamouflage(const FPlacedBlock& B);

	// 블록 전체 아이콘을 Overlay_Blocks에 배치/업데이트
	void PaintBlockWholeImage(const FPlacedBlock& B, const TArray<FIntPoint>& Cells);

	// 요리아이템 저장 복원 유틸
	// CamPlaced 배열에 저장된 블록들 화면에 그리기
	void RebuildFromPlaced();

	// 점유 재계산
	void RecomputeCamOccFromPlaced();


	FPlacedBlock* DraggedBlockPtr;
	bool bDraggingBlock = false;

	FIntPoint OriginalGrabOffset;

	void MoveBlock(const FGuid& BlockId, int32 NewRow, int32 NuwCol, int32 NewRot);
	bool CanMoveBlock(const FGuid& BlockId, int32 r0, int32 c0, const TArray<FIntPoint>& Cells) const;
	FPlacedBlock* FindBlockAtLocation(int32 InRow, int32 InCol);

	// 드래그 고스트
	void CreateDragGhost(UTexture2D* Icon, const TArray<FIntPoint>& CellsR);
	void UpdateDragGhostPos(int32 r0, int32 c0);
	void DestroyDragGhost();

	// 드래그 중 블록 숨기기
	UPROPERTY() TSet<FGuid> HiddenBlocksVisual;

	// 블록별 오버레이 이미지(UImage) 포인터를 캐시
	UPROPERTY() TMap<FGuid, TWeakObjectPtr<UImage>> BlockOverlayMap;

	void SetBlockHidden(const FGuid& BlockId, bool bHidden);

	// 인벤 전용 드래그 비주얼 생성/업데이트
	UWidget* CreateInventoryDragVisual(const struct FBlockShapeDef* Def, int32 Rotation, class UTexture2D* Icon, float CellPx);
	void UpdateDragGhostPos_CenterPivot(int32 r0, int32 c0);

	// 커서 자유 이동용
	void UpdateDragGhostPos_FreeCursor(const FDragDropEvent& E);

	// 회전
	void RotateActiveDrag(int32 DeltaSteps);
	
	// 프리뷰 자기 자신 무시
	//void BeginBlockDrag(FPlacedBlock& B);

	void AttachDishTooltipToWidget(UWidget* Target, int32 DishId);

private:
	FVector2D GridSize;
};
