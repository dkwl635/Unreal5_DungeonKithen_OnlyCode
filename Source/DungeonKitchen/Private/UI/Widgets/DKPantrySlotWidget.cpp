// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKPantrySlotWidget.h"
#include "Components/Image.h"
#include "Data/ItemDataStructures.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/DKDragDropOperation.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Subsystem/CookingSystem.h"
#include "Blueprint/WidgetTree.h"

// 생성자
UDKPantrySlotWidget::UDKPantrySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDKPantrySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;

	if (ClickButton)
	{
		ClickButton->OnClicked.RemoveAll(this);
		// ClickButton->OnClicked.AddDynamic(this, &UDKPantrySlotWidget::HandleClicked);


		//ClickButton->SetVisibility(ESlateVisibility::HitTestInvisible);
		ClickButton->SetIsEnabled(false);

		FButtonStyle S = ClickButton->WidgetStyle;

		// 1) 모든 상태를 Normal과 동일하게 (자동 hover/pressed 하이라이트 제거)
		S.Hovered = S.Normal;
		S.Pressed = S.Normal;
		S.Disabled = S.Normal;

		// 2) 아예 버튼 배경을 안 그리게 하고 싶다면(선택)
		//    -> 시각은 내부 Image/Overlay가 담당
		S.Normal.DrawAs = ESlateBrushDrawType::NoDrawType;
		S.Hovered.DrawAs = ESlateBrushDrawType::NoDrawType;
		S.Pressed.DrawAs = ESlateBrushDrawType::NoDrawType;
		S.Disabled.DrawAs = ESlateBrushDrawType::NoDrawType;

		ClickButton->SetStyle(S);

		// 3) 포커스 하이라이트도 막기
		//ClickButton->SetIsFocusable(false);
	}
}

FReply UDKPantrySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	// 마우스 왼쪽 버튼으로 드래그를 시작합니다.
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	UE_LOG(LogTemp, Warning, TEXT("클릭클릭"));
	return FReply::Unhandled();
}

void UDKPantrySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation)
{
	// 슬롯에 아이템이 없으면 드래그 불가능
	if (!FoodItem && !BlockId.IsValid())
	{
		OutOperation = nullptr;
		return;
	}

	// 드래그 앤 드롭 오퍼레이션 객체 생성
	UDKDragDropOperation* DragDropOp = NewObject<UDKDragDropOperation>(this);
	DragDropOp->Food = FoodItem;
	DragDropOp->SourceInventory = InventoryType; // 팬트리 인벤토리임을 지정
	DragDropOp->SourceIndex = SlotIndex; // 현재 슬롯의 인덱스 저장

	// 드래그 중인 위젯의 시각적 표현 생성
	DragDropOp->DefaultDragVisual = GetDragVisualWidget();
	DragDropOp->Pivot = EDragPivot::MouseDown;   // 또는 EDragPivot::CenterCenter

	OutOperation = DragDropOp;
}

