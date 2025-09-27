// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKInventoryWidget.h"
#include "Game/DKPlayerController.h"
#include "UI/Widgets/DKPantrySlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridSlot.h"
#include "UI/DKBlockDragDropOp.h"
#include "Data/ItemDataStructures.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Game/Subsystem/CookingSystem.h"
#include "Game/Subsystem/DigestiveSystem.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Game/Subsystem/DataManager.h"
#include "Data/ItemDataStructures.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "UI/Widgets/DKDishTooltipWidget.h"
#include "Blueprint/WidgetNavigation.h"
#include "Framework/Application/NavigationConfig.h"

static UWidget* FindFirstInTree(UWidget* Root, UClass* WantClass)
{
	if (!Root) return nullptr;
	if (Root->IsA(WantClass)) return Root;

	if (UPanelWidget* Panel = Cast<UPanelWidget>(Root))
	{
		const int32 N = Panel->GetChildrenCount();
		for (int32 i = 0; i < N; ++i)
		{
			if (UWidget* Found = FindFirstInTree(Panel->GetChildAt(i), WantClass))
				return Found;
		}
	}
	return nullptr;
}

static bool ApplyColorToSlotWidget(UWidget* SlotWidget, const FLinearColor& Color)
{
	if (!SlotWidget) return false;

	UImage* Img = Cast<UImage >(FindFirstInTree(SlotWidget, UImage::StaticClass()));
	UBorder* Border = Cast<UBorder>(FindFirstInTree(SlotWidget, UBorder::StaticClass()));

	// ★ 알파 0 → 진짜로 "지우기" (브러시 제거)
	if (Color.A <= 0.001f)
	{
		if (Img)
		{
			FSlateBrush Br;                        // 아무것도 그리지 않음
			Br.DrawAs = ESlateBrushDrawType::NoDrawType;
			Br.TintColor = FSlateColor(FLinearColor::Transparent);
			Img->SetBrush(Br);
			return true;
		}
		if (Border)
		{
			FSlateBrush Br;
			Br.DrawAs = ESlateBrushDrawType::NoDrawType;
			Br.TintColor = FSlateColor(FLinearColor::Transparent);
			Border->SetBrush(Br);
			return true;
		}
		if (UUserWidget* UW = Cast<UUserWidget>(SlotWidget)) { UW->SetColorAndOpacity(FLinearColor::Transparent); return true; }
		SlotWidget->SetRenderOpacity(0.f);
		return true;
	}

	// ★ 색을 칠해야 하는 경우 → 텍스처 곱색 방지: Box + Tint 로 새 브러시 세팅
	if (Img)
	{
		FSlateBrush Br;
		Br.DrawAs = ESlateBrushDrawType::Box;
		Br.TintColor = FSlateColor(Color);
		Img->SetBrush(Br);
		return true;
	}
	if (Border)
	{
		FSlateBrush Br;
		Br.DrawAs = ESlateBrushDrawType::Box;
		Br.TintColor = FSlateColor(Color);
		Border->SetBrush(Br);
		return true;
	}

	if (UUserWidget* UW = Cast<UUserWidget>(SlotWidget)) { UW->SetColorAndOpacity(Color); return true; }
	SlotWidget->SetRenderOpacity(Color.A);
	return false;
}

// 텍스처를 슬롯의 첫 번째 Image에 적용 (없으면 실패 반환)
static bool ApplyTextureToSlotWidget(UWidget* SlotWidget, UTexture2D* Texture, const FLinearColor& Tint)
{
    if (!SlotWidget || !Texture) return false;

    UImage* Img = Cast<UImage >(FindFirstInTree(SlotWidget, UImage::StaticClass()));
    if (Img)
    {
        Img->SetBrushFromTexture(Texture, /*bMatchSize*/ false);
        Img->SetColorAndOpacity(Tint);
        return true;
    }
    return false;
}

static FLinearColor EffectiveBlockColor(const FLinearColor& In)
{
	// 전혀 색 정보가 없으면 보기 쉬운 틴트 사용
	if (In.A <= 0.01f && In.R <= 0.01f && In.G <= 0.01f && In.B <= 0.01f)
	{
		return FLinearColor(0.20f, 0.85f, 1.0f, 0.60f); // 시안색 반투명
	}

	// RGB는 있는데 알파만 0/매우 낮음 → 적당한 알파로 보이게
	FLinearColor Out = In;
	if (Out.A <= 0.01f) Out.A = 0.60f;
	return Out;
}

// ★★★ 행/열로 그리드 자식 찾기
static UWidget* GetGridChildByCell(UUniformGridPanel* Grid, int32 Row, int32 Col)
{
	if (!Grid) return nullptr;
	const int32 N = Grid->GetChildrenCount();
	for (int32 i = 0; i < N; ++i)
	{
		UWidget* Child = Grid->GetChildAt(i);
		if (!Child) continue;
		if (UUniformGridSlot* S = Cast<UUniformGridSlot>(Child->Slot))
		{
			if (S->Row == Row && S->Column == Col)
				return Child;
		}
	}
	return nullptr;
}

// ★★★ (옵션) r,c를 int로 패킹해서 PreviewIndices에 넣고 빼기
static FORCEINLINE int32 PackRC(int32 R, int32 C) { return (R << 16) | (C & 0xFFFF); }
static FORCEINLINE void  UnpackRC(int32 Packed, int32& OutR, int32& OutC)
{
	OutR = (Packed >> 16);
	OutC = (Packed & 0xFFFF);
}

void UDKInventoryWidget::NativeConstruct()
{
	// 슬롯크기 기본값
	Rows = 5;
	Cols = 5;

	if (UGameInstance* GI = GetGameInstance())
	{
		Inv = GI->GetSubsystem<UPlayerInventoryManager>();
	}

	
	// 탭 네비게이션 끄기
	//FNavigationConfig& NavigationConfig = *FSlateApplication::Get().GetNavigationConfig();

	//NavigationConfig.bTabNavigation = false;

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);

	// 위장 그리드 초기화
	InitCamouflageGrid();
	// 그리드 패널 크기 설정
	UpdateGridDimensions(Rows, Cols);

	// ApplyColorToSlotWidget(GetCell(0, 0), FLinearColor(1, 1, 0, 1));

	if (UGameInstance* GI = GetGameInstance())
	{
		auto* Digest = GI->GetSubsystem<UDigestiveSystem>();
		if (Digest)
		{
			if (Digest->bWipeOnNextLoad)
			{
				// 재시작
				CamPlaced.Empty();
				Digest->SavedDishBlocks.Empty();
				Digest->bWipeOnNextLoad = false;
			}
			else
			{
				CamPlaced = Digest->SavedDishBlocks;
				// 아이콘 누락 보정 로직 제거: 반드시 IconTexture가 저장되어 있어야 함
				UE_LOG(LogTemp, Log, TEXT("인벤토리 UI 생성. DigestSystem에서 %d개 블록을 복사했습니다."), CamPlaced.Num());
			}
		}
		
	}

	if (UGameInstance* GI = GetGameInstance()) {
		if (auto* DM = GI->GetSubsystem<UDataManager>()) {
			for (FPlacedBlock& B : CamPlaced) {
				if (!B.IconTexture.IsValid() && B.DishId >= 0) {
					const FDKDishData DishData = DM->GetDishData(B.DishId);
					if (DishData.ItemIcon) {
						// 런타임에라도 보존되도록 포인터를 넣어줌
						B.IconTexture = DishData.ItemIcon;
					}
				}
			}
		}
	}

	CamOcc.Init(false, Rows * Cols);
	for (const FPlacedBlock& Block : CamPlaced)
	{
		if (Inv)
		{
			const FBlockShapeDef* Def = Inv->GetBlock(Block.ShapeId);
			if(!Def) continue;

			TArray<FIntPoint> RotatedCells = GetRotatedCells(*Def, Block.Rotation);
			for (const FIntPoint& d : RotatedCells)
			{
				const int32 R = Block.Row + d.X;
				const int32 C = Block.Col + d.Y;
				const int32 Idx = R * Cols + C;
				if (CamOcc.IsValidIndex(Idx))
				{
					CamOcc[Idx] = true;
				}
			}
		}

	}
    // CamPlaced 배열에 저장된 블록 화면에 그리기 (셀/오버레이 동시 재구성)
    RebuildFromPlaced();

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float)
			{
				if (Overlay_Blocks) Overlay_Blocks->ClearChildren();
				RebuildFromPlaced(); 
				UE_LOG(LogTemp, Log, TEXT("다시 그리기."));
				return false; // 1회만
			}),
		0.f
	);

}

void UDKInventoryWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		auto* Digest = GI->GetSubsystem<UDigestiveSystem>();
		if (Digest)
		{
			if (!Digest->bWipeOnNextLoad) // 재시작 중이면 저장하지 않음
			{
				Digest->SavedDishBlocks = CamPlaced;
				UE_LOG(LogTemp, Log, TEXT("인벤토리 UI 종료. DigestSystem에 %d개 블록을 저장했습니다."), Digest->SavedDishBlocks.Num());
			}
		}
		
	}
	Super::NativeDestruct();
}


