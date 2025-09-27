// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/PlayerInventoryManager.h"

#include "Components/CanvasPanelSlot.h"
#include "Game/Subsystem/DataManager.h"
#include "UI/Widgets/DKFoodLogWidget.h"

void UPlayerInventoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Foods.Init(nullptr,MaxFoods);
	InitBlockShapes();

	FWorldDelegates::OnPostWorldCreation.AddUObject(this, &UPlayerInventoryManager::OnNewLevelLoaded);

	if (GEngine == nullptr)
	{
		return;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			CurrentWorld  = Context.World();
		}
	}

#if WITH_EDITOR
	// PIE 실행이 아니고, 그냥 에디터 뷰포트에서 편집 중일 때
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			CurrentWorld  = Context.World();
		}
	}
#endif

	FoodLogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
	FTickerDelegate::CreateWeakLambda(this, [this](float){ SpawnFoodLog(); return true; }),
	FoodLogDelay);
}

void UPlayerInventoryManager::Deinitialize()
{
	Super::Deinitialize();

	FWorldDelegates::OnPostWorldCreation.RemoveAll(this);

	if (FoodLogTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FoodLogTickerHandle);
		UE_LOG(LogTemp, Warning , TEXT("GetCoreTicker().RemoveTicker"));
	}
}

bool UPlayerInventoryManager::FirstAddFood(UDKFood* Food)
{
	int32 EmptySlotIndex = -1;
	for (int32 i = 0; i < MaxFoods; ++i)
	{
		if (Foods[i] == nullptr)
		{
			EmptySlotIndex = i;
			break;
		}
	}

	if (EmptySlotIndex == -1)
	{
		return  false;
	}

	FoodLogQueue.Enqueue(Food);

	return  AddFood(Food , EmptySlotIndex);
}


bool UPlayerInventoryManager::AddFood(UDKFood* Food, int32 SlotIndex)
{
	Foods[SlotIndex] = Food;
	UE_LOG(LogTemp, Warning , TEXT("PlayerInventoryManager::AddItem - Food %d") , SlotIndex);
	OnFoodEvent.Broadcast();
	OnPantryChanged.Broadcast(Foods);

	return  true;
}

UDKFood* UPlayerInventoryManager::RemoveFood(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxFoods)
	{
		return  Foods[SlotIndex];
	}

	return  nullptr;
}

UDKFood* UPlayerInventoryManager::GetFood(int32 SlotIndex)
{
	return  Foods[SlotIndex];
}

UDKFood* UPlayerInventoryManager::RemovePantryFoodByIndex(int32 Index)
{
	if (!Foods.IsValidIndex(Index)) return nullptr;

	UDKFood* FoodToRemove = Foods[Index];
	if (FoodToRemove)
	{
		Foods[Index] = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[INV] Remove/Move called."));
		OnPantryChanged.Broadcast(Foods);
		return FoodToRemove;
	}
	return nullptr;
}

void UPlayerInventoryManager::SwapFoodItems(TArray<TObjectPtr<UDKFood>>& TargetArray, int32 IndexA, int32 IndexB)
{
	// 두 인덱스가 유효한지 확인
	if (!TargetArray.IsValidIndex(IndexA) || !TargetArray.IsValidIndex(IndexB))
	{
		return;
	}

	// 아이템 교환 로직
	UDKFood* TempFood = TargetArray[IndexA];
	TargetArray[IndexA] = TargetArray[IndexB];
	TargetArray[IndexB] = TempFood;

	OnPantryChanged.Broadcast(Foods);
	OnCookingChanged.Broadcast(CookingFoods);
}