bool UDKPantrySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	// 드롭된 아이템의 유효성 검사
	UDKDragDropOperation* DragDropOp = Cast<UDKDragDropOperation>(InOp);
	if (!DragDropOp || !DragDropOp->Food) return false;

	auto* Inv = GetGameInstance()->GetSubsystem<UPlayerInventoryManager>();
	auto* CS = GetGameInstance()->GetSubsystem<UCookingSystem>();
	if (!Inv || !CS) return false;

	

	const int32 To = SlotIndex;

	// ===== 타겟이 팬트리 슬롯 =====
	if (InventoryType == EDKInventoryType::Pantry)
	{
		if (DragDropOp->SourceInventory == EDKInventoryType::Pantry)
		{
			// 팬트리 → 팬트리 : 스왑 + 브로드캐스트
			// 유효 인덱스인지 검사
			if (!Inv->Foods.IsValidIndex(DragDropOp->SourceIndex) || !Inv->Foods.IsValidIndex(To))
			{
				return false;
			}

			// 둘 중 하나라도 빈칸이면 스왑 안 함
			if (Inv->Foods[DragDropOp->SourceIndex] == nullptr || Inv->Foods[To] == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Swap skipped: empty slot (Src=%d, To=%d)"),
					DragDropOp->SourceIndex, To);
				return false;
			}

			Inv->SwapFoodItems(Inv->Foods, DragDropOp->SourceIndex, To);
			Inv->OnPantryChanged.Broadcast(Inv->Foods);
			return true;
		}

		if (DragDropOp->SourceInventory == EDKInventoryType::PrepTray)
		{
			// 1) 트레이에서 "미리보기"만 (성공 전까지 Remove 금지)
			UDKFood* Moving = CS->GetSlotFood(DragDropOp->SourceIndex);
			if (!Moving) Moving = DragDropOp->Food;
			if (!Moving) return false;

			// 2) 목적 인덱스 계산: 범위 밖이면 첫 빈칸 탐색
			auto FindFirstEmpty = [&]() -> int32
				{
					for (int32 i = 0; i < Inv->Foods.Num(); ++i)
					{
						if (Inv->Foods[i] == nullptr) return i;
					}
					return INDEX_NONE;
				};

			int32 TargetIndex = FindFirstEmpty();

			if (!Inv->Foods.IsValidIndex(TargetIndex))
			{
				const int32 Empty = FindFirstEmpty();
				if (Empty == INDEX_NONE) return false; // 빈칸 없음 → 실패
				TargetIndex = Empty;
			}

			// 3) "빈칸에만" 꽂기: 차있으면 실패(스왑 불가 정책)
			if (Inv->Foods[TargetIndex] != nullptr)
			{
				return false; // 드랍 실패 → 원본은 손대지 않았으니 자동 복귀
			}

			// 4) 성공 확정 시에만 실제 제거 + 추가
			CS->RemoveSlotFood(DragDropOp->SourceIndex); // 트레이에서 제거(내부에서 Refresh/MakeDish 발생)
			Inv->AddFood(Moving, TargetIndex);           // 팬트리에 꽂기(내부에서 OnPantryChanged 발생)
			return true;
		}

		return false;
		//	if (Inv->Foods.IsValidIndex(To) && Inv->Foods[To] == nullptr)
		//	{
		//		int32 FilledCount = 0;
		//		for (UDKFood* Food : Inv->Foods)
		//		{
		//			if (Food) FilledCount++;
		//		}

		//		Inv->AddFood(moved, FilledCount); // 내부에서 OnPantryChanged.Broadcast 호출됨
		//		return true;
		//	}
		//	// (옵션) 차있으면 교체/스왑 로직 넣고 싶으면 여기에 추가
		//	return false;
		//}
		//return false;
	}

	// ===== 타겟이 프렙 슬롯 =====
	if (InventoryType == EDKInventoryType::PrepTray)
	{
		if (CS->bHasDishInPalette) return false;
		if (DragDropOp->SourceInventory == EDKInventoryType::PrepTray)
		{
			// 프렙 ↔ 프렙 스왑
			UDKFood* moved = CS->RemoveSlotFood(DragDropOp->SourceIndex);
			UDKFood* prev = CS->AddSlotFood(To, moved);
			if (prev) CS->AddSlotFood(DragDropOp->SourceIndex, prev);
			return true;
		}

		if (DragDropOp->SourceInventory == EDKInventoryType::Pantry)
		{
			// 0) 이동 후보 확보 (페이로드 우선)
			UDKFood* Candidate = DragDropOp->Food;
			if (!Candidate)
			{
				if (!Inv || !Inv->Foods.IsValidIndex(DragDropOp->SourceIndex)) return false;
				Candidate = Inv->Foods[DragDropOp->SourceIndex];
			}
			if (!Candidate) return false;

			const bool bTargetEmpty = (CS->GetSlotFood(To) == nullptr);

			// ---- 스왑/삽입을 안전하게 되돌릴 헬퍼 ----
			auto RestoreToPantry = [&](UDKFood* Food)
				{
					if (!Food || !Inv) return;

					// 1) 원래 인덱스가 비어있으면 거기에
					if (Inv->Foods.IsValidIndex(DragDropOp->SourceIndex) && Inv->Foods[DragDropOp->SourceIndex] == nullptr)
					{
						Inv->Foods[DragDropOp->SourceIndex] = Food;
						Inv->OnPantryChanged.Broadcast(Inv->Foods);
						return;
					}

					// 2) 첫 빈칸
					int32 Empty = INDEX_NONE;
					for (int32 i = 0; i < Inv->Foods.Num(); ++i)
					{
						if (Inv->Foods[i] == nullptr) { Empty = i; break; }
					}
					if (Empty != INDEX_NONE)
					{
						Inv->Foods[Empty] = Food;
						Inv->OnPantryChanged.Broadcast(Inv->Foods);
						return;
					}

					// 3) 그래도 못 넣으면 맨 뒤에 추가
					Inv->Foods.Add(Food);
					Inv->OnPantryChanged.Broadcast(Inv->Foods);
				};

			// ---- 타겟이 비었으면: 일반 이동 ----
			if (bTargetEmpty)
			{
				UDKFood* Moved = Inv->RemovePantryFoodByIndex(DragDropOp->SourceIndex);
				if (!Moved) Moved = Candidate;
				if (!Moved) return false;

				CS->AddSlotFood(To, Moved);
				return true;
			}

			// ---- 타겟이 차있으면: 스왑 ----
			{
				// 팬트리에서 실제 제거
				UDKFood* Moved = Inv->RemovePantryFoodByIndex(DragDropOp->SourceIndex);
				if (!Moved) Moved = Candidate;
				if (!Moved) return false;

				// 프렙에 넣으면서 이전 아이템(prev)을 받음
				UDKFood* Prev = CS->AddSlotFood(To, Moved);

				// 기존 프렙 아이템(prev)을 팬트리로 되돌림(원래 자리 → 빈칸 → 끝에 추가)
				RestoreToPantry(Prev);
				return true;
			}
		}
	}
	return false;
}

