// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKBlockPaletteWidget.h"
#include "UI/Widgets/DKPantrySlotWidget.h"
#include "UI/DKBlockDragDropOp.h"
#include "Data/ItemDataStructures.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Game/Subsystem/CookingSystem.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/Border.h"

// ★ 슬롯 하나에 색 입히기 (Border -> Image -> 폴백)
static void TintOne(UWidget* W, const FLinearColor& Col)
{
	if (!W) return;

	if (UBorder* B = Cast<UBorder>(W)) { B->SetBrushColor(Col); return; }
	if (UImage* I = Cast<UImage>(W)) { I->SetColorAndOpacity(Col); return; }

	// 슬롯이 UserWidget일 수도 있으니 루트에 한번 더 시도
	if (UUserWidget* UW = Cast<UUserWidget>(W))
	{
		if (UWidget* RW = UW->GetRootWidget())
		{
			if (UBorder* RB = Cast<UBorder>(RW)) { RB->SetBrushColor(Col); return; }
			if (UImage* RI = Cast<UImage>(RW)) { RI->SetColorAndOpacity(Col); return; }
		}
		UW->SetColorAndOpacity(Col);
		return;
	}

	// 최후 폴백: 알파만
	W->SetRenderOpacity(Col.A);
}

void UDKBlockPaletteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	 EnsurePreviewLayer();  // 프리뷰 레이어 확보
	 PrewarmPreview();      // 더미로 한 번 레이아웃 고정


	if (UGameInstance* GI = GetGameInstance())
	{
		Inv = GI->GetSubsystem<UPlayerInventoryManager>();
		CookingSystem = GI->GetSubsystem<UCookingSystem>();
		
		if (CookingSystem)
		{
			CookingSystem->OnDishCreated.AddDynamic(this, &UDKBlockPaletteWidget::SetPaletteByDish);
			CookingSystem->OnDishPreveiwCreated.AddDynamic(this, &UDKBlockPaletteWidget::ShowIconPreview);
			CookingSystem->OnPreviewShouldClear.AddDynamic(this, &UDKBlockPaletteWidget::ClearIconPreview);


			// ★ 이미 만들어진 Dish가 있으면 즉시 세팅
			// MakedDish = CookingSystem->GetMakedDish();

			if (UDKDish* Dish = CookingSystem->GetMakedDish())
			{
				CookingSystem->bHasDishInPalette = true;
				SetPaletteByDish(Dish);
			}
			else
			{
				CookingSystem->bHasDishInPalette = false;
			}
			/*if (MakedDish)
			{
				SetPaletteByDish(MakedDish);
			}*/
		}
	}

	// 테스트용 ID 입력
	/*PaletteShapeIDs = {2};*/

	//RebuildPaletteFromManager();
	//Image->SetVisibility(ESlateVisibility::Hidden);
	
}

void UDKBlockPaletteWidget::NativeDestruct()
{
	if (CookingSystem) { CookingSystem->bHasDishInPalette = false; }

	if (Inv)
	{
		Inv = nullptr;
	}
	Super::NativeDestruct();
}

FReply UDKBlockPaletteWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	if (CookingSystem && !CookingSystem->bDishInteractive)
		return FReply::Handled();

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!HasBlockInPalette())
		{
			// 상세 원인 로그
			UDKDish* DishDbg = (CookingSystem ? CookingSystem->GetMakedDish() : nullptr);
			const FBlockShapeDef* DefDbg = (Inv && DishDbg) ? Inv->GetBlock(DishDbg->GetShapeId()) : nullptr;
			UE_LOG(LogTemp, Warning, TEXT("[Palette] no block (Inv=%d, Cooking=%d, Dish=%d, Def=%d, Cells=%d)"),
				Inv != nullptr, CookingSystem != nullptr, DishDbg != nullptr, DefDbg != nullptr,
				DefDbg ? DefDbg->Cells.Num() : -1);

			return FReply::Handled(); // 이벤트 소비하고 종료(버블업 방지)
		}

		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeo, InMouseEvent);
}

