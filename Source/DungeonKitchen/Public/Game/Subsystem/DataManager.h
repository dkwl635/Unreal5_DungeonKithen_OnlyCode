// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Data/EnemyDataStructures.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Data/ItemDataStructures.h"
#include "DataManager.generated.h"

/**
 * 아이템 관리 서브시스템
 * Food 데이터 테이블을 관리합니다.
 */
UCLASS(Blueprintable, BlueprintType ,Abstract)
class DUNGEONKITCHEN_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
public:
	//Food Item 생성
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	 UDKFood* GetFoodItem(const int32 FoodID, const int32 Stack) const;

	//Food Item 생성
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	 UDKDish* GetDishItem(const int32 DishID, const int32 Stack) const;

	//음식 데이터
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	FDKFoodData GetFoodData(const int32 FoodID) const;

	//요리 데이터
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	FDKDishData GetDishData(const int32 DishID) const;

	//레시피 데이터
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	FDKRecipeData GetRecipeData(const int32 RecipeID) const;
	//조합이 가능한 레시피들
	TArray<FDKRecipeData*> GetRecipesData(const TArray<int32>& IngredientIDs ) const;

	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	FDKRecipeData FindRecipeByIngredients(const TArray<int32>& IngredientIDs) const;

	//Enemy BP Class 리턴
	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	bool GetEnemyData(const int32 EnemyID , FDKEnemyData& InData);

	UFUNCTION(BlueprintCallable, Category = "Item Manager")
	bool GetWeaponData(const int32 WeaponID , FDKWeaponData& InData);



protected:
	// Food 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* FoodDataTable = nullptr;

	// Enemy 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* EnemyDataTable = nullptr;

	// Dish 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* DishDataTable = nullptr;

	// DishEffect 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* DishEffectTable = nullptr;

	// Recipe 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* RecipeDataTable = nullptr;

	// Weapon 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UDataTable* WeaponDataTable = nullptr;


private:
	FDKFoodData* GetFoodData_C(const int32 FoodID) const;
	FDKDishData* GetDishData_C(const int32 DishID) const;
	FDKDishEffectData* GetDishEffectData_C(const int32 EffectID) const;
	//레시피 케싱
	void InitializeRecipeCache() const;
	FString CreateIngredientKey(const TArray<int32>& IngredientIDs) const;
	mutable TMap<FString, FDKRecipeData*> RecipeCache;
	//재료가능한 레시피인지
	bool DoesRecipeMatch(const FDKRecipeData* Recipe,const TArray<int32>& InputIngredients) const;

};