FReply UDKInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("Tab Click"));
		// 컨트롤러 캐스팅 후, 메뉴 닫기 함수 호출
		ADKPlayerController* PlayerController = Cast<ADKPlayerController>(GetOwningPlayer());
		if (PlayerController)
		{
			PlayerController->ToggleInventory();
			 // 입력이 처리되었음을 알림
		}
		return FReply::Handled();
	}
	return NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDKInventoryWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Tab)
	{
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("Tab pressed"));

		if (ADKPlayerController* PC = Cast<ADKPlayerController>(GetOwningPlayer()))
		{
			PC->ToggleInventory();
		}
		return FReply::Handled();  // 네비게이션으로 안 넘어가게 여기서 종료
	}

	if (Key == EKeys::Escape)
	{
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("Escape pressed"));

		if (ADKPlayerController* PC = Cast<ADKPlayerController>(GetOwningPlayer()))
		{
			PC->ToggleInventory();
		}
		return FReply::Handled();  // 네비게이션으로 안 넘어가게 여기서 종료
	}

	//if (Key == EKeys::Escape)
	//{
	//	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("ESC pressed"));

	//	if (ADKPlayerController* PC = Cast<ADKPlayerController>(GetOwningPlayer()))
	//	{
	//		PC->ToggleInventory();
	//		return FReply::Handled();  // 네비게이션으로 안 넘어가게 여기서 종료
	//	}
	//	return FReply::Handled();
	//}

	if (InKeyEvent.GetKey() == EKeys::E)
	{
		if (UDragDropOperation* Op = UWidgetBlueprintLibrary::GetDragDroppingContent())
		{
			if (UDKBlockDragDropOp* BOp = Cast<UDKBlockDragDropOp>(Op))
			{
				// 회전 스텝 갱신
				BOp->DragRotation = (BOp->DragRotation + 1) & 3;

				// 팔레트에서 만든 기본 드래그 비주얼도 같이 돌려주기(간단 폴백)
				if (UWidget* V = BOp->DefaultDragVisual)
				{
					V->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
					FWidgetTransform Xf;
					Xf.Angle = BOp->DragRotation * 90.f;
					V->SetRenderTransform(Xf);
				}


				// 인벤 위젯에 “프리뷰/고스트 갱신” 위임
				RotateActiveDrag(0);
			}
		}
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UDKInventoryWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		UE_LOG(LogTemp, Warning, TEXT("마우스 클릭 이벤트 감지 (Preview)"));
		int32 rClicked, cClicked;
		if (ScreenToCamouflageCell(InGeometry, InMouseEvent.GetScreenSpacePosition(), rClicked, cClicked))
		{
			// 클릭한 위치에 블록 있는지 찾기
			// FindBlockAtLocation 함수가 올바르게 구현되어 있어야 합니다.
			DraggedBlockPtr = FindBlockAtLocation(rClicked, cClicked);

			if (DraggedBlockPtr)
			{
				OriginalGrabOffset = FIntPoint(rClicked - DraggedBlockPtr->Row, cClicked - DraggedBlockPtr->Col);

				UE_LOG(LogTemp, Warning, TEXT("Block Drag Start (Preview)"));

				return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
			}
		}
	}
	return FReply::Unhandled();
}

FReply UDKInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	
	return FReply::Unhandled();
}


void UDKInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation)
{
	
	if (!DraggedBlockPtr)
	{
		OutOperation = nullptr;
		return;
	}

	UDKBlockDragDropOp* BlockDragOp = NewObject<UDKBlockDragDropOp>(this);
	if (!BlockDragOp)
	{
		OutOperation = nullptr;
		return;
	}

	// 드래그 시작 시점의 블록 정보를 DraggedBlockPtr에서 가져와서 채웁니다.
	const FBlockShapeDef* Def = Inv->GetBlock(DraggedBlockPtr->ShapeId);
	if (Def)
	{
		BlockDragOp->InitFromShape(*Def, DraggedBlockPtr->Rotation);
	}

	BlockDragOp->OriginalBlockId = DraggedBlockPtr->Id;
	BlockDragOp->SourceInventory = EDKInventoryType::Dish; // 내부 이동임을 명시
	BlockDragOp->GrabCell = OriginalGrabOffset; // 오프셋 저장
	BlockDragOp->DragRotation = DraggedBlockPtr->Rotation;

	Icon = nullptr;
	if(auto* DM = GetGameInstance()->GetSubsystem<UDataManager>())
		if(DraggedBlockPtr->DishId >= 0)
			Icon = DM->GetDishData(DraggedBlockPtr->DishId).ItemIcon;

	if (Icon)
	{
		BlockDragOp->BlockIconTexture = Icon;  
	}

	// 프리뷰에 그려주는 드래그 비주얼
	//if (DraggedBlockPtr) {
	//	SetBlockHidden(DraggedBlockPtr->Id, true);
	//}

	//TArray<FIntPoint> CellsR = GetRotatedCells(*Def, DraggedBlockPtr->Rotation);
	//// 드래그 비주얼 위젯 생성 로직
	//CreateDragGhost(Icon, CellsR);

	// 셀 픽셀 크기
	FVector2D grid = Grid_Camouflage->GetCachedGeometry().GetLocalSize();
	if (grid.X < 2.f || grid.Y < 2.f) grid = GridSize; // 0일 때 보정
	const float cellPx = (Cols > 0) ? grid.X / Cols : 32.f;

	// 인벤 오버레이에 팔레트 스타일 비주얼 생성
	UWidget* Visual = CreateInventoryDragVisual(Def, DraggedBlockPtr->Rotation, Icon, cellPx);
	if (UOverlaySlot* S = Overlay_Drag->AddChildToOverlay(Visual)) {
		S->SetHorizontalAlignment(HAlign_Left);
		S->SetVerticalAlignment(VAlign_Top);
	}
	DragGhost = Visual;

	// 중앙 피벗 기준: 현재 커서 위치로 1회 초기화
	if (Overlay_Drag)
	{
		const FVector2D p = Overlay_Drag->GetCachedGeometry()
			.AbsoluteToLocal(InPointerEvent.GetScreenSpacePosition());
		if (UWidget* W = DragGhost.Get())
		{
			W->SetRenderTranslation(p);
		}
	}

	if (DraggedBlockPtr) SetBlockHidden(DraggedBlockPtr->Id, true);
	
	// 엔진 기본 드래그 비주얼은 사용 안 함
	BlockDragOp->DefaultDragVisual = nullptr;
	BlockDragOp->SourceInventoryWidget = this;
	OutOperation = BlockDragOp;

	// 드래그가 시작되면 원본 위치의 블록을 숨기거나 제거하는 로직을 추가
	//RemoveBlockFromUI(DraggedBlockPtr->Id);
}

void UDKInventoryWidget::NativeOnDragEnter(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	UE_LOG(LogTemp, Warning, TEXT("[INV] DragEnter"));
	Super::NativeOnDragEnter(InGeo, InDragDropEvent, InOp);
	ClearPreview();
}

void UDKInventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	UE_LOG(LogTemp, Warning, TEXT("[INV] DragLeave"));
	
	// DestroyDragGhost();
	ClearPreview();
	Super::NativeOnDragLeave(InDragDropEvent, InOp);
}