FReply UDKBlockPaletteWidget::NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("Button Down2"));
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeo, InMouseEvent);
}

void UDKBlockPaletteWidget::NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, class UDragDropOperation*& OutOperation)
{
	// if (bIsPreview) { OutOperation = nullptr; return; }

	if (CookingSystem && !CookingSystem->bDishInteractive) { OutOperation = nullptr; return; }
	UE_LOG(LogTemp, Warning, TEXT("[Palette] GI=%p Inv=%p Cook=%p"), GetGameInstance(), Inv, CookingSystem);
	CachedDish = CookingSystem ? CookingSystem->GetMakedDish() : nullptr;
	UE_LOG(LogTemp, Warning, TEXT("[Palette] Dish=%p"), CachedDish);
	int32 Sid = CachedDish ? CachedDish->GetShapeId() : -1;
	UE_LOG(LogTemp, Warning, TEXT("[Palette] ShapeId=%d"), Sid);
	if (Inv)
	{
		if (const FBlockShapeDef* Test = Inv->GetBlock(Sid))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Palette] Def OK, cells=%d"), Test->Cells.Num());
		}
			
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Palette] Inv->GetBlock(%d) returned NULL"), Sid);
		}
			
	}

	OutOperation = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("[Palette] NativeOnDragDetected ENTER"));
	// ★ 캐시 검증(팔레트 셋업이 끝난 상태여야 함)
	if (!CachedDef || CachedDef->Cells.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Palette] Drag fail: no cached block (shape=%d)"), CachedShapeId);
		return;
	}


	UDKBlockDragDropOp* Op = NewObject<UDKBlockDragDropOp>(this);
	if (!Op)
	{
		UE_LOG(LogTemp, Error, TEXT("[Palette] Failed to create DragDropOp"));
		return;
	}

	// 캐시 사용(불필요한 재조회 최소화)
	const FBlockShapeDef * Def = CachedDef;

	// 이 부분에서 Def가 유효한지, 그리고 Def->Cells에 데이터가 있는지 확인해야 합니다.
	if (!Def)
	{
		UE_LOG(LogTemp, Error, TEXT("[Palette] Failed: Def is NULL."));
		return;
	}
	if (Def->Cells.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Palette] Failed: Def->Cells is empty. ShapeId: %d"), CachedDish->GetShapeId());
		UE_LOG(LogTemp, Error, TEXT("[Palette] Failed: Def->Cells is empty. ShapeId: %d"), CachedShapeId);
		return;
	}

	Op->Dish = CachedDish;
	Op->InitFromShape(*Def, 0);
	Op->GrabCell = FIntPoint::ZeroValue; // GrabCell 값은 나중에 계산해야 합니다.
	Op->SourceInventory = EDKInventoryType::Cooking;

	// ▼ 4) 드래그 비주얼: GetDragVisualWidget() 실패해도 폴백을 “반드시” 넣기
	// UWidget* Visual = GetDragVisualWidget(DishIcon, Def);
	UWidget* Visual = GetDragVisualWidget(CachedDishIcon, Def);
	Op->DefaultDragVisual = Visual;
	Op->Pivot = EDragPivot::CenterCenter;

	// ▼ 5) 필수: OutOperation 세팅
	OutOperation = Op;

	// ▼ 6) 디버그
	UE_LOG(LogTemp, Warning, TEXT("[Palette] NativeOnDragDetected SET OutOperation"));
}