void UPlayerInventoryManager::InitBlockShapes()
{
	BlockShapes.Reset();

	FBlockShapeDef Shape0; // 한칸
	Shape0.ShapeId = 0;
	Shape0.Cells = { {0,0} };
	Shape0.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape0);

	FBlockShapeDef Shape1; // ㄱ
	Shape1.ShapeId = 1;
	Shape1.Cells = { {0,1},{1,0},{1,1} };
	Shape1.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape1);

	FBlockShapeDef Shape2; // 역 ㄱ
	Shape2.ShapeId = 2;
	Shape2.Cells = { {0,0},{1,0},{1,1} };
	Shape2.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape2);

	FBlockShapeDef Shape3; // 2X2 사각형
	Shape3.ShapeId = 3;
	Shape3.Cells = { {0,0},{0,1},{1,0}, {1,1} };
	Shape3.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape3);

	FBlockShapeDef Shape4; // 가로 2칸
	Shape4.ShapeId = 4;
	Shape4.Cells = { {0,0},{0,1} };
	Shape4.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape4);

	FBlockShapeDef Shape5; // 가로 3칸
	Shape5.ShapeId = 5;
	Shape5.Cells = { {0,0},{0,1},{0,2} };
	Shape5.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape5);

	FBlockShapeDef Shape6; // 가로 4칸
	Shape6.ShapeId = 6;
	Shape6.Cells = { {0,0},{0,1},{0,2}, {0,3} };
	Shape6.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape6);

	FBlockShapeDef Shape7; // 세로 2칸
	Shape7.ShapeId = 7;
	Shape7.Cells = { {0,0},{1,0} };
	Shape7.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape7);

	FBlockShapeDef Shape8; // 세로 3칸
	Shape8.ShapeId = 8;
	Shape8.Cells = { {0,0},{1,0},{2,0} };
	Shape8.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape8);

	FBlockShapeDef Shape9; // 세로 4칸
	Shape9.ShapeId = 9;
	Shape9.Cells = { {0,0},{1,0},{2,0},{3,0} };
	Shape9.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape9);

	FBlockShapeDef Shape10; // 가로2 세로3
	Shape10.ShapeId = 10;
	Shape10.Cells = { {0,0},{0,1},{0,2},{1,0},{1,1},{1,2} };
	Shape10.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape10);

	FBlockShapeDef Shape11; // 가로3 세로2
	Shape11.ShapeId = 11;
	Shape11.Cells = { {0,0},{0,1},{1,0},{1,1},{2,0},{2,1} };
	Shape11.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape11);

	FBlockShapeDef Shape12; // ㄴ
	Shape12.ShapeId = 12;
	Shape12.Cells = { {0,0},{1,0},{1,1} };
	Shape12.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape12);

	FBlockShapeDef Shape13; // 역 ㄴ
	Shape13.ShapeId = 13;
	Shape13.Cells = { {0,1},{1,0},{1,1} };
	Shape13.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape13);

	FBlockShapeDef Shape14; // 더하기 +
	Shape14.ShapeId = 14;
	Shape14.Cells = { {0,1},{1,0},{1,1},{1,2},{2,1} };
	Shape14.Color = FLinearColor::Gray;
	BlockShapes.Add(Shape14);
}

const FBlockShapeDef* UPlayerInventoryManager::GetBlock(int32 ShapeID) const
{
	// BlockShapes 배열을 순회하며 ShapeId가 일치하는 블록을 찾습니다.
	for (const FBlockShapeDef& Shape : BlockShapes)
	{
		if (Shape.ShapeId == ShapeID)
		{
			// 찾았으면, 해당 객체의 주소를 반환합니다.
			return &Shape;
		}
	}

	// 배열을 모두 순회했는데도 일치하는 블록이 없다면,
	// 오류 로그를 남기고 nullptr을 반환합니다.
	UE_LOG(LogTemp, Error, TEXT("GetBlock() failed: ShapeId %d not found!"), ShapeID);
	return nullptr;
}

void UPlayerInventoryManager::SpawnFoodLogUI(UDKFood* Food)
{
	if (!FoodLogWidgetClass)
		return;

	if (ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer())
	if (LP)
	{
		if (UDKFoodLogWidget* widget = 	CreateWidget<UDKFoodLogWidget>(LP->GetWorld(), FoodLogWidgetClass))
		{
			widget->AddToViewport();
			widget->SetItem(Food);
			// 오른쪽 아래 정렬
			widget->SetAlignmentInViewport(FVector2D(1.0f, 1.0f));

		}
	}

}


void UPlayerInventoryManager::OnNewLevelLoaded(UWorld* NewWorld)
{
	if (CurrentWorld != NewWorld)
	{
		CurrentWorld = NewWorld;
	}

}

void UPlayerInventoryManager::SpawnFoodLog()
{
	if (FoodLogQueue.IsEmpty())
		return;

	TWeakObjectPtr<UDKFood> Food;
	FoodLogQueue.Dequeue(Food);

	if (!Food.IsValid())
		return;

	SpawnFoodLogUI(Food.Get());
}