bool UDKInventoryWidget::NativeOnDragOver(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	// 함수 진입 확인
	UE_LOG(LogTemp, Warning, TEXT("[INV] DragOver - START"));

	LastDragScreenPos = InDragDropEvent.GetScreenSpacePosition();

	const auto* Op = Cast<UDKBlockDragDropOp>(InOp);
	if (!Op || Op->DragBlockDef.Cells.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("[INV] DragOver 실패: Op가 유효하지 않거나 셀이 없습니다."));
		ClearPreview();
		return false;
	}
	//// 1단계 통과
	//UE_LOG(LogTemp, Warning, TEXT("[INV] DragOver - Op 유효성 검사 통과."));

	//TArray<FIntPoint> CellsR;
	//FIntPoint GrabR(0, 0);
	//Op->GetRotatedCellsAndGrab(CellsR, GrabR);
	//if (CellsR.Num() == 0) {
	//	UE_LOG(LogTemp, Error, TEXT("[INV] DragOver 실패: 회전된 셀이 비어있습니다."));
	//	ClearPreview();
	//	return false;
	//}
	//// 2단계 통과
	//UE_LOG(LogTemp, Warning, TEXT("[INV] DragOver - 회전된 셀 수: %d"), CellsR.Num());

	//int32 rDrop, cDrop;
	//if (!ScreenToCamouflageCell(InGeo, InDragDropEvent.GetScreenSpacePosition(), rDrop, cDrop))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("[INV] DragOver 실패: 스크린 좌표를 셀로 변환하지 못했습니다."));
	//	ClearPreview();
	//	return false;
	//}
	//// 3단계 통과
	//UE_LOG(LogTemp, Warning, TEXT("[INV] DragOver - 변환된 셀: (%d, %d)."), rDrop, cDrop);

	//int32 r0, c0;
	//bool bCan = false;

	//if (Op->SourceInventory == EDKInventoryType::Dish)
	//{
	//	r0 = rDrop - Op->GrabCell.X;
	//	c0 = cDrop - Op->GrabCell.Y;
	//	bCan = CanMoveBlock(Op->OriginalBlockId, r0, c0, CellsR);
	//}
	//else
	//{
	//	r0 = rDrop - GrabR.X;
	//	c0 = cDrop - GrabR.Y;
	//	bCan = CanPlaceCamouflage(r0, c0, CellsR);
	//}
	//// 최종 결과 확인
	//UE_LOG(LogTemp, Warning, TEXT("[INV] DragOver - 배치 가능: %s"), bCan ? TEXT("true") : TEXT("false"));

	// 1) 고스트는 커서를 자유롭게 따라감 (연속 이동)
	UpdateDragGhostPos_FreeCursor(InDragDropEvent);

	// 2) 프리뷰는 중앙 스냅 좌표 계산 후 그리드에만 배치
	/*TArray<FIntPoint> CellsR; FIntPoint GrabR;
	Op->GetRotatedCellsAndGrab(CellsR, GrabR);*/

	TArray<FIntPoint> CellsR = GetRotatedCells(Op->DragBlockDef, Op->DragRotation);

	//// 바운딩 → centerR, centerC
	//int32 minR = INT_MAX, minC = INT_MAX, maxR = 0, maxC = 0;
	//for (auto& P : CellsR) {
	//	minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y);
	//	maxR = FMath::Max(maxR, P.X); maxC = FMath::Max(maxC, P.Y);
	//}
	//const int32 rows = FMath::Max(1, maxR - minR + 1);
	//const int32 cols = FMath::Max(1, maxC - minC + 1);
	//const int32 centerR = rows / 2;
	//const int32 centerC = cols / 2;

	//int32 rDrop, cDrop;
	//if (!ScreenToCamouflageCell(InGeo, InDragDropEvent.GetScreenSpacePosition(), rDrop, cDrop)) {
	//	ClearPreview(); return false;
	//}
	//const int32 r0 = rDrop - centerR;
	//const int32 c0 = cDrop - centerC;

	//const bool bCan = (Op->SourceInventory == EDKInventoryType::Dish)
	//	? CanMoveBlock(Op->OriginalBlockId, r0, c0, CellsR)
	//	: CanPlaceCamouflage(r0, c0, CellsR);

	//// 프리뷰만 갱신 (타일)
	//PaintPreview(r0, c0, CellsR, bCan);

	// 바운딩 → rows/cols
	int32 minR = INT_MAX, minC = INT_MAX, maxR = INT_MIN, maxC = INT_MIN;
	for (const auto& P : CellsR) {
		minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y);
		maxR = FMath::Max(maxR, P.X); maxC = FMath::Max(maxC, P.Y);
	}
	const int32 rows = FMath::Max(1, maxR - minR + 1);
	const int32 cols = FMath::Max(1, maxC - minC + 1);

	// 중앙(실수) 스냅 기준
	const float centerRf = (rows - 1) * 0.5f;
	const float centerCf = (cols - 1) * 0.5f;

	int32 rDrop, cDrop;
	if (!ScreenToCamouflageCell(InGeo, InDragDropEvent.GetScreenSpacePosition(), rDrop, cDrop))
	{
		ClearPreview(); return false;
	}

	// 좌상단 스냅 (짝수 사이즈도 오류 없이)
	const int32 r0 = FMath::FloorToInt(rDrop - centerRf);
	const int32 c0 = FMath::FloorToInt(cDrop - centerCf);

	// 배치 가능 여부는 드롭과 동일한 함수로
	const bool bCan = (Op->SourceInventory == EDKInventoryType::Dish)
		? CanMoveBlock(Op->OriginalBlockId, r0, c0, CellsR)
		: CanPlaceCamouflage(r0, c0, CellsR);

	// 프리뷰는 r0,c0 + CellsR 그대로 칠하기
	PaintPreview(r0, c0, CellsR, bCan);
	return true;
}

bool UDKInventoryWidget::NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	UE_LOG(LogTemp, Warning, TEXT("[INV] Drop() called"));

	const auto* Op = Cast<UDKBlockDragDropOp>(InOp);
	if (!Op || Op->DragBlockDef.Cells.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("[INV] Drop() Fail: Invalid Op or no cells"));
		ClearPreview();
		return false;
	}

	// 회전/그랩 계산 (CellsR/GrabR는 써도 되고 무시해도 됨)
	/*TArray<FIntPoint> CellsR; FIntPoint GrabR(0, 0);
	Op->GetRotatedCellsAndGrab(CellsR, GrabR);*/

	TArray<FIntPoint> CellsR = GetRotatedCells(Op->DragBlockDef, Op->DragRotation);

	// 🔸 바운딩 & 중앙 오프셋 (프리뷰와 동일)
	int32 minR = INT_MAX, minC = INT_MAX, maxR = INT_MIN, maxC = INT_MIN;
	for (const auto& P : CellsR) {
		minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y);
		maxR = FMath::Max(maxR, P.X); maxC = FMath::Max(maxC, P.Y);
	}
	const int32 rows = FMath::Max(1, maxR - minR + 1);
	const int32 cols = FMath::Max(1, maxC - minC + 1);
	const int32 centerR = rows / 2;
	const int32 centerC = cols / 2;

	int32 rDrop, cDrop;
	if (!ScreenToCamouflageCell(InGeo, InDragDropEvent.GetScreenSpacePosition(), rDrop, cDrop)) {
		ClearPreview(); return false;
	}

	// 드롭 기준도 중앙 스냅으로
	const int32 r0 = rDrop - centerR;
	const int32 c0 = cDrop - centerC;

	const bool bCan = (Op->SourceInventory == EDKInventoryType::Dish)
		? CanMoveBlock(Op->OriginalBlockId, r0, c0, CellsR)
		: CanPlaceCamouflage(r0, c0, CellsR);


	if (!bCan) {
		ClearPreview();
		return false;
	}

	
	if (Op->SourceInventory == EDKInventoryType::Dish)
	{
		MoveBlock(Op->OriginalBlockId, r0, c0, Op->DragRotation);
	}
	else 
	{
		FPlacedBlock Out;
		Out.DishId = Op->Dish->GetItemID();
		Out.IconTexture = Op->BlockIconTexture;

		PlaceCamouflage_At(r0, c0, CellsR, Op->DragBlockDef.ShapeId, Op->DragRotation, Out);
		PaintBlockCamouflage(Out);

		// Clear the palette
		if (UGameInstance* GI = GetGameInstance())
		{
			auto* Cooking = GI->GetSubsystem<UCookingSystem>();
			auto* Digest = GI->GetSubsystem<UDigestiveSystem>();

			UDKDish* Dish = nullptr;
			if (Op && Op->Dish)           Dish = Op->Dish.Get();
			if (!Dish && Cooking)         Dish = Cooking->GetMakedDish();

			if (Digest && Dish && !Dish->bIsEffect)
			{
				Digest->AddDish(Dish); // 버프 적용
			}
			if (Cooking)
			{
				Cooking->BringDish();
				Cooking->OnDishCreated.Broadcast(nullptr);
			}
		}
	}

	DestroyDragGhost();
	ClearPreview();
	return true;
}

void UDKInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (DraggedBlockPtr) {
		SetBlockHidden(DraggedBlockPtr->Id, false); // 원래 자리에 복원
	}
	DestroyDragGhost();
	ClearPreview();
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

UWidget* UDKInventoryWidget::GetCell(int32 R, int32 C) const
{
	const int32 Idx = R * Cols + C;
	return (CellWidgets.IsValidIndex(Idx) ? CellWidgets[Idx] : nullptr);
}


UWidget* UDKInventoryWidget::CreateInventoryDragVisual(const struct FBlockShapeDef* Def, int32 Rotation, class UTexture2D* DragIcon, float CellPx)
{
	// 기존 회전 + 정규화 로직 (유지)
	TArray<FIntPoint> Cells = Def ? Def->Cells : TArray<FIntPoint>{ FIntPoint(0,0) };
	Rotation = (Rotation % 4 + 4) % 4;
	for (int k = 0;k < Rotation;k++) {
		for (auto& P : Cells) { const int x = P.X, y = P.Y; P.X = -y; P.Y = x; }
		int minR = INT_MAX, minC = INT_MAX;
		for (const auto& P : Cells) { minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y); }
		for (auto& P : Cells) { P.X -= minR; P.Y -= minC; }
	}

	// 바운딩 박스 계산 (유지)
	int32 minR = INT_MAX, minC = INT_MAX, maxR = 0, maxC = 0;
	for (const auto& C : Cells) {
		minR = FMath::Min(minR, C.X); minC = FMath::Min(minC, C.Y);
		maxR = FMath::Max(maxR, C.X); maxC = FMath::Max(maxC, C.Y);
	}
	DragGhostMinR = minR; DragGhostMinC = minC;
	DragGhostRows = FMath::Max(1, maxR - minR + 1);
	DragGhostCols = FMath::Max(1, maxC - minC + 1);

	// ----------------------------------------------------
	// ★★★ 수정된 부분: Root 위젯의 크기 및 회전 설정
	// ----------------------------------------------------
	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());

	// 회전된 크기를 사용하여 SizeBox의 크기 설정
	const bool bOddRotation = ((Rotation % 4 + 4) % 4) & 1;
	if (bOddRotation) {
		Root->SetWidthOverride(DragGhostRows * CellPx);
		Root->SetHeightOverride(DragGhostCols * CellPx);
	}
	else {
		Root->SetWidthOverride(DragGhostCols * CellPx);
		Root->SetHeightOverride(DragGhostRows * CellPx);
	}

	// 나머지 코드 (유지)
	Root->SetVisibility(ESlateVisibility::HitTestInvisible);
	Root->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 격자(블록 셀) 레이어 (유지)
	if (Cells.Num() > 0) {
		TSet<FIntPoint> S; for (auto& P : Cells) S.Add(P);
		UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());
		Grid->SetVisibility(ESlateVisibility::HitTestInvisible);
		for (int32 r = 0;r < DragGhostRows;++r)
			for (int32 c = 0;c < DragGhostCols;++c) {
				USizeBox* Cell = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
				Cell->SetWidthOverride(CellPx); Cell->SetHeightOverride(CellPx);
				Cell->SetVisibility(ESlateVisibility::HitTestInvisible);
				UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				FSlateBrush Br; Br.DrawAs = ESlateBrushDrawType::Image;
				Br.ImageSize = FVector2D(CellPx, CellPx);
				const bool bFilled = S.Contains(FIntPoint(r, c));
				Br.TintColor = FSlateColor(FLinearColor::Transparent);
				Img->SetBrush(Br);
				Img->SetVisibility(ESlateVisibility::HitTestInvisible);
				Cell->AddChild(Img);
				Grid->AddChildToUniformGrid(Cell, r, c);
			}
		UBorder* Outline = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Outline->SetPadding(FMargin(0));
		Outline->SetBrushColor(FLinearColor(0, 0, 0, 0));
		Outline->SetContent(Grid);
		Overlay->AddChildToOverlay(Outline);
	}

	// ----------------------------------------------------
	// ★★★ 수정된 부분: 아이콘 레이어에 회전 로직 추가
	// ----------------------------------------------------
	if (DragIcon) {
		UScaleBox* Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
		Scale->SetStretch(EStretch::ScaleToFit); // 비율 유지
		Scale->SetStretchDirection(EStretchDirection::Both);
		Scale->SetVisibility(ESlateVisibility::HitTestInvisible);

		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Img->SetBrushFromTexture(DragIcon, /*bMatchSize*/ true);
		Img->SetRenderOpacity(0.95f);
		Img->SetVisibility(ESlateVisibility::HitTestInvisible);

		Scale->AddChild(Img);
		if (UOverlaySlot* S = Overlay->AddChildToOverlay(Scale)) {
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetVerticalAlignment(VAlign_Center);
		}

		// Render Transform을 ScaleBox에 적용
		Scale->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		FWidgetTransform RotXform;
		RotXform.Angle = Rotation * 90.f;
		Scale->SetRenderTransform(RotXform);
	}

	Root->AddChild(Overlay);
	return Root;

}