FReply UDKBlockPaletteWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
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

			}
			return FReply::Handled();
		}

	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UDKBlockPaletteWidget::SetPaletteByDish(UDKDish* Dish)
{
	ClearIconPreview();
	// ClearPaletteVisuals();

	/*const bool bInteractive = (CookingSystem && CookingSystem->bDishInteractive);
	SetRenderOpacity(bInteractive ? 1.f : 0.6f);
	SetVisibility(bInteractive ? ESlateVisibility::Visible
		: ESlateVisibility::HitTestInvisible);*/

	CachedDish = Dish;

	if (CookingSystem) { CookingSystem->bHasDishInPalette = (Dish != nullptr); }

	if (!Dish)
	{
		CachedShapeId = INDEX_NONE;
		CachedDishIcon = nullptr;
		CachedDef = nullptr;
		PaletteShapeIDs.Reset();
		// ★ 아이콘은 Grid_Palette의 '부모(=BoxOverlay)'에 Grid_Palette와 나란히 붙어 있음.
		//    → 형제들(아이콘 등)을 제거해 잔상 방지
		if (Grid_Palette)
		{
			if (UPanelWidget* Parent = Cast<UPanelWidget>(Grid_Palette->GetParent()))
			{
				for (int32 i = Parent->GetChildrenCount() - 1; i >= 0; --i)
				{
					UWidget* Child = Parent->GetChildAt(i);
					if (Child && Child != Grid_Palette)
					{
						Parent->RemoveChild(Child);   // 아이콘/BlockBox 오버레이 제거
					}
				}
			}

			Grid_Palette->ClearChildren(); // 블록 셀 비우기
		}

		// ★ 전체 팔레트 위젯 숨김(아이콘/테두리 등 모든 잔상 차단)
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Dish가 있으면 다시 보여주고 재구성
	SetVisibility(ESlateVisibility::Visible);
	CachedShapeId = Dish->GetShapeId();
	CachedDishIcon = Dish->GetItemIcon();
	CachedDef = (Inv ? Inv->GetBlock(CachedShapeId) : nullptr);

	PaletteShapeIDs.Reset();
	if (CachedShapeId != INDEX_NONE) PaletteShapeIDs.Add(CachedShapeId);


	RebuildPaletteFromManager(); // 아래에서 캐시만 사용하도록 수정
}

// 나중에 요리가 생성되면 브로드캐스트로 이 함수를 호출
void UDKBlockPaletteWidget::RebuildPaletteFromManager()
{
	if (!Grid_Palette || !Inv) return;
	Grid_Palette->ClearChildren();

	if (CachedShapeId == INDEX_NONE || !CachedDef) return;

	// UTexture2D* DishIcon = CachedDishIcon; // 로컬로 가져다 씀
	const FBlockShapeDef* Def = CachedDef; // ★ CookingSystem 조회 금지

	//DishIcon = CachedDish->GetItemIcon();
	// const FBlockShapeDef* Def = Inv->GetBlock(CachedDish->GetShapeId());
	if (!Def) return;

	// === 색상 결정: 팔레트에서 이 색을 사용 ===
	FLinearColor ShapeCol = Def->Color;
	if (ShapeCol.A <= 0.01f) ShapeCol.A = 0.85f; // 보이게 보정

	// 블록 bounding box (정규화용 최소/최대 동시 계산)
	int32 MinRow = INT_MAX, MinCol = INT_MAX;
	int32 MaxRow = 0, MaxCol = 0;
	for (const FIntPoint& C : Def->Cells)
	{
		MinRow = FMath::Min(MinRow, C.X);
		MinCol = FMath::Min(MinCol, C.Y);
		MaxRow = FMath::Max(MaxRow, C.X);
		MaxCol = FMath::Max(MaxCol, C.Y);
	}
	const int32 Rows = (MaxRow - MinRow + 1);
	const int32 Cols = (MaxCol - MinCol + 1);

	// 셀 픽셀 고정 크기
	constexpr float CellPx = 90.f;
	const FVector2D BlockSize(Cols * CellPx, Rows * CellPx);

	// ----- 1) Grid_Palette의 부모를 Overlay로 보장하고, 그 안을 재구성 -----
	UPanelWidget* Parent = Cast<UPanelWidget>(Grid_Palette->GetParent());
	if (!Parent) return;

	// 부모가 이미 Overlay인지 확인
	UOverlay* Layer = Cast<UOverlay>(Parent);
	int32 InsertIndex = INDEX_NONE;

	if (!Layer)
	{
		// 새 Overlay로 감싸기
		InsertIndex = Parent->GetChildIndex(Grid_Palette);
		Parent->RemoveChild(Grid_Palette);

		Layer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		Parent->InsertChildAt(InsertIndex, Layer);
	}
	else
	{
		// 이미 Overlay면 내부만 갈아끼움
		Layer->RemoveChild(Grid_Palette);
	}

	// ----- 2) 블록 전체 컨테이너(BlockBox)를 중앙 배치 -----
	USizeBox* BlockBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	BlockBox->SetWidthOverride(BlockSize.X);
	BlockBox->SetHeightOverride(BlockSize.Y);

	UOverlaySlot* BoxSlot = Layer->AddChildToOverlay(BlockBox);
	BoxSlot->SetHorizontalAlignment(HAlign_Center);
	BoxSlot->SetVerticalAlignment(VAlign_Center);

	// BlockBox 안에 아이콘 + 그리드를 포개기 위한 Overlay
	UOverlay* BoxOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	BlockBox->AddChild(BoxOverlay);

	// ----- 3) (아래층) Dish 아이콘: 비율 유지로 BlockBox에 맞춤 + 중앙정렬 -----
	if (CachedDishIcon)
	{
		UScaleBox* IconScale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
		IconScale->SetStretch(EStretch::ScaleToFit);
		IconScale->SetStretchDirection(EStretchDirection::Both);
		IconScale->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // ★ 입력 패스

		UImage* IconImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		IconImg->SetBrushFromTexture(CachedDishIcon, /*bMatchSize*/ true);
		IconImg->SetVisibility(ESlateVisibility::SelfHitTestInvisible);   // ★ 입력 패스

		IconScale->AddChild(IconImg);

		UOverlaySlot* IconSlot = BoxOverlay->AddChildToOverlay(IconScale);
		IconSlot->SetHorizontalAlignment(HAlign_Center);
		IconSlot->SetVerticalAlignment(VAlign_Center);
	}

	// ----- 4) (윗층) 라인 그리드: 셀을 90×90 SizeBox로 고정해서 퍼짐 방지 -----
	for (const FIntPoint& Cell : Def->Cells)
	{
		// 기본 라인 비주얼 (슬롯 위젯 또는 플레이스홀더)
		UWidget* Visual = nullptr;

		if (SlotClass)
		{
			if (UDKPantrySlotWidget* LineSlot = CreateWidget<UDKPantrySlotWidget>(this, SlotClass))
			{
				LineSlot->SetRenderOpacity(0.25f); 
				LineSlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // ★ 입력 패스
				Visual = LineSlot;
			}
		}
		if (!Visual)
		{
			UImage* Placeholder = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			FSlateBrush Br;
			Br.TintColor = FSlateColor(FLinearColor(0.0f, 0.6f, 1.0f, 0.35f));

			Placeholder->SetBrush(Br);
			Placeholder->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // ★ 입력 패스
			Visual = Placeholder;
		}

		// 셀 크기 고정
		USizeBox* CellBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		CellBox->SetWidthOverride(CellPx);
		CellBox->SetHeightOverride(CellPx);
		CellBox->AddChild(Visual);
		CellBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// ★ 좌상단(0,0) 기준으로 정규화해서 배치
		const int32 R = Cell.X - MinRow;
		const int32 C = Cell.Y - MinCol;
		Grid_Palette->AddChildToUniformGrid(CellBox, R, C);
	}

	// 그리드를 BlockBox 중앙에 얹기
	UOverlaySlot* GridSlot = BoxOverlay->AddChildToOverlay(Grid_Palette);
	GridSlot->SetHorizontalAlignment(HAlign_Center);
	GridSlot->SetVerticalAlignment(VAlign_Center);

	// ★ 팔레트 내부 컨테이너들도 모두 입력 패스 → 부모(이 위젯)로 이벤트가 올라옴
	Grid_Palette->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BoxOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BlockBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 루트는 입력을 받아야 하므로 Visible 유지
	SetVisibility(ESlateVisibility::Visible);

	CachedDef = Def;
}

bool UDKBlockPaletteWidget::HasBlockInPalette() const
{
	return (CachedDef && CachedDef->Cells.Num() > 0);
}

UWidget* UDKBlockPaletteWidget::GetDragVisualWidget(UTexture2D* Icon, const FBlockShapeDef* Def, float CellPx)
{
	// 도형 크기 계산(없으면 1x1)
	int32 Rows = 1, Cols = 1;
	int32 MinR = INT_MAX, MinC = INT_MAX, MaxR = 0, MaxC = 0;
	if (Def && Def->Cells.Num() > 0)
	{
		for (const FIntPoint& C : Def->Cells) {
			MinR = FMath::Min(MinR, C.X); MinC = FMath::Min(MinC, C.Y);
			MaxR = FMath::Max(MaxR, C.X); MaxC = FMath::Max(MaxC, C.Y);
		}
		Rows = FMath::Max(1, MaxR - MinR + 1);
		Cols = FMath::Max(1, MaxC - MinC + 1);
	}

	// 항상 크기 보장
	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Root->SetWidthOverride(Cols * CellPx);
	Root->SetHeightOverride(Rows * CellPx);
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// Overlay 위에 블록, 그 위에 아이콘 합성 ===
	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Overlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 1) 블록 격자 레이어 (있을 때만)
	if (Def && Def->Cells.Num() > 0)
	{
		UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());
		Grid->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		TSet<FIntPoint> S;
		for (const FIntPoint& P : Def->Cells) S.Add(FIntPoint(P.X - MinR, P.Y - MinC));

		for (int32 r = 0; r < Rows; ++r)
		{
			for (int32 c = 0; c < Cols; ++c)
			{
				USizeBox* Cell = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
				Cell->SetWidthOverride(CellPx);
				Cell->SetHeightOverride(CellPx);
				Cell->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

				UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				FSlateBrush Br;
				// 비어있는 칸은 완전 투명
				const bool bFilled = S.Contains(FIntPoint(r, c));
				//Br.TintColor = FSlateColor(bFilled ? Def->Color : FLinearColor(0, 0, 0, 0));
				Br.TintColor = FSlateColor(FLinearColor(0, 0, 0, 0));
				Br.DrawAs = ESlateBrushDrawType::Image;
				Br.ImageSize = FVector2D(CellPx, CellPx);
				Img->SetBrush(Br);
				Img->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

				Cell->AddChild(Img);
				Grid->AddChildToUniformGrid(Cell, r, c);
			}
		}

		// 외곽선(옵션) : 격자 전체를 감싸도록 얇은 테두리
		UBorder* Outline = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Outline->SetPadding(FMargin(1.f));
		Outline->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.0f)); // 살짝 어두운 테두리(머터리얼로 바꿔도 됨)
		Outline->SetContent(Grid);

		Overlay->AddChildToOverlay(Outline);
	}

	// 2) 아이콘 레이어 (있을 때만)
	if (Icon)
	{
		UScaleBox* Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
		Scale->SetStretch(EStretch::ScaleToFit);
		Scale->SetStretchDirection(EStretchDirection::Both);
		Scale->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Img->SetBrushFromTexture(Icon, /*bMatchSize*/ true);
		Img->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Img->SetRenderOpacity(0.95f); // 약간만 투명 → 회색 위에서도 또렷

		Scale->AddChild(Img);

		UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(Scale);
		IconSlot->SetHorizontalAlignment(HAlign_Center);
		IconSlot->SetVerticalAlignment(VAlign_Center);
	}

	// 3) 둘 다 없으면 폴백
	if (Overlay->GetChildrenCount() == 0)
	{
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Box->SetBrushColor(FLinearColor(0, 0, 0, 0.f));
		Box->SetPadding(FMargin(0));
		Box->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Root->AddChild(Box);
	}
	else
	{
		Root->AddChild(Overlay);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Palette] DragVisual Icon=%s  Cells=%d"),
		Icon ? *Icon->GetName() : TEXT("NULL"),
		Def ? Def->Cells.Num() : 0);

	return Root;
}

