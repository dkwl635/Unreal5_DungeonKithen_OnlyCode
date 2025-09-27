// Copyright © 2025 Tartare Studio

#include "Data/ItemDataStructures.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Character/DKCharacterBase.h"
#include "DishEffect/DishEffectBase.h"

void UDKItem::Init(FDKItemData* Data , int32 InStack)
{
	ItemTag = Data->ItemTag;
	ItemID = Data->ItemID;
	ItemName = Data->ItemName;
	Description = Data->Description;
	ItemIcon = Data->ItemIcon;

	Mesh = Data->ItemMesh;
	Stack = InStack;
}

int32 UDKItem::AddStack(int32 AddStack)
{
	int32 OldQuantity = Stack;
	int32 NewQuantity = OldQuantity + AddStack;

	if (NewQuantity > MaxStack)
	{
		// 최대 스택 초과
		Stack = MaxStack;
		int32 Overflow = NewQuantity - MaxStack;
		return Overflow;  // 오버된 수량 리턴
	}

	// 최대 스택 이하
	Stack = NewQuantity;
	return 0;  // 오버 없음

}

int32 UDKItem::RemoveStack(int32 RemoveStack)
{

	int NewQuantity = Stack - RemoveStack;

	if (NewQuantity <= 0 )
	{
		Stack = 0;
		return NewQuantity;
	}

	return 0;
}

void UDKFood::Init(FDKItemData* Data, int32  InStack)
{
	Super::Init(Data,InStack);

	// 음식 전용 속성 초기화
	if ( FDKFoodData* FoodData = static_cast<FDKFoodData*>(Data))
	{
		FoodRank = FoodData->FoodRank;
		ShapeId = FoodData->ShapeId;
	}
}

void UDKDish::Init(FDKItemData* Data, int32 InStack)
{
	Super::Init(Data, InStack);

	if (const FDKDishData* DishData = static_cast<FDKDishData*>(Data))
	{
		ShapeId = DishData->ShapeId;
	}

}