void UDKInventoryWidget::UpdateDragGhostPos_CenterPivot(int32 r0, int32 c0)
{
	if (!DragGhost.IsValid() || !Grid_Camouflage || !Overlay_Blocks) return;

	const FGeometry& GGrid = Grid_Camouflage->GetCachedGeometry();
	const FGeometry& GOverlay = Overlay_Blocks->GetCachedGeometry();

	const FVector2D gridSize = GGrid.GetLocalSize();
	if (gridSize.X < 2.f || gridSize.Y < 2.f || Rows <= 0 || Cols <= 0) return;

	const float cellW = gridSize.X / Cols;
	const float cellH = gridSize.Y / Rows;

	// 블록 좌상단(바운딩) 셀 좌표
	const int32 baseR = r0 + DragGhostMinR;
	const int32 baseC = c0 + DragGhostMinC;

	// ── 1) 그리드 로컬(셀 좌상단) → 절대 → 오버레이 로컬(셀 좌상단)
	const FVector2D gridLocalTopLeft(baseC * cellW, baseR * cellH);
	const FVector2D absTopLeft = GGrid.LocalToAbsolute(gridLocalTopLeft);
	FVector2D overlayLocalTopLeft = GOverlay.AbsoluteToLocal(absTopLeft);

	// 좌상단 피벗이므로 그대로 넣는다 (half-size 보정 전부 제거)
	DragGhost->SetRenderTranslation(overlayLocalTopLeft);

	if (USizeBox* SB = Cast<USizeBox>(DragGhost)) {
		SB->SetWidthOverride(DragGhostCols * cellW);
		SB->SetHeightOverride(DragGhostRows * cellH);
	}

	// (디버그)
	UE_LOG(LogTemp, Verbose, TEXT("[Ghost] r0=%d c0=%d tl=%s cell=(%.1f,%.1f)"),
		r0, c0, *overlayLocalTopLeft.ToString(), cellW, cellH);
}

void UDKInventoryWidget::UpdateDragGhostPos_FreeCursor(const FDragDropEvent& E)
{
	if (!Overlay_Drag) return;
	if (UWidget* W = DragGhost.Get())
	{
		const FVector2D p = Overlay_Drag->GetCachedGeometry()
			.AbsoluteToLocal(E.GetScreenSpacePosition());

		// 고스트 중앙을 마우스에 맞춤
		FVector2D size(0, 0);
		if (USizeBox* SB = Cast<USizeBox>(W))
		{
			size.X = SB->GetWidthOverride();
			size.Y = SB->GetHeightOverride();
		}
		else if (UImage* Img = Cast<UImage>(W))
		{
			size = DragGhostSize; // (CreateInventoryDragVisual 등에서 채워둔 값)
		}

		W->SetRenderTranslation(p - size * 0.5f);
	}
}

void UDKInventoryWidget::RotateActiveDrag(int32 DeltaSteps)
{
	UE_LOG(LogTemp, Log, TEXT("RotateActoiveDrag1"));
	// 현재 드래그 오퍼레이션
	UDragDropOperation* Op = UWidgetBlueprintLibrary::GetDragDroppingContent();
	UDKBlockDragDropOp* BOp = Op ? Cast<UDKBlockDragDropOp>(Op) : nullptr;
	if (!BOp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Rotate] EARLY RETURN: DragDropOperation이 UDKBlockDragDropOp가 아님(혹은 드래그 중 아님)."));
		return;
	}
	if (!Grid_Camouflage)
	{
		UE_LOG(LogTemp, Error, TEXT("[Rotate] EARLY RETURN: Grid_Camouflage가 nullptr. (BindWidget 누락/InitCamouflageGrid 전에 호출)"));
		return;
	}
	if (!Overlay_Drag)
	{
		UE_LOG(LogTemp, Error, TEXT("[Rotate] EARLY RETURN: Overlay_Drag가 nullptr. (UpdateGridDimensions에서 오버레이 생성 안 됨)"));
		return;
	}

	// 0..3 스텝 회전
	BOp->DragRotation = (BOp->DragRotation + DeltaSteps) & 3;

	// 기존 고스트 제거
	if (UWidget* Old = DragGhost.Get())
	{
		Overlay_Drag->RemoveChild(Old);
	}
	DragGhost.Reset();

	// 셀 픽셀 크기 계산
	GridSize = Grid_Camouflage->GetCachedGeometry().GetLocalSize();
	const float CellPx = (Cols > 0 && GridSize.X > 1.f) ? GridSize.X / Cols : 32.f;

	// ★ SoftObject → UTexture2D* 로 해제
	UTexture2D* IconTex = nullptr;
	if (!BOp->BlockIconTexture.IsNull())
	{
		IconTex = BOp->BlockIconTexture.Get();
		if (!IconTex) IconTex = BOp->BlockIconTexture.LoadSynchronous();
	}

	// 새 고스트 생성
	UWidget* Visual = CreateInventoryDragVisual(
		&BOp->DragBlockDef,
		BOp->DragRotation,
		IconTex,                 // ← 포인터로 전달
		CellPx
	);

	if (UOverlaySlot* S = Overlay_Drag->AddChildToOverlay(Visual))
	{
		S->SetHorizontalAlignment(HAlign_Left);
		S->SetVerticalAlignment(VAlign_Top);
	}
	DragGhost = Visual;

	// 마우스 "정중앙"에 맞추기(반 사이즈 보정)
	if (UWidget* W = DragGhost.Get())
	{
		const FVector2D p = Overlay_Drag->GetCachedGeometry()
			.AbsoluteToLocal(LastDragScreenPos);

		FVector2D size(0, 0);
		if (USizeBox* SB = Cast<USizeBox>(W))
		{
			size.X = SB->GetWidthOverride();
			size.Y = SB->GetHeightOverride();
		}
		else if (UImage* Img = Cast<UImage>(W))
		{
			size = DragGhostSize; // CreateInventoryDragVisual에서 계산해 둔 값
		}
		W->SetRenderTranslation(p - size * 0.5f);
	}

	// 프리뷰 즉시 갱신(중앙 스냅 공식과 동일)
	TArray<FIntPoint> CellsR = GetRotatedCells(BOp->DragBlockDef, BOp->DragRotation);
	int32 minR = INT_MAX, minC = INT_MAX, maxR = INT_MIN, maxC = INT_MIN;
	for (const auto& P : CellsR)
	{
		minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y);
		maxR = FMath::Max(maxR, P.X); maxC = FMath::Max(maxC, P.Y);
	}

	const int32 rows = FMath::Max(1, maxR - minR + 1);
	const int32 cols = FMath::Max(1, maxC - minC + 1);
	// const int32 centerR = rows / 2, centerC = cols / 2;
	const float centerRf = (rows - 1) * 0.5f;
	const float centerCf = (cols - 1) * 0.5f;

	int32 rDrop, cDrop;
	if (ScreenToCamouflageCell(Grid_Camouflage->GetCachedGeometry(), LastDragScreenPos, rDrop, cDrop))
	{
		/*const int32 r0 = rDrop - centerR;
		const int32 c0 = cDrop - centerC;*/

		const int32 r0 = FMath::FloorToInt(rDrop - centerRf);
		const int32 c0 = FMath::FloorToInt(cDrop - centerCf);

		const bool bCan = (BOp->SourceInventory == EDKInventoryType::Dish)
			? CanMoveBlock(BOp->OriginalBlockId, r0, c0, CellsR)
			: CanPlaceCamouflage(r0, c0, CellsR);

		PaintPreview(r0, c0, CellsR, bCan);
	}
	else
	{
		ClearPreview();
	}
}