void UDKBlockPaletteWidget::NativeOnMouseEnter(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	bPaletteHovered = true;
	ApplyPaletteHover(true);
	Super::NativeOnMouseEnter(InGeo, InMouseEvent);

}

void UDKBlockPaletteWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	bPaletteHovered = false;
	ApplyPaletteHover(false);
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UDKBlockPaletteWidget::ApplyPaletteHover(bool bEnable)
{
	if (!Grid_Palette) return;
	if (CookingSystem && !CookingSystem->bDishInteractive)
		return;

	const FLinearColor Col = bEnable ? PaletteHoverTint : FLinearColor(0, 0, 0, 0);

	const int32 N = Grid_Palette->GetChildrenCount();
	for (int32 i = 0; i < N; ++i)
	{
		if (UWidget* Child = Grid_Palette->GetChildAt(i))
		{
			// 슬롯이 우리 슬롯 클래스면 슬롯 API를 쓰고
			if (auto* SlotW = Cast<UDKPantrySlotWidget>(Child))
			{
				// 슬롯에 이런 함수가 없으면 TintOne으로 바로 칠해도 OK
				// SlotW->ApplyHoverVisualWithColor(Col);
				TintOne(SlotW, Col);
			}
			else
			{
				// 일반 위젯(예: Overlay/Border/Image)이면 바로 칠함
				TintOne(Child, Col);
			}
		}
	}
}

