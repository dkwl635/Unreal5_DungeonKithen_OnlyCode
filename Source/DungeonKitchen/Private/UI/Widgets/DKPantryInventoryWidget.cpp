// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKPantryInventoryWidget.h"
#include "Data/ItemDataStructures.h"
#include "UI/Widgets/DKPantrySlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridSlot.h"
#include "Components/UniformGridPanel.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "UI/DKDragDropOperation.h"

void UDKPantryInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Grid || !SlotClass) return;

	if (Columns <= 0)
	{
		Columns = 5;
	}

	// 디폴트 5 X 5 
	PreCreate();

	if (UGameInstance* GI = GetGameInstance())
	{
		Inv = GI->GetSubsystem<UPlayerInventoryManager>();
		if (Inv)
		{
			// 인벤토리 매니저에서 브로드캐스트 받아서 UI 갱신 이벤트 바인딩
			Inv->OnPantryChanged.RemoveDynamic(this, &UDKPantryInventoryWidget::RefreshFromPantry);
			Inv->OnPantryChanged.AddDynamic(this, &UDKPantryInventoryWidget::RefreshFromPantry);
			// 처음 열 때 현재 Foods 배열로 동기화
			RefreshFromPantry(Inv->Foods);
		}
	}
}

void UDKPantryInventoryWidget::NativeDestruct()
{
	// 위젯 제거될 때 호출
	if (Inv)
	{
		// 인벤토리 매니저 이벤트 바인딩 해제
		Inv->OnPantryChanged.RemoveDynamic(this, &UDKPantryInventoryWidget::RefreshFromPantry);
		Inv = nullptr;
	}
	Super::NativeDestruct();
}

UDKPantrySlotWidget* UDKPantryInventoryWidget::PreCreate()
{
	// 슬롯 디폴트 생성
	if (!Grid || !SlotClass) return nullptr;

	const int32 NeedSlots = DefaultRows * Columns;
	UDKPantrySlotWidget* SlotW = WidgetTree->ConstructWidget<UDKPantrySlotWidget>(SlotClass);
	UUniformGridSlot* GridSlot = Grid->AddChildToUniformGrid(SlotW);

	const int32 Row = NeedSlots / Columns;
	const int32 Col = NeedSlots % Columns;
	GridSlot->SetRow(Row);
	GridSlot->SetColumn(Col);

	SlotW->SetEmpty();
	Slots.Add(SlotW);
	return SlotW;
}

void UDKPantryInventoryWidget::RefreshFromPantry(const TArray<UDKFood*>& Foods)
{
	UE_LOG(LogTemp, Warning, TEXT("Pantry::Refresh Foods=%d"), Foods.Num());
	if (!Grid || !SlotClass) return;

	Grid->ClearChildren();

	// 1) 보이는 원본 인덱스만 모으기(빈칸은 스킵)
	TArray<int32> VisibleSrcIdx;
	VisibleSrcIdx.Reserve(Foods.Num());
	for (int32 i = 0; i < Foods.Num(); ++i)
		if (Foods[i]) VisibleSrcIdx.Add(i);

	const int32 NumVisible = VisibleSrcIdx.Num();

	// 2) 기본 행렬(예: 5x5) 유지
	const int32 NeedRows = DefaultRows;            // 고정
	const int32 NeedSlots = NeedRows * Columns;     // 예: 25

	int32 k = 0; // 집어넣은 '보이는' 아이템 수
	for (int32 idx = 0; idx < NeedSlots; ++idx)
	{
		UDKPantrySlotWidget* SlotW = WidgetTree->ConstructWidget<UDKPantrySlotWidget>(SlotClass);
		if (!SlotW) continue;

		if (auto* GS = Grid->AddChildToUniformGrid(SlotW))
		{
			GS->SetRow(idx / Columns);
			GS->SetColumn(idx % Columns);
		}

		SlotW->OnPantrySlotClicked.RemoveAll(this);
		SlotW->OnPantrySlotClicked.AddDynamic(this, &UDKPantryInventoryWidget::HandleSlotClicked);

		if (k < NumVisible)
		{
			const int32 src = VisibleSrcIdx[k++];      // 원본 배열의 실제 인덱스
			SlotW->SetupContext(EDKInventoryType::Pantry, src);
			SlotW->SetItem(Foods[src]);                // 아이템 채우기
			SlotW->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 남는 칸은 비우되, 기본 행렬은 유지
			SlotW->SetupContext(EDKInventoryType::Pantry, -1);
			SlotW->SetEmpty();
			SlotW->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UDKPantryInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
	// 슬롯 클릭되었을 때 호출되는 함수
	if (!Inv) return;
	UE_LOG(LogTemp, Warning, TEXT("Clicked Slot=%d  PantrySize=%d"), SlotIndex, Inv->Foods.Num());

	// 빈 칸 클릭이면 무시
	if (!Inv->Foods.IsValidIndex(SlotIndex) || !Inv->Foods[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid or Empty slot clicked"));
		return;
	}

}
