// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CookingSystem.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCookingDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreviewShouldClear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDishCreatedDelegate, UDKDish*, Dish);

class UDKDish;
class UDKFood;


UCLASS()
class DUNGEONKITCHEN_API UCookingSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()



public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;



public:
	UFUNCTION(BlueprintCallable)
	UDKFood* AddSlotFood(int32 SlotIndex ,  UDKFood* Food);

	UFUNCTION(BlueprintCallable)
	UDKFood* RemoveSlotFood(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	UDKFood* GetSlotFood(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	UDKDish* MakeDish();

	UFUNCTION(BlueprintCallable)
	UDKDish* GetMakedDish();

	UFUNCTION(BlueprintCallable)
	UDKDish*  BringDish();

	UPROPERTY(BlueprintAssignable)
	FCookingDelegate RefreshEvent;

	// 요리 생성 시 알림
	UPROPERTY(BlueprintAssignable)
    FDishCreatedDelegate OnDishCreated;

	UPROPERTY(BlueprintAssignable)
    FDishCreatedDelegate OnDishPreveiwCreated;

	UPROPERTY(BlueprintAssignable)
	FOnPreviewShouldClear OnPreviewShouldClear;

	int32 GetPrepSlotCount() const;

	UPROPERTY(BlueprintReadOnly)
    bool bHasDishInPalette = false;

	UPROPERTY(BlueprintReadWrite)
	bool bDishInteractive = false;

	UFUNCTION(BlueprintCallable)
	UDKDish* GetPendingDish() const;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UDKFood>> SlotFoods;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDKDish* MakedDish;
};