void UDKBlockPaletteWidget::ShowIconPreview(UDKDish* Dish)
{
	UE_LOG(LogTemp, Log, TEXT("프리뷰!"));
	if(!Dish) return;

	UTexture2D* Icon;
	if (Dish->GetItemIcon())
	{
		Icon = Dish->GetItemIcon();
	}
	else
	{
		return;
	}

	SetRenderOpacity(1.f);

	//PreviewBox->SetVisibility(ESlateVisibility::Visible);

	// 프리뷰 모드 플래그
	// bIsPreview = true;

	EnsurePreviewLayer();      // PreviewBox/PreviewImg 확보
	// --- 크기 결정: 셀 기준 ---
	constexpr float CellPx = 90.f;
	int32 Rows = 3, Cols = 3;  // 기본값
	if (Inv)
	{
		if (const FBlockShapeDef* Def = Inv->GetBlock(Dish->GetShapeId()))
		{
			int32 minR = INT_MAX, minC = INT_MAX, maxR = 0, maxC = 0;
			for (const FIntPoint& C : Def->Cells) {
				minR = FMath::Min(minR, C.X); minC = FMath::Min(minC, C.Y);
				maxR = FMath::Max(maxR, C.X); maxC = FMath::Max(maxC, C.Y);
			}
			Rows = FMath::Max(1, maxR - minR + 1);
			Cols = FMath::Max(1, maxC - minC + 1);
		}
	}
	const FVector2D Size(Cols * CellPx, Rows * CellPx);
	PreviewBox->SetWidthOverride(Size.X);
	PreviewBox->SetHeightOverride(Size.Y);

	// --- 아이콘 브러시 세팅 (bMatchSize=false) ---
	if (Icon)
	{
		PreviewImg->SetBrushFromTexture(Icon, /*bMatchSize*/ false);
		PreviewImg->SetRenderOpacity(0.6f);
		PreviewImg->SetColorAndOpacity(FLinearColor(1, 1, 1, 0.6f));


	}

	// 보이되 입력은 막기 (프리뷰 상태)
	PreviewBox->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(1.f);
	SetVisibility(ESlateVisibility::HitTestInvisible);

	InvalidateLayoutAndVolatility();
}

