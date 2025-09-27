// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKPrepTrayWidget.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "UI/Widgets/DKPantrySlotWidget.h"
#include "UI/DKDragDropOperation.h"
#include "Components/Button.h"
#include "UI/Widgets/DKBlockPaletteWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Subsystem/CookingSystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"


void UDKPrepTrayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		Inv = GI->GetSubsystem<UPlayerInventoryManager>();

		CookingSystem = GI->GetSubsystem<UCookingSystem>();
		if (CookingSystem)
		{
			// RefreshFromCookingSystem -> 여기서는 슬롯 초기화
			CookingSystem->RefreshEvent.AddDynamic(this, &UDKPrepTrayWidget::RefreshFromCookingSystem);
		}
	}


	if (Button_Cooking)
	{
		Button_Cooking->OnClicked.AddDynamic(this, &UDKPrepTrayWidget::OnCookingButtonClicked);
	}

	BuildTraySlots(NumSlots);
	RefreshFromCookingSystem();
}

void UDKPrepTrayWidget::NativeDestruct()
{
	if (CookingSystem)
	{
		CookingSystem->RefreshEvent.RemoveDynamic(this, &UDKPrepTrayWidget::RefreshFromCookingSystem);
	}
	Super::NativeDestruct();
}

/**
 * @brief 다른 위젯에서 드래그된 아이템이 이 위젯에 드롭될 때 호출됩니다.
 *
 * 드롭된 아이템의 유효성을 검사하고, 인벤토리에서 트레이로 음식을 이동시키는 로직을 처리합니다.
 *
 * @param InGeo 드롭이 발생한 위젯의 지오메트리 정보.
 * @param InDragDropEvent 드래그 앤 드롭 이벤트 정보.
 * @param InOp 드롭된 아이템에 대한 정보.
 * @return 드롭 처리에 성공하면 true, 아니면 false를 반환합니다.
 */
bool UDKPrepTrayWidget::NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOp)
{
	if (!InOp || !CookingSystem) return false;

	if (CookingSystem && CookingSystem->bHasDishInPalette)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PrepTray] Drop blocked: dish exists in palette."));
		return false;
	}

	if (UDKDragDropOperation* DKOp = Cast<UDKDragDropOperation>(InOp))
	{
		// 드롭된 아이템이 팬트리에서 왔고, 음식이며, 인벤토리 매니저가 유효한지 확인
		if (DKOp->SourceInventory == EDKInventoryType::Pantry && DKOp->Food && Inv)
		{
			// CookingSystem의 슬롯 상태를 확인
			// 현재 CookingSystem에 비어 있는 슬롯 찾기
			int32 FreeSlotIndex = -1;
			for (int32 i = 0; i < 3; i++)
			{
				if (!CookingSystem->GetSlotFood(i))
				{
					FreeSlotIndex = i;
					break;
				}
			}

			// 빈 슬롯이 없으면 드롭 실패
			if (FreeSlotIndex == -1)
			{
				UE_LOG(LogTemp, Warning, TEXT("PrepTray is full"));
				return false;
			}

			// 인벤토리에서 음식을 제거하고, CookingSystem의 해당 슬롯에 추가
			UDKFood* FoodToMove = Inv->RemovePantryFoodByIndex(DKOp->SourceIndex);
			if (FoodToMove)
			{
				CookingSystem->AddSlotFood(FreeSlotIndex, FoodToMove);
				return true;
			}
		}
	}
	return false;
}

/**
 * @brief '요리하기' 버튼 클릭 시 호출됩니다.
 *
 * BlockPaletteWidget 포인터가 유효한지 확인하고,
 * 유효하면 팔레트 위젯의 RebuildPaletteFromManager() 함수를 호출하여
 * 인벤토리 팔레트를 갱신합니다.
 */