void UDKInventoryWidget::InitCamouflageGrid()
{
	CamOcc.Init(false, Rows * Cols);
	PaintClearCamouflage();

}


void UDKInventoryWidget::PaintClearCamouflage()
{
	if (!Grid_Camouflage) return;
	const int32 N = Grid_Camouflage->GetChildrenCount();
	for (int32 i = 0; i < N; ++i)
	{
		if (UWidget* W = Grid_Camouflage->GetChildAt(i))
			ApplyColorToSlotWidget(W, GridIdleColor);
	}
}


void UDKInventoryWidget::UpdateGridDimensions(int32 inRows, int32 inCols)
{
	Rows = inRows;
	Cols = inCols;

	if (!Grid_Camouflage) return;
    Grid_Camouflage->ClearChildren();

	// 루트는 드롭 받아야 하므로
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);
	// 그리드는 입력 패스
	Grid_Camouflage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    // 그리드의 부모를 Overlay로 감싸 전체 이미지를 얹을 레이어 구성
    if (UPanelWidget* Parent = Cast<UPanelWidget>(Grid_Camouflage->GetParent()))
    {
        UOverlay* AsOverlay = Cast<UOverlay>(Parent);
        if (!AsOverlay)
        {
            const int32 InsertIdx = Parent->GetChildIndex(Grid_Camouflage);
            Parent->RemoveChild(Grid_Camouflage);
            AsOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
            Parent->InsertChildAt(InsertIdx, AsOverlay);
        }

        // 기존 Overlay_Blocks 제거 후 재생성
        if (Overlay_Blocks)
        {
            AsOverlay->RemoveChild(Overlay_Blocks);
            Overlay_Blocks = nullptr;
        }

        // 먼저 그리드를 올리고
        if (!AsOverlay->HasAnyChildren() || AsOverlay->GetChildIndex(Grid_Camouflage) == INDEX_NONE)
        {
            AsOverlay->AddChildToOverlay(Grid_Camouflage);
        }
        // 그 위에 블록 오버레이 추가
        Overlay_Blocks = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
        if (Overlay_Blocks)
        {
            UOverlaySlot* BlockSlot = AsOverlay->AddChildToOverlay(Overlay_Blocks);
            if (BlockSlot)
            {
				BlockSlot->SetHorizontalAlignment(HAlign_Fill);
				BlockSlot->SetVerticalAlignment(VAlign_Fill);
            }
            Overlay_Blocks->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

		Overlay_Drag = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		Overlay_Preview = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		auto AddFill = [&](UOverlay* X) {
			if (!X) return;
			if (UOverlaySlot* S = AsOverlay->AddChildToOverlay(X)) {
				S->SetHorizontalAlignment(HAlign_Fill);
				S->SetVerticalAlignment(VAlign_Fill);
			}
			X->SetVisibility(ESlateVisibility::HitTestInvisible);
			};
		// 순서 중요: Blocks(아래) → Drag(중간) → Preview(최상단)
		AddFill(Overlay_Drag);
		AddFill(Overlay_Preview);
    }

	CellWidgets.Empty();
	CellWidgets.SetNum(Rows * Cols);

	for (int32 r = 0; r < Rows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			UDKPantrySlotWidget* SlotW = WidgetTree->ConstructWidget<UDKPantrySlotWidget>(SlotClass);
			if (SlotW)
			{
				Grid_Camouflage->AddChildToUniformGrid(SlotW, r, c);
				SlotW->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

				ApplyColorToSlotWidget(SlotW, GridIdleColor);

				const int32 Idx = r * Cols + c;
				CellWidgets[Idx] = SlotW; // ★★★ 직접 캐시
			}
		}
	}

	CamOcc.Init(false, Rows * Cols);

	// 디버그: 실제 채워졌는지 확인
	UE_LOG(LogTemp, Warning, TEXT("[INV] Grid children=%d, cells cached=%d"),
		Grid_Camouflage->GetChildrenCount(), CellWidgets.Num());

    // 전체 이미지는 CamPlaced 기반 페인트에서 생성

}


TArray<FIntPoint> UDKInventoryWidget::GetRotatedCells(const FBlockShapeDef& Def, int32 Rot) const
{
	// 회전 값을 0~3 범위로 정규화
	Rot = (Rot % 4 + 4) % 4;
	TArray<FIntPoint> Cells = Def.Cells;

	for (int32 k = 0;k < Rot;k++)
	{
		// 90도 회전 공식 (x, y) -> (y, -x) 적용
		for (FIntPoint& P : Cells) { int x = P.X, y = P.Y; P.X = y; P.Y = -x; }
		int minR = INT_MAX, minC = INT_MAX;
		// 회전 후 블록의 좌표가 0,0을 기준으로 재배치되도록 오프셋을 계산
		for (const FIntPoint& P : Cells) { minR = FMath::Min(minR, P.X); minC = FMath::Min(minC, P.Y); }
		for (FIntPoint& P : Cells) { P.X -= minR; P.Y -= minC; }
	}
	return Cells;
}


