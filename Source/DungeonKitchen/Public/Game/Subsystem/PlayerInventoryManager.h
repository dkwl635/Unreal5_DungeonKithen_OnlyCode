// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ItemDataStructures.h"
#include "PlayerInventoryManager.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFoodEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPantryChanged, const TArray<UDKFood*>&, Foods);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCookingChanged, const TArray<UDKFood*>&, Foods);

class UDKFood;

UCLASS(Blueprintable, BlueprintType ,Abstract)
class DUNGEONKITCHEN_API UPlayerInventoryManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnFoodEvent OnFoodEvent;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPantryChanged OnPantryChanged;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCookingChanged OnCookingChanged;

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 MaxFoods = 25;
	UPROPERTY(EditDefaultsOnly)
	float FoodLogDelay = 0.2f;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDKFoodLogWidget> FoodLogWidgetClass;
public:
	/*
	 * Aad Inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryManager")
	bool FirstAddFood(UDKFood* Food);

	bool AddFood(UDKFood* Food , int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryManager")
	UDKFood* RemoveFood(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryManager")
	UDKFood* GetFood(int32 SlotIndex);

	/**
	 * @brief 팬트리에서 특정 인덱스의 음식을 제거하고 반환합니다.
	 * @param Index 제거할 음식의 인덱스.
	 * @return 제거된 UDKFood 포인터, 또는 nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryManager")
	UDKFood* RemovePantryFoodByIndex(int32 Index);

	// TArray<UDKFood*>를 참조(&)로 받아서 두 요소를 교환하는 함수
	void SwapFoodItems(TArray<TObjectPtr<UDKFood>>& TargetArray, int32 IndexA, int32 IndexB);

	// 블록
	void InitBlockShapes();

	const FBlockShapeDef* GetBlock(int32 ShapeID) const;

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UDKFood>> Foods;
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UDKFood>> CookingFoods;
	UPROPERTY(EditDefaultsOnly)
	TArray<FBlockShapeDef> BlockShapes;
protected:
	//UPROPERTY(EditDefaultsOnly)
	//TSubclassOf<> FoodClass;
private:
	void SpawnFoodLogUI(UDKFood* Food);

	void OnNewLevelLoaded(UWorld* NewWorld);
	UFUNCTION()
	void SpawnFoodLog();


	TQueue<TWeakObjectPtr<UDKFood>> FoodLogQueue;
	UPROPERTY()
	UWorld* CurrentWorld = nullptr;
	FTimerHandle TimerHandleFoodLog;

	FTSTicker::FDelegateHandle FoodLogTickerHandle;
};

