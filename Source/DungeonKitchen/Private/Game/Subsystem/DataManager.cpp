// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/DataManager.h"

#include "Data/EnemyDataStructures.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeRecipeCache();
	UE_LOG(LogTemp, Warning, TEXT("Item Manager initialized : %s"), *GetName());
}

UDKFood* UDataManager::GetFoodItem(const int32 FoodID, const int32 Stack) const
{
	FDKFoodData* Data = GetFoodData_C(FoodID);

	if (!Data)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a FoodDataTable"));
		return nullptr;
	}

	//데이터 생성
	UDKFood* Food = NewObject<UDKFood>();
	Food->Init(Data, Stack);

	return  Food;
}



UDKDish* UDataManager::GetDishItem(const int32 DishID, const int32 Stack) const
{
	FDKDishData* Data = GetDishData_C(DishID);

	if (!Data)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a DishDataTable"));
		return nullptr;
	}

	//데이터 생성
	UDKDish* Dish = NewObject<UDKDish>();
	Dish->Init(Data, Stack);
	Dish->EffectData = GetDishEffectData_C(Data->DishEffectIndex);


	return  Dish;
}

FDKFoodData UDataManager::GetFoodData(const int32 FoodID) const
{
	FDKFoodData* Data =  GetFoodData_C(FoodID);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("No %d Food Data") , FoodID);
		return FDKFoodData();
	}

	return *Data;
}

FDKDishData UDataManager::GetDishData(const int32 DishID) const
{
	FDKDishData* Data =  GetDishData_C(DishID);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("No %d Dish Data") , DishID);
		return FDKDishData();
	}

	return *Data;
}

FDKRecipeData UDataManager::GetRecipeData(const int32 RecipeID) const
{
	// 1. 데이터 테이블 확인
	if (!RecipeDataTable)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a RecipeDataTable"));
		return FDKRecipeData();
	}

	// 2. ID를 문자열로 변환하여 행 이름으로 사용
	FString RowName = FString::FromInt(RecipeID);
	FDKRecipeData* Data = EnemyDataTable->FindRow<FDKRecipeData>(FName(*RowName), TEXT(""));
	if (!Data)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a RecipeDataTable"));
		return FDKRecipeData();
	}


	return *Data;
}

TArray<FDKRecipeData*> UDataManager::GetRecipesData(const TArray<int32>& IngredientIDs) const
{
	TArray<FDKRecipeData*> Result;
	TArray<int32> InputIngredients = IngredientIDs;
	InputIngredients.Sort();

	for (auto Data : RecipeCache)
	{
		FDKRecipeData* Recipe = Data.Value;
		if (Recipe)
		{
			if (DoesRecipeMatch(Recipe , InputIngredients))
			{
				Result.Add(Recipe);
			}
		}
	}

	return Result;
}

FDKRecipeData UDataManager::FindRecipeByIngredients(const TArray<int32>& IngredientIDs) const
{
	if (!RecipeDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data Manager doesn't have a RecipeDataTable"));
		return FDKRecipeData();
	}

	// 해시맵에서 빠른 검색
	FString Key = CreateIngredientKey(IngredientIDs);
	if (FDKRecipeData** FoundRecipe = RecipeCache.Find(Key))
	{
		return **FoundRecipe;
	}

	UE_LOG(LogTemp, Warning, TEXT("No recipe found for ingredients"));
	return FDKRecipeData();
}

bool UDataManager::GetEnemyData(const int32 EnemyID, FDKEnemyData& InData)
{
	// 1. 데이터 테이블 확인
	if (!EnemyDataTable)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a EnemyDataTable"));
		return false;
	}


	// 2. ID를 문자열로 변환하여 행 이름으로 사용
	FString RowName = FString::FromInt(EnemyID);
	FDKEnemyData* data = EnemyDataTable->FindRow<FDKEnemyData>(FName(*RowName), TEXT(""));
	if (!data)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a EnemyDataTable"));
		return false;
	}

	InData = *data;
	return true;
}

bool UDataManager::GetWeaponData(const int32 WeaponID, FDKWeaponData& InData)
{
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data Manager doesn't have a WeaponDataTable"));
		return false;
	}

	FString RowName = FString::FromInt(WeaponID);
	FDKWeaponData* data = WeaponDataTable->FindRow<FDKWeaponData>(FName(*RowName), TEXT(""));
	if (!data)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable doesn't have %d ID"), WeaponID);
		return false;
	}

	InData = *data;
	return true;
}

FDKFoodData* UDataManager::GetFoodData_C(const int32 FoodID) const
{
	if (!FoodDataTable)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a FoodDataTable"));
		return nullptr;
	}

	FString RowName = FString::FromInt(FoodID);
	return FoodDataTable->FindRow<FDKFoodData>(FName(*RowName), TEXT(""));;

}

FDKDishData* UDataManager::GetDishData_C(const int32 DishID) const
{
	if (!DishDataTable)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a DishDataTable"));
		return nullptr;
	}

	FString RowName = FString::FromInt(DishID);
	return DishDataTable->FindRow<FDKDishData>(FName(*RowName), TEXT(""));;
}

FDKDishEffectData* UDataManager::GetDishEffectData_C(const int32 EffectID) const
{
	if (!DishEffectTable)
	{
		UE_LOG(LogTemp, Warning , TEXT("Data Manager doesn't have a DishEffectTable"));
		return nullptr;
	}

	FString RowName = FString::FromInt(EffectID);
	return DishEffectTable->FindRow<FDKDishEffectData>(FName(*RowName), TEXT(""));;
}

void UDataManager::InitializeRecipeCache() const
{
	if (!RecipeDataTable)
		return;

	RecipeCache.Empty();

	TArray<FName> RowNames = RecipeDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FDKRecipeData* Data = RecipeDataTable->FindRow<FDKRecipeData>(RowName, TEXT(""));
		if (Data)
		{
			FString Key = CreateIngredientKey(Data->Ingredients);
			RecipeCache.Add(Key, Data);

		}
	}
}

FString UDataManager::CreateIngredientKey(const TArray<int32>& IngredientIDs) const
{
	TArray<int32> Sorted = IngredientIDs;
	Sorted.Sort(); // 순서 무시

	FString Key;
	for (int32 ID : Sorted)
	{
		Key += FString::FromInt(ID) + "_";
	}
	return Key;
}

bool UDataManager::DoesRecipeMatch(const FDKRecipeData* Recipe, const TArray<int32>& InputIngredients) const
{
	// 복사 후 정렬
	TArray<int32> RecipeIngs = Recipe->Ingredients;

	int32 count = 3;

	// 인덱스별로 매칭 확인
	for (int32 i = 0; i < 3; i++)
	{
		if (InputIngredients[i] == 999999) // 와일드카드
		{
			count--;
			continue;
		}

		if (RecipeIngs.Contains(InputIngredients[i]))
		{
			count--;
			RecipeIngs.RemoveSingle(InputIngredients[i]);
		}
	}

	return count <= 0;
}