void UDKBlockPaletteWidget::HandleDishPreviewCreated_Delayed(UDKDish* Dish)
{
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this, Dish](float)
			{
				ShowIconPreview(Dish);
				return false;
			}),
		0.f
	);
}

void UDKBlockPaletteWidget::EnsurePreviewLayer()
{
	if (PreviewBox) return;

	// Grid_Palette 부모를 Overlay로 보장
	UPanelWidget* Parent = Cast<UPanelWidget>(Grid_Palette ? Grid_Palette->GetParent() : nullptr);
	UOverlay* Layer = Parent ? Cast<UOverlay>(Parent) : nullptr;
	if (!Layer && Parent && Grid_Palette)
	{
		const int32 Idx = Parent->GetChildIndex(Grid_Palette);
		Parent->RemoveChild(Grid_Palette);
		Layer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		Parent->InsertChildAt(Idx, Layer);
		Layer->AddChild(Grid_Palette);
	}
	if (!Layer) return;

	// PreviewBox(SizeBox) + ScaleBox + Image
	PreviewBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	UScaleBox* Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
	Scale->SetStretch(EStretch::ScaleToFit);
	Scale->SetStretchDirection(EStretchDirection::Both);
	Scale->SetVisibility(ESlateVisibility::HitTestInvisible);

	PreviewImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	FSlateBrush Br; Br.DrawAs = ESlateBrushDrawType::Image; Br.ImageSize = FVector2D(512, 512);
	// Br.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0)); // 투명 더미
	PreviewImg->SetBrush(Br);
	PreviewImg->SetVisibility(ESlateVisibility::HitTestInvisible);

	Scale->AddChild(PreviewImg);
	PreviewBox->AddChild(Scale);

	if (UOverlaySlot* S = Layer->AddChildToOverlay(PreviewBox))
	{
		S->SetHorizontalAlignment(HAlign_Center);
		S->SetVerticalAlignment(VAlign_Center);
	}

	PreviewBox->SetVisibility(ESlateVisibility::Collapsed);
}