bool UDKInventoryWidget::CanPlaceCamouflage(int32 r0, int32 c0, const TArray<FIntPoint>& Cells) const
{
	// 1단계: 그리드 경계 이탈 검사
		for (const FIntPoint& d : Cells)
		{
			const int32 R = r0 + d.X;
			const int32 C = c0 + d.Y;
			if (R < 0 || R >= Rows || C < 0 || C >= Cols) {
				UE_LOG(LogTemp, Error, TEXT("Placement failed: Out of bounds at R=%d, C=%d"), R, C);
				return false; // 그리드 경계 확인
			}
		}

	// 2단계: 점유 상태(다른 블록과의 충돌) 검사
	for (const FIntPoint& d : Cells)
	{
		const int32 R = r0 + d.X;
		const int32 C = c0 + d.Y;
		const int32 Idx = R * Cols + C;

		if (!CamOcc.IsValidIndex(Idx) || CamOcc[Idx]) {
			UE_LOG(LogTemp, Error, TEXT("Placement failed: Cell occupied or invalid index at R=%d, C=%d"), R, C);
			return false; // 점유 상태 확인
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Placement successful: All cells are valid and empty."));
	return true; // 배치 가능
}

FLinearColor UDKInventoryWidget::GetBaseColorAtCell(int32 R, int32 C) const
{
	if (R < 0 || R >= Rows || C < 0 || C >= Cols) return GridIdleColor;

	// ★ 변경: 정의 조회
	if (UGameInstance* GI = GetGameInstance())
	{
		if (Inv)
		{
			for (int32 i = CamPlaced.Num() - 1; i >= 0; --i)
			{
				const FPlacedBlock& B = CamPlaced[i];
				// 투명 반환
				/*if (HiddenBlocksVisual.Contains(B.Id))
					continue;*/

				if (const FBlockShapeDef* Def = Inv->GetBlock(B.ShapeId))
				{
					const TArray<FIntPoint> L = GetRotatedCells(*Def, B.Rotation);
					for (const FIntPoint& P : L)
					{
						const int r = B.Row + P.X;
						const int c = B.Col + P.Y;
						if (r == R && c == C)
						{
							return EffectiveBlockColor(Def->Color); // ★ 변경
						}
					}
				}
			}
		}
	}
	return GridIdleColor;
}

void UDKInventoryWidget::ClearPreview()
{
	UE_LOG(LogTemp, Warning, TEXT("ClearPreview1"));
	if (!Grid_Camouflage) return;
	UE_LOG(LogTemp, Warning, TEXT("ClearPreview1-1"));
	for (int32 Packed : PreviewIndices)
	{

		int32 R, C;
		UnpackRC(Packed, R, C);

		if (UWidget* W = GetCell(R, C))   // ← 여기!
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearPreview2"));
			const FLinearColor Base = GetBaseColorAtCell(R, C);
			if (!ApplyColorToSlotWidget(W, Base))
			{
				UE_LOG(LogTemp, Warning, TEXT("ClearPreview3"));
				// W->SetRenderOpacity(Base.A);
				if (UUserWidget* UW = Cast<UUserWidget>(W)) UW->SetColorAndOpacity(Base);
				else W->SetRenderOpacity(Base.A);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearPreview: child nullptr at r=%d c=%d"), R, C);
		}
	}
	PreviewIndices.Reset();

	/*if (!Overlay_Preview) return;
	for (auto& T : PreviewTiles)
		if (T.IsValid()) Overlay_Preview->RemoveChild(T.Get());
	PreviewTiles.Reset();*/
}

void UDKInventoryWidget::PaintPreview(int32 r0, int32 c0, const TArray<FIntPoint>& Cells, bool bPlaceable)
{
	UE_LOG(LogTemp, Warning, TEXT("PaintPreview0"));
	ClearPreview();
	if (!Grid_Camouflage) return;
	UE_LOG(LogTemp, Warning, TEXT("PaintPreview1"));

	const FLinearColor Col = bPlaceable ? PreviewOK : PreviewBAD;

	for (const FIntPoint& d : Cells)
	{
		const int32 R = r0 + d.X;
		const int32 C = c0 + d.Y;
		if (R < 0 || R >= Rows || C < 0 || C >= Cols) continue;

		if (UWidget* W = GetCell(R, C))  // ★★★ 캐시 사용
		{
			// ApplyColorToSlotWidget(W, Col);
			if (!ApplyColorToSlotWidget(W, Col))
			{
				if (UUserWidget* UW = Cast<UUserWidget>(W)) UW->SetColorAndOpacity(Col);
				else W->SetRenderOpacity(Col.A);
			}
			PreviewIndices.AddUnique(PackRC(R, C));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[INV] PaintPreview: cell null r=%d c=%d"), R, C);
		}
	}

	//ClearPreview();
	//if (!Grid_Camouflage || !Overlay_Preview) return;

	//const FGeometry& GG = Grid_Camouflage->GetCachedGeometry();
	//const FGeometry& GP = Overlay_Preview->GetCachedGeometry();
	//const FVector2D size = GG.GetLocalSize();
	//if (Rows <= 0 || Cols <= 0 || size.X < 2.f || size.Y < 2.f) return;

	//const float cw = size.X / Cols;
	//const float ch = size.Y / Rows;
	//const FLinearColor col = bPlaceable ? PreviewOK : PreviewBAD;

	//for (const FIntPoint& d : Cells)
	//{
	//	const int32 R = r0 + d.X, C = c0 + d.Y;
	//	if (R < 0 || R >= Rows || C < 0 || C >= Cols) continue;

	//	// 그리드 로컬 → 절대 → 프리뷰 오버레이 로컬
	//	const FVector2D tl = GP.AbsoluteToLocal(GG.LocalToAbsolute(FVector2D(C * cw, R * ch)));

	//	UImage* tile = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	//	FSlateBrush br; br.DrawAs = ESlateBrushDrawType::Box; br.TintColor = FSlateColor(col);
	//	tile->SetBrush(br);
	//	tile->SetDesiredSizeOverride({ cw, ch });
	//	tile->SetRenderTranslation(tl);
	//	tile->SetVisibility(ESlateVisibility::HitTestInvisible);

	//	Overlay_Preview->AddChildToOverlay(tile);
	//	PreviewTiles.Add(tile);
	//}
}


bool UDKInventoryWidget::PlaceCamouflage_FirstFit(const FBlockShapeDef& Def, int32 Rot, FPlacedBlock& Out)
{
	TArray<FIntPoint> CellsR = GetRotatedCells(Def, Rot);

	for (int32 r = 0; r < Rows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			if (CanPlaceCamouflage(r, c, CellsR))
			{
				PlaceCamouflage_At(r, c, CellsR, /*ShapeId=*/Def.ShapeId, Rot, Out); // ★ 유지
				PaintBlockCamouflage(Out);
				return true;
			}
		}
	}
	return false;
}


void UDKInventoryWidget::PlaceCamouflage_At(int32 r0, int32 c0, const TArray<FIntPoint>& Cells, int32 ShapeId, int32 Rot, FPlacedBlock& Out)
{
	Out.Row = r0;
	Out.Col = c0;
	Out.Rotation = Rot;
	Out.ShapeId = ShapeId;
	//Out.RotatedCells = Cells;      // 유지할 거면
	Out.RotatedCells.Reset();
	Out.Id = FGuid::NewGuid();

	// 점유 갱신
	for (const FIntPoint& d : Cells)
	{
		const int32 R = r0 + d.X;
		const int32 C = c0 + d.Y;
		const int32 Idx = R * Cols + C; // (또는 CamCols)

		if (CamOcc.IsValidIndex(Idx))
			CamOcc[Idx] = true;
	}

	CamPlaced.Add(Out);

	// Digest에도 즉시 동기화 저장 (세션 동안 유지)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Digest = GI->GetSubsystem<UDigestiveSystem>())
		{
			Digest->SavedDishBlocks = CamPlaced;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("--- CamPlaced Array Contents ---"));
	for (int32 i = 0; i < CamPlaced.Num(); ++i)
	{
		const FPlacedBlock& B = CamPlaced[i];

		UE_LOG(LogTemp, Log, TEXT("Index: %d, ShapeId: %d, Row: %d, Col: %d, Rotation: %d, DishId : %d"),
			i,
			B.ShapeId,
			B.Row,
			B.Col,
			B.Rotation,
			B.DishId
		);
	}
	UE_LOG(LogTemp, Log, TEXT("--------------------------------"));
}


bool UDKInventoryWidget::ScreenToCamouflageCell(const FGeometry& InGeo, const FVector2D& ScreenPos, int32& OutRow, int32& OutCol) const
{
	OutRow = OutCol = -1;
	if (!Grid_Camouflage) return false;

	// 그리드의 지오메트리 정보를 가져옴
	const FGeometry& G = Grid_Camouflage->GetCachedGeometry();
	// 스크린 좌표를 그리드 위젯의 로컬 좌표로 변환
	FVector2D Local = G.AbsoluteToLocal(ScreenPos);

	const FVector2D Size = G.GetLocalSize();
	if (Size.X <= 1 || Size.Y <= 1) return false;

	// 셀의 크기를 계산
	const float CellW = Size.X / Cols;
	const float CellH = Size.Y / Rows;

	// 로컬 좌표를 셀 인덱스로 변환
	int32 c = FMath::FloorToInt(Local.X / CellW);
	int32 r = FMath::FloorToInt(Local.Y / CellH);

	// 변환된 셀 인덱스가 유효한지 확인
	if (r >= 0 && r < Rows && c >= 0 && c < Cols)
	{
		OutRow = r; OutCol = c;
		return true;
	}
	return false;

}


void UDKInventoryWidget::PaintBlockCamouflage(const FPlacedBlock& B)
{
	if (!Grid_Camouflage) return;

	// ★ 변경: 정의 조회
	Inv = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPlayerInventoryManager>()
		: nullptr;
	if (!Inv) return;

	const FBlockShapeDef* Def = Inv->GetBlock(B.ShapeId);
	if (!Def) return;

	// 셀 좌표: 배치 시 저장된 회전/정규화 셀을 우선 사용
	/*const TArray<FIntPoint> Cells = (B.RotatedCells.Num() > 0)
		? B.RotatedCells
		: GetRotatedCells(*Def, B.Rotation);*/
	const TArray<FIntPoint> Cells = GetRotatedCells(*Def, B.Rotation);
	const FLinearColor PaintCol = EffectiveBlockColor(Def->Color);

	// ★★★ 통합된 아이콘 소스 로직 (PaintBlockWholeImage에서 가져옴) ★★★
	Icon = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* DM = GI->GetSubsystem<class UDataManager>())
		{
			if (B.DishId >= 0)
			{
				const FDKDishData DishData = DM->GetDishData(B.DishId);
				Icon = DishData.ItemIcon;
			}
		}
	}

	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		const FIntPoint& P = Cells[i];
		const int R = B.Row + P.X, C = B.Col + P.Y;
		if (R < 0 || R >= Rows || C < 0 || C >= Cols) continue;

		if (UWidget* W = GetCell(R, C))
		{
			// ★★★ Icon 변수를 사용하여 셀 텍스처를 적용 ★★★
			if (Icon)
			{
				if (!ApplyTextureToSlotWidget(W, Icon, FLinearColor(1.f, 1.f, 1.f, 1.f)))
				{
					UE_LOG(LogTemp, Warning, TEXT("[INV] ApplyTexture failed at r=%d c=%d, falling back to color."), R, C);
					ApplyColorToSlotWidget(W, PaintCol);
				}
			}
			else
			{
				// 아이콘을 찾지 못하면 기본 색상으로 적용
				ApplyColorToSlotWidget(W, PaintCol);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[INV] PaintBlock: child null r=%d c=%d"), R, C);
		}
	}

	const FGeometry& G = Grid_Camouflage->GetCachedGeometry();
	GridSize = G.GetLocalSize();

	// 블록 전체 이미지를 상단 오버레이에 그림
	PaintBlockWholeImage(B, Cells);
}