void UDKPrepTrayWidget::OnCookingButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Block"));
	if (!CookingSystem) return;

	// CookingSystem의 슬롯에 재료가 3개 있는지 확인
	int32 FilledSlots = 0;
	for (int32 i = 0; i < 3; i++)
	{
		if (CookingSystem->GetSlotFood(i))
		{
			FilledSlots++;
		}
	}
	// 재료가 3개가 아닐 경우 함수 종료
	if (FilledSlots < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("item < 3"));
		return;
	}

	/*UDKDish* Built = CookingSystem->MakeDish();
	if (!Built)
	{
		UE_LOG(LogTemp, Warning, TEXT("MakeDish failed"));
		return;
	}*/

	// 1) 이미 팔레트에 '미리보기 디시'가 있는 경우 → 새로 만들지 말고 '활성화'만
	if (CookingSystem->bHasDishInPalette && !CookingSystem->bDishInteractive)
	{
		CookingSystem->bDishInteractive = true; // 활성화
		// 팔레트가 상태를 다시 반영하도록 같은 디시를 재브로드캐스트
		if (UDKDish* Dish = CookingSystem->GetMakedDish())
		{
			CookingSystem->OnDishCreated.Broadcast(Dish);
		}
		return;
	}

	// 2) 디시가 없으면 '실제 상호작용'으로 생성
	CookingSystem->bDishInteractive = true;
	
	if (UDKDish* Built = CookingSystem->MakeDish())
	{
		UE_LOG(LogTemp, Log, TEXT("MakeDish (interactive) success."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MakeDish failed"));
	}

	//UDKDish* NewDish = CookingSystem->BringDish(); // SlotFoods = nullptr 로 초기화 + RefreshEvent 브로드캐스트
	//if (!NewDish)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("BringDish returned null"));
	//	return;
	//}
	//
	//UE_LOG(LogTemp, Warning, TEXT("Dish created: %s (ShapeId=%d)"),
	//	*NewDish->GetItemName().ToString(), NewDish->GetShapeId());
}

void UDKPrepTrayWidget::RefreshFromCookingSystem()
{
	if (!CookingSystem) return;

	//BuildTraySlots(NumSlots);

	// 슬롯 아이콘/가시성 갱신
	for (int32 i = 0; i < TraySlots.Num(); ++i)
	{
		UDKPantrySlotWidget* TraySlot = TraySlots[i].Get();
		if (!TraySlot) continue;

		TraySlot->SetupContext(EDKInventoryType::PrepTray, i);
		TraySlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (UDKFood* F = CookingSystem->GetSlotFood(i))
		{
			TraySlot->SetItem(F);
		}
		else
		{
			TraySlot->SetEmpty();
		}
	}

	// 버튼 활성/비활성은 한 번만 계산해서 반영
	int32 Filled = 0;
	for (int32 i = 0; i < 3; ++i)
	{
		if (CookingSystem->GetSlotFood(i)) ++Filled;
	}
	if (Button_Cooking)
	{
		Button_Cooking->SetIsEnabled(Filled == 3);
	}

	if (Filled == 3 && !CookingSystem->bHasDishInPalette)
	{
		CookingSystem->bDishInteractive = false;                // 미리보기 상태
		if (UDKDish* Built = CookingSystem->GetPendingDish())         // OnDishCreated 브로드캐스트됨
		{
			UE_LOG(LogTemp, Log, TEXT("[PrepTray] Auto GetPendingDish (preview mode)."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PrepTray] Auto GetPendingDish failed."));
		}
	}

	//if (Filled == 3 && !CookingSystem->bHasDishInPalette)
	//{
	//	CookingSystem->bDishInteractive = false;                // 미리보기 상태
	//	if (UDKDish* Built = CookingSystem->MakeDish())         // OnDishCreated 브로드캐스트됨
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("[PrepTray] Auto MakeDish (preview mode)."));
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("[PrepTray] Auto MakeDish failed."));
	//	}
	//}

	// ★ 프리뷰: 3칸 꽉 & 팔레트에 디시 없음일 때만 띄우기
	//if (BlockPaletteWidget)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("프리뷰 1"));
	//	const bool bHasDish = (CookingSystem && CookingSystem->bHasDishInPalette);
	//	if (Filled == 3 && !bHasDish)
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("프리뷰 2"));

	//		// 아이콘은 간단히 첫 슬롯 아이템 아이콘으로 (정확한 결과 모양은 버튼 이후 확정)
	//		UTexture2D* PreviewIcon = nullptr;
	//		for (int32 i = 0; i < 3; ++i)
	//			if (UDKFood* F = CookingSystem->GetSlotFood(i))
	//				if (UTexture2D* I = F->GetItemIcon()) { PreviewIcon = I; break; }

	//		if (PreviewIcon)
	//		{
	//			UE_LOG(LogTemp, Log, TEXT("프리뷰 3"));
	//			BlockPaletteWidget->ShowIconPreview(PreviewIcon);
	//		}
	//		else
	//		{
	//			UE_LOG(LogTemp, Log, TEXT("프리뷰 4"));
	//			BlockPaletteWidget->ClearIconPreview();
	//		}
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("프리뷰 5"));
	//		BlockPaletteWidget->ClearIconPreview();
	//	}
	//}

	//auto Apply = [&](UDKPantrySlotWidget* TraySlot, int32 Index)
	//	{
	//		if (!TraySlot) return;

	//		// 프렙 컨텍스트 보증 (어딘가에서 초기화 못 받았더라도 여기서 항상 잡아줌)
	//		TraySlot->SetupContext(EDKInventoryType::PrepTray, Index);

	//		// 드롭은 부모(PrepTrayWidget)가 받게 하고, 클릭은 슬롯이 처리하도록
	//		TraySlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	//		UDKFood* F = CookingSystem->GetSlotFood(Index);
	//		if (F)
	//		{
	//			TraySlot->SetItem(F);
	//			if (Button_Cooking) Button_Cooking->SetIsEnabled(true);
	//		}
	//		else
	//		{
	//			TraySlot->SetEmpty();
	//			if (Button_Cooking) Button_Cooking->SetIsEnabled(false);
	//		}
	//	};

	//Apply(Slot_0, 0);
	//Apply(Slot_1, 1);
	//Apply(Slot_2, 2);

	//// CookingSystem의 슬롯 내용을 가져와 UI 슬롯에 반영
	//if (Slot_0)
	//{
	//	Slot_0->SetItem(CookingSystem->GetSlotFood(0));
	//}
	//if (Slot_1)
	//{
	//	Slot_1->SetItem(CookingSystem->GetSlotFood(1));
	//}
	//if (Slot_2)
	//{
	//	Slot_2->SetItem(CookingSystem->GetSlotFood(2));
	//}
}

void UDKPrepTrayWidget::BuildTraySlots(int32 SlotCount)
{
	if (!HBox_Slots || !TraySlotClass) return;

	// 기존 자식 전부 제거
	HBox_Slots->ClearChildren();
	TraySlots.Reset();

	// 새 슬롯 생성 → HBox에 부착
	for (int32 i = 0; i < SlotCount; ++i)
	{
		UDKPantrySlotWidget* NewSlot = WidgetTree->ConstructWidget<UDKPantrySlotWidget>(TraySlotClass);
		if (!NewSlot) continue;

		// 프렙 컨텍스트 세팅(인벤타입/인덱스)
		NewSlot->SetupContext(EDKInventoryType::PrepTray, i);

		// 드롭은 부모가 받도록, 슬롯은 클릭만 받게 (필요 시)
		NewSlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		UHorizontalBoxSlot* Added = HBox_Slots->AddChildToHorizontalBox(NewSlot);
		if (Added)
		{
			// 간격/정렬 등 스타일(원하면)
			Added->SetPadding(FMargin(4.f));
			Added->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			Added->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		}

		TraySlots.Add(NewSlot);
	}
}