void UDKBlockPaletteWidget::PrewarmPreview()
{
	EnsurePreviewLayer();
	if (!PreviewBox) return;

	constexpr float CellPx = 90.f;
	// 적당한 기본 크기(예: 3x2). 실제 프리뷰 때는 Shape 크기로 다시 세팅됨.
	const FVector2D DefaultSize(3 * CellPx, 2 * CellPx);

	PreviewBox->SetWidthOverride(DefaultSize.X);
	PreviewBox->SetHeightOverride(DefaultSize.Y);

	// 입력은 막고, 화면엔 안 보이게
	PreviewBox->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(0.f);

	// 레이아웃 한 번 강제 통과
	InvalidateLayoutAndVolatility();
	//if (UWorld* W = GetWorld())
	//{
	//	// 다음 틱에 완전히 숨김으로 돌려놓기(입력/레이아웃만 준비)
	//	W->GetTimerManager().SetTimerForNextTick(
	//		FTimerDelegate::CreateUObject(this, &UDKBlockPaletteWidget::ClearIconPreview));
	//}
}

void UDKBlockPaletteWidget::ClearIconPreview()
{
	// bIsPreview = false;

	if (PreviewImg)
	{
		PreviewImg->SetRenderOpacity(0.f);
		PreviewImg->SetColorAndOpacity(FLinearColor(1, 1, 1, 0.f));
	}
	if (PreviewBox)
	{
		PreviewBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDKBlockPaletteWidget::ClearPaletteVisuals()
{
	if (!Grid_Palette) return;

	if (UPanelWidget* Parent = Cast<UPanelWidget>(Grid_Palette->GetParent()))
	{
		// Grid_Palette와 무관한 형제(아이콘, BlockBox, 오버레이 등) 제거
		for (int32 i = Parent->GetChildrenCount() - 1; i >= 0; --i)
		{
			UWidget* Child = Parent->GetChildAt(i);
			if (Child && Child != Grid_Palette)
				Parent->RemoveChild(Child);
		}
	}
	Grid_Palette->ClearChildren();
}