UWidget* UDKPantrySlotWidget::GetDragVisualWidget()
{
	if (!FoodItem) return nullptr;
	
	// 1) 아이콘 이미지 위젯 즉석 생성
	UImage* DragImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	if (!DragImage) return nullptr;

	if (UTexture2D* Icon = FoodItem->GetItemIcon())
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Icon);
		Brush.ImageSize = FVector2D(80.f, 80.f); 
		Brush.DrawAs = ESlateBrushDrawType::Image;

		DragImage->SetBrush(Brush);
	}

	DragImage->SetRenderOpacity(0.65f); // 반투명
	DragImage->SetVisibility(ESlateVisibility::HitTestInvisible); // 드롭 방해 X
	return DragImage;
}

void UDKPantrySlotWidget::HandleClicked()
{
	OnPantrySlotClicked.Broadcast(SlotIndex);
}

void UDKPantrySlotWidget::SetupContext(EDKInventoryType InType, int32 InIndex)
{
	InventoryType = InType;
	SlotIndex = InIndex;
}

void UDKPantrySlotWidget::SetItem(UDKFood* Item)
{
	if (!IconImage || !IsValid(Item)) { SetEmpty(); return; }

	if (UTexture2D* Icon = Item->GetItemIcon())
	{
		/*IconImage->SetBrushFromTexture(Icon);
		FoodItem = Item;
		SetVisibility(ESlateVisibility::Visible);*/

		FSlateBrush Brush;
		Brush.SetResourceObject(Icon);
		IconImage->SetBrush(Brush);

		FoodItem = Item;
		SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Warning, TEXT("Pantry Slot Item Set : %s"), *GetNameSafe(Icon));
	}
	else
	{
		SetEmpty();
		UE_LOG(LogTemp, Warning, TEXT("Pantry Slot Item is null"));
	}
}

void UDKPantrySlotWidget::SetEmpty()
{
	if (IconImage)
	{
		//IconImage->SetBrushFromTexture(nullptr);
		/*FSlateBrush EmptyBrush;
		EmptyBrush.SetResourceObject(nullptr);
		IconImage->SetBrush(EmptyBrush);*/

		// 슬롯 이미지 하얀색으로 바뀌는 오류 해결 코드
		FSlateBrush Empty;
		Empty.DrawAs = ESlateBrushDrawType::NoDrawType;
		IconImage->SetBrush(Empty);
	}
	FoodItem = nullptr;
	SetVisibility(ESlateVisibility::Visible);

	// 빈칸은 버튼 클릭만 막고 드롭은 받게 하고 싶다면
	if (ClickButton) { ClickButton->SetIsEnabled(false); }
}