void UDKInventoryWidget::PaintBlockWholeImage(const FPlacedBlock& B, const TArray<FIntPoint>& Cells)
{
	if (!Overlay_Blocks || !Grid_Camouflage) return;

	// 1) 아이콘 로드
	UTexture2D* LocalIcon = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* DM = GI->GetSubsystem<UDataManager>())
		{
			if (B.DishId >= 0)
			{
				const FDKDishData DishData = DM->GetDishData(B.DishId);
				LocalIcon = DishData.ItemIcon;
			}
		}
	}
	if (!LocalIcon) return;

	// 2) 셀 크기
	const FVector2D GridLocalSize = Grid_Camouflage->GetCachedGeometry().GetLocalSize();
	if (GridLocalSize.X <= 1.f || GridLocalSize.Y <= 1.f || Cols <= 0 || Rows <= 0) return;

	/*const float CellW = GridLocalSize.X / Cols;
	const float CellH = GridLocalSize.Y / Rows;*/

	// 1. Cells를 블록의 Rotation에 맞게 회전 및 정규화 (CreateDragGhost와 동일한 방식)
	TArray<FIntPoint> RotatedCells = GetRotatedCells(*Inv->GetBlock(B.ShapeId), B.Rotation);

	// 2. 회전된 Cells 배열을 기준으로 바운딩 박스 계산
	int32 minR = INT_MAX, minC = INT_MAX, maxR = INT_MIN, maxC = INT_MIN;
	for (const FIntPoint& P : RotatedCells)
	{
		minR = FMath::Min(minR, P.X);
		minC = FMath::Min(minC, P.Y);
		maxR = FMath::Max(maxR, P.X);
		maxC = FMath::Max(maxC, P.Y);
	}
	if (minR == INT_MAX) return;

	const int32 rows = (maxR - minR + 1);
	const int32 cols = (maxC - minC + 1);

	// 3. 픽셀 위치/크기 계산: B.Row, B.Col은 블록의 앵커(중앙 스냅으로 저장된 시작점)일 수 있으므로
	// B.Row/B.Col이 이미 블록의 좌상단을 가리킨다는 가정 하에 계산합니다.
	// (만약 B.Row/B.Col이 중앙 앵커라면, 이 계산은 NativeOnDrop 수정 후의 값이라고 가정)

	const float CellW = GridSize.X / Cols;
	const float CellH = GridSize.Y / Rows;

	// USizeBox의 위치는 B.Row, B.Col로 이미 결정되어 있어야 합니다.
	const FVector2D Pos(B.Col * CellW, B.Row * CellH);
	const FVector2D Dim(cols * CellW, rows * CellH);

	// ★★★ 위젯 계층 구조 및 속성 설정

	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	if (!Box) return;
	Box->SetWidthOverride(Dim.X);
	Box->SetHeightOverride(Dim.Y);
	Box->SetVisibility(ESlateVisibility::Visible);
	Box->SetRenderTranslation(Pos);

	// 투명 Border (배경 투명화)
	UBorder* TransparentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	TransparentBorder->SetBrushColor(FLinearColor::Transparent);
	TransparentBorder->SetPadding(FMargin(0));
	TransparentBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// ScaleBox (비율 유지)
	UScaleBox* Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
	Scale->SetStretch(EStretch::ScaleToFit);
	Scale->SetStretchDirection(EStretchDirection::Both);
	Scale->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Image (아이콘)
	UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	if (!Img) return;
	Img->SetBrushFromTexture(Icon, /*bMatchSize*/ true);
	Img->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	Scale->AddChild(Img);
	TransparentBorder->SetContent(Scale);
	Box->AddChild(TransparentBorder);

	if (UOverlaySlot* DishSlot = Overlay_Blocks->AddChildToOverlay(Box))
	{
		// ★★★ OverlaySlot 정렬은 Left/Top으로 유지 (RenderTranslation을 존중하기 위해)
		DishSlot->SetHorizontalAlignment(HAlign_Left);
		DishSlot->SetVerticalAlignment(VAlign_Top);
	}

	// ★★★ ScaleBox 회전 적용 (이미지 자체를 돌림)
	Scale->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	FWidgetTransform Xf;
	Xf.Angle = B.Rotation * 90.f;
	Scale->SetRenderTransform(Xf);

	// 7) 툴팁: Box가 호버를 받으므로 여기에 부착
	AttachDishTooltipToWidget(Box, B.DishId);

	// 8) 드래그/숨김 복원 등을 위해 보관 (기존 맵 사용 유지)
	BlockOverlayMap.FindOrAdd(B.Id) = Img;
}

// RebuildWholeImages 제거: RebuildFromPlaced 내에서 처리


void UDKInventoryWidget::RebuildFromPlaced()
{
    if (!Grid_Camouflage) return;
    // 셀 색 초기화
    PaintClearCamouflage();
    // 오버레이 초기화
    if (Overlay_Blocks) Overlay_Blocks->ClearChildren();

	CamOcc.Init(false, Rows * Cols);

	if (!Inv) {
		if (UGameInstance* GI = GetGameInstance())
			Inv = GI->GetSubsystem<UPlayerInventoryManager>();
	}

	if (Inv)
	{
		for (const FPlacedBlock& B : CamPlaced)
		{
			const FBlockShapeDef* Def = Inv->GetBlock(B.ShapeId);
			if (!Def) continue;

			/*const TArray<FIntPoint> Cells = (B.RotatedCells.Num() > 0)
				? B.RotatedCells
				: GetRotatedCells(*Def, B.Rotation);*/

			const TArray<FIntPoint> Cells = GetRotatedCells(*Def, B.Rotation);

			// 디버그 
			FString PerBlock;
			PerBlock.Reserve(Cells.Num() * 16);

			for (const FIntPoint& d : Cells)
			{
				const int32 R = B.Row + d.X;
				const int32 C = B.Col + d.Y;
				const int32 Idx = R * Cols + C;
				if (CamOcc.IsValidIndex(Idx)) CamOcc[Idx] = true;

				// 문자열 누적
				PerBlock += FString::Printf(TEXT("(%d,%d) "), R, C);
			}
			UE_LOG(LogTemp, Log, TEXT("[OCC]   world-cells: %s"), *PerBlock);
		}
	}


    // CamPlaced에 저장된 데이터만으로 복원
    for (const FPlacedBlock& B : CamPlaced)
    {
        PaintBlockCamouflage(B);
    }
}

void UDKInventoryWidget::RecomputeCamOccFromPlaced()
{
	CamOcc.Init(false, Rows * Cols);

	if (!Inv) {
		if (UGameInstance* GI = GetGameInstance())
			Inv = GI->GetSubsystem<UPlayerInventoryManager>();
	}
	if (!Inv) return;

	for (const FPlacedBlock& B : CamPlaced)
	{
		if (const FBlockShapeDef* Def = Inv->GetBlock(B.ShapeId))
		{
			/*const TArray<FIntPoint> RC = (B.RotatedCells.Num() > 0)
				? B.RotatedCells
				: GetRotatedCells(*Def, B.Rotation);*/

			const TArray<FIntPoint> RC = GetRotatedCells(*Def, B.Rotation);

			for (const FIntPoint& d : RC)
			{
				const int32 R = B.Row + d.X;
				const int32 C = B.Col + d.Y;
				const int32 Idx = R * Cols + C;
				if (CamOcc.IsValidIndex(Idx)) CamOcc[Idx] = true;
			}
		}
	}
}

