// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/CookingSystem.h"
#include "Data/ItemDataStructures.h"
#include "Game/Subsystem/DataManager.h"

void UCookingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SlotFoods.Init(nullptr, 3);
}

UDKFood* UCookingSystem::AddSlotFood(int32 SlotIndex, UDKFood* Food)
{
	UDKFood* Prev = SlotFoods[SlotIndex];

	SlotFoods[SlotIndex]= Food;

	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();
	TArray<int32> IngredientIDs;
	for (int32 i = 0; i < SlotFoods.Num(); i++)
	{
		if (SlotFoods[i] == nullptr)
			IngredientIDs.Add(999999);
		else
		{
			IngredientIDs.Add(SlotFoods[i]->GetItemID());
		}
	}
	int32 Num = DataManager->GetRecipesData( IngredientIDs ).Num();
	UE_LOG(LogTemp, Display, TEXT("Num = %d"), Num);


	RefreshEvent.Broadcast();
	//MakeDish();
	return  Prev;
}

UDKFood* UCookingSystem::RemoveSlotFood(int32 SlotIndex)
{
	UDKFood* Prev = SlotFoods[SlotIndex];

	SlotFoods[SlotIndex]= nullptr;

	RefreshEvent.Broadcast();
	OnPreviewShouldClear.Broadcast();
	//MakeDish();
	return  Prev;
}

UDKFood* UCookingSystem::GetSlotFood(int32 SlotIndex)
{
	return SlotFoods[SlotIndex];
}

UDKDish* UCookingSystem::MakeDish()
{
	TArray<int32> FoodIndexes;
	for (int32 i = 0; i < SlotFoods.Num(); i++)
	{
		if (SlotFoods[i])
		{
			FoodIndexes.Add(SlotFoods[i]->GetItemID());
		}
		else
		{
			FoodIndexes.Add(999999);
		}
	}
	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();
	FDKRecipeData RecipeData = DataManager->FindRecipeByIngredients(FoodIndexes);
	UDKDish* Dish = DataManager->GetDishItem(RecipeData.ResultDish, 1);

	MakedDish = Dish;
	if (MakedDish)
	{
		SlotFoods[0] = nullptr;
		SlotFoods[1] = nullptr;
		SlotFoods[2] = nullptr;
	}
	RefreshEvent.Broadcast();
	OnDishCreated.Broadcast(MakedDish);



	return Dish;
}

UDKDish* UCookingSystem::GetMakedDish()
{
	return MakedDish;
}

UDKDish* UCookingSystem::BringDish()
{
	UDKDish* Dish = MakedDish;
	if (!Dish)
	{
		return nullptr;
	}

	MakedDish = nullptr;



	RefreshEvent.Broadcast();
	return Dish;
}

int32 UCookingSystem::GetPrepSlotCount() const
{
	return SlotFoods.Num();
}

UDKDish* UCookingSystem::GetPendingDish() const
{
	TArray<int32> FoodIndexes;
	for (int32 i = 0; i < SlotFoods.Num(); i++)
	{
		if (SlotFoods[i])
		{
			FoodIndexes.Add(SlotFoods[i]->GetItemID());
		}
		else
		{
			FoodIndexes.Add(-1);
		}
	}
	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();
	FDKRecipeData RecipeData = DataManager->FindRecipeByIngredients(FoodIndexes);
	UDKDish* Dish = DataManager->GetDishItem(RecipeData.ResultDish, 1);
	// UTexture2D* Icon = Dish->GetItemIcon();

	OnDishPreveiwCreated.Broadcast(Dish);

	return Dish;
}