void UDKInventoryWidget::MoveBlock(const FGuid& BlockId, int32 NewRow, int32 NewCol, int32 NewRot)
{
	// 1. 기존 블록을 찾아서 제거
	int32 FoundIndex = INDEX_NONE;
	FPlacedBlock FoundBlock;

	for (int32 i = 0; i < CamPlaced.Num(); ++i)
	{
		if (CamPlaced[i].Id == BlockId)
		{
			FoundIndex = i;
			FoundBlock = CamPlaced[i];
			CamPlaced.RemoveAt(i);
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Block not found. Cannot move."));
		return;
	}

	// 2. CamOcc 배열 업데이트 (기존 위치를 비워줌)
	// 이 로직은 RebuildFromPlaced()에서 한번에 처리하는 것이 효율적일 수 있습니다.

	// 3. 새 위치에 블록 정보 업데이트 및 추가
	FoundBlock.Row = NewRow;
	FoundBlock.Col = NewCol;
	FoundBlock.Rotation = NewRot;
	UE_LOG(LogTemp, Warning, TEXT("NewRot : %d"), FoundBlock.Rotation);
	CamPlaced.Add(FoundBlock);

	// 4. UI 및 데이터 업데이트
	// PlaceCamouflage_At(FoundBlock); // 이 함수를 재활용할 수도 있습니다.

	if (UGameInstance* GI = GetGameInstance())
	{
		auto* Digest = GI->GetSubsystem<UDigestiveSystem>();
		if (Digest)
		{
			Digest->SavedDishBlocks = CamPlaced;
		}
	}
	RecomputeCamOccFromPlaced();
	RebuildFromPlaced();
}

bool UDKInventoryWidget::CanMoveBlock(const FGuid& BlockId, int32 r0, int32 c0, const TArray<FIntPoint>& Cells) const
{
	// 1. 이동하려는 블록의 원본 위치와 셀을 찾습니다.
	const FPlacedBlock* OriginalBlock = nullptr;
	for (const FPlacedBlock& B : CamPlaced)
	{
		if (B.Id == BlockId)
		{
			OriginalBlock = &B;
			break;
		}
	}
	if (!OriginalBlock) return false;

	// 2. 그리드 경계 이탈 검사
	for (const FIntPoint& d : Cells)
	{
		const int32 R = r0 + d.X;
		const int32 C = c0 + d.Y;
		if (R < 0 || R >= Rows || C < 0 || C >= Cols) return false;
	}

	// 2) "자기 자리" 인덱스 사전 계산 (현재 회전/위치 기준)
	TSet<int32> SelfIdx;
	{
		// Inv 보장
		if (!Inv)
			if (UGameInstance* GI = GetGameInstance())
				const_cast<UDKInventoryWidget*>(this)->Inv = GI->GetSubsystem<UPlayerInventoryManager>();

		const FBlockShapeDef* Def = Inv ? Inv->GetBlock(OriginalBlock->ShapeId) : nullptr;

		const TArray<FIntPoint> RC = Def
			? const_cast<UDKInventoryWidget*>(this)->GetRotatedCells(*Def, OriginalBlock->Rotation)
			: TArray<FIntPoint>{};

		for (const FIntPoint& d : RC)
		{
			const int32 r = OriginalBlock->Row + d.X;
			const int32 c = OriginalBlock->Col + d.Y;
			if (r >= 0 && r < Rows && c >= 0 && c < Cols)
				SelfIdx.Add(r * Cols + c);
		}
	}

	// 3. 충돌 검사 (원본 블록의 위치는 무시)
	for (const FIntPoint& d : Cells)
	{
		const int32 R = r0 + d.X;
		const int32 C = c0 + d.Y;
		const int32 Idx = R * Cols + C;
		if (!CamOcc.IsValidIndex(Idx)) return false;

		// 자기 자리이면 무시 (회전 중 원래 칸 겹침 허용)
		if (SelfIdx.Contains(Idx))
			continue;

		// 다른 블록이 차지했는지 정확히 확인 (CamOcc만 보지 말고 CamPlaced로 판별)
		// → CamOcc가 자기 자리 때문에 true여도 여기서 걸러짐
		bool bOccupiedByOther = false;
		for (const FPlacedBlock& B : CamPlaced)
		{
			if (B.Id == OriginalBlock->Id) continue; // 자기 자신은 제외

			const FBlockShapeDef* Def = Inv ? Inv->GetBlock(B.ShapeId) : nullptr;
			if (!Def) continue;

			/*const TArray<FIntPoint> RC = (B.RotatedCells.Num() > 0)
				? B.RotatedCells
				: const_cast<UDKInventoryWidget*>(this)->GetRotatedCells(*Def, B.Rotation);*/

			const TArray<FIntPoint> RC = Def ? GetRotatedCells(*Def, B.Rotation) : TArray<FIntPoint>{};

			for (const FIntPoint& dd : RC)
			{
				if (B.Row + dd.X == R && B.Col + dd.Y == C)
				{
					bOccupiedByOther = true;
					break;
				}
			}
			if (bOccupiedByOther) break;
		}

		if (bOccupiedByOther)
			return false;
	}

	return true;
}

FPlacedBlock* UDKInventoryWidget::FindBlockAtLocation(int32 InRow, int32 InCol)
{
	// const를 제거하여 수정 가능한 객체에 대한 포인터를 얻습니다.
	for (int32 i = 0; i < CamPlaced.Num(); ++i)
	{
		// CamPlaced[i]는 이제 수정 가능합니다.
		FPlacedBlock& Block = CamPlaced[i];

		// 블록의 회전된 셀 좌표를 가져옵니다.
		const FBlockShapeDef* Def = Inv->GetBlock(Block.ShapeId);
		if (!Def) continue;

		TArray<FIntPoint> RotatedCells = GetRotatedCells(*Def, Block.Rotation);

		for (const FIntPoint& Cell : RotatedCells)
		{
			if (Block.Row + Cell.X == InRow && Block.Col + Cell.Y == InCol)
			{
				return &CamPlaced[i];
			}
		}
	}

	return nullptr;
}

void UDKInventoryWidget::CreateDragGhost(UTexture2D* GhostIcon, const TArray<FIntPoint>& CellsR)
{
	if (!Overlay_Blocks || DragGhostImg) return;

	// 0) 그리드/오버레이 지오메트리 확보
	auto GetReadyGridSize = [this]() -> FVector2D {
		FVector2D S(0, 0);
		if (Grid_Camouflage) S = Grid_Camouflage->GetCachedGeometry().GetLocalSize();
		if ((S.X < 2.f || S.Y < 2.f) && Overlay_Blocks) {
			// 오버레이가 먼저 잡히는 경우도 고려
			S = Overlay_Blocks->GetCachedGeometry().GetLocalSize();
		}
		return S;
		};

	FVector2D Grid = GetReadyGridSize();
	if (Grid.X < 2.f || Grid.Y < 2.f || Rows <= 0 || Cols <= 0)
	{
		// 아직 레이아웃이 안 잡혔으면 이 함수 안에서 1틱 지연 재시도
		FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateWeakLambda(this, [this, GhostIcon, CellsR](float)
				{
					CreateDragGhost(GhostIcon, CellsR);
					return false; // 한 번만
				}),
			0.f
		);
		UE_LOG(LogTemp, Verbose, TEXT("[Ghost] Grid not ready; deferred one tick."));
		return;
	}

	// 1) 회전된 셀 바운딩
	int32 minR = INT_MAX, minC = INT_MAX, maxR = INT_MIN, maxC = INT_MIN;
	for (const auto& P : CellsR) {
		minR = FMath::Min(minR, P.X);  minC = FMath::Min(minC, P.Y);
		maxR = FMath::Max(maxR, P.X);  maxC = FMath::Max(maxC, P.Y);
	}
	DragGhostMinR = minR;  DragGhostMinC = minC;
	DragGhostCellsR = CellsR;

	const float cellW = Grid.X / Cols;
	const float cellH = Grid.Y / Rows;
	const int32 rows = (maxR - minR + 1);
	const int32 cols = (maxC - minC + 1);
	DragGhostSize = FVector2D(cols * cellW, rows * cellH);
	UE_LOG(LogTemp, Warning, TEXT("[Ghost] grid=%s cell=(%.1f,%.1f) size=%s"),
		*Grid.ToString(), cellW, cellH, *DragGhostSize.ToString());

	// 2) 이미지 생성 + "브러시 ImageSize" 지정 (중요!)
	DragGhostImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	DragGhostImg->SetVisibility(ESlateVisibility::HitTestInvisible);

	FSlateBrush Br;
	Br.DrawAs = ESlateBrushDrawType::Image;
	Br.SetResourceObject(GhostIcon);
	Br.ImageSize = DragGhostSize;
	Br.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0.65f));
	DragGhostImg->SetBrush(Br);


	if (UOverlaySlot* S = Overlay_Blocks->AddChildToOverlay(DragGhostImg)) {
		S->SetHorizontalAlignment(HAlign_Left);
		S->SetVerticalAlignment(VAlign_Top);
	}
}

void UDKInventoryWidget::UpdateDragGhostPos(int32 r0, int32 c0)
{
	if (!DragGhostImg || !Grid_Camouflage) return;
	const FVector2D gridSize = Grid_Camouflage->GetCachedGeometry().GetLocalSize();
	const float cellW = gridSize.X / Cols;
	const float cellH = gridSize.Y / Rows;

	// r0,c0는 GrabCell을 고려한 앵커(우리가 Drop/Preview에 쓰는 그 좌표)
	const int32 baseR = r0 + DragGhostMinR;
	const int32 baseC = c0 + DragGhostMinC;

	DragGhostImg->SetRenderTranslation(FVector2D(baseC * cellW, baseR * cellH));
}

void UDKInventoryWidget::DestroyDragGhost()
{
	if (Overlay_Drag)
	{
		if (UWidget* W = DragGhost.Get())
		{
			Overlay_Drag->RemoveChild(W);
		}
	}
	DragGhost.Reset(); // nullptr 대용
}

void UDKInventoryWidget::SetBlockHidden(const FGuid& BlockId, bool bHidden)
{
	// 표시 상태만 토글
	if (bHidden) HiddenBlocksVisual.Add(BlockId);
	else         HiddenBlocksVisual.Remove(BlockId);

	// 1) 오버레이 이미지 숨김/복원
	if (UImage* Img = BlockOverlayMap.FindRef(BlockId).Get())
	{
		Img->SetVisibility(bHidden ? ESlateVisibility::Hidden
			: ESlateVisibility::HitTestInvisible);
	}

	// 2) 셀 텍스처 숨김/복원
	// CamPlaced에서 해당 블록 찾기
	const FPlacedBlock* PB = nullptr;
	for (const FPlacedBlock& B : CamPlaced) {
		if (B.Id == BlockId) { PB = &B; break; }
	}
	if (!PB || !Inv) return;

	if (const FBlockShapeDef* Def = Inv->GetBlock(PB->ShapeId))
	{
		const TArray<FIntPoint> Cells = GetRotatedCells(*Def, PB->Rotation);

		// 아이콘(복원 때 필요)
		Icon = nullptr;
		if (auto* DM = GetGameInstance()->GetSubsystem<UDataManager>()) {
			if (PB->DishId >= 0) Icon = DM->GetDishData(PB->DishId).ItemIcon;
		}

		for (const FIntPoint& P : Cells)
		{
			const int r = PB->Row + P.X, c = PB->Col + P.Y;
			if (UWidget* W = GetCell(r, c))
			{
				if (bHidden) {
					// 숨김: 칸을 비워 보이게(배경색/NoDraw)
					ApplyColorToSlotWidget(W, GridIdleColor);
				}
				else {
					// 복원: 아이콘이 있으면 다시 칠함(없으면 색)
					if (!Icon || !ApplyTextureToSlotWidget(W, Icon, GridIdleColor)) {
						// EffectiveBlockColor로 색 복원도 가능하지만 아이콘이 우선
						if (const FBlockShapeDef* D = Inv->GetBlock(PB->ShapeId))
							ApplyColorToSlotWidget(W, EffectiveBlockColor(D->Color));
					}
				}
			}
		}
	}
}

void UDKInventoryWidget::AttachDishTooltipToWidget(UWidget* Target, int32 DishId)
{
	if (!Target || !DishTooltipClass) return;

	if (Target->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
		Target->SetVisibility(ESlateVisibility::Visible);

	FDKDishData DishData;
	bool bFound = false;

	if (UGameInstance* GI = GetGameInstance())
		if (auto* DM = GI->GetSubsystem<UDataManager>())
		{
			DishData = DM->GetDishData(DishId);
			bFound = true;
		}

	if (!bFound) return;
	if (DishData.Description.IsEmpty() && DishData.DishEffect.IsEmpty()
		&& DishData.ItemName.IsNone())
		return;

	if (UDKDishTooltipWidget* Tip = CreateWidget<UDKDishTooltipWidget>(this, DishTooltipClass))
	{
		Tip->SetTooltipFromDishData(DishData); 
		Target->SetToolTip(Tip);
	}
}
