// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ItemDataStructures.h"
#include "DigestiveSystem.generated.h"

/**
 *
 */

struct FGameplayAttribute;
struct FDKDishEffectData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDigestiveRefresh);

UCLASS(Blueprintable, BlueprintType ,Abstract)
class DUNGEONKITCHEN_API UDigestiveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<class UDKDish>> Dishes;

	UPROPERTY(BlueprintAssignable)
	FOnDigestiveRefresh OnRefresh;

	UPROPERTY()
    TArray<FPlacedBlock> SavedDishBlocks;

	UPROPERTY()
	bool bWipeOnNextLoad = false; // 다음 로드에서 인벤 비우기

public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerCharacter(AActor* Actor);
	UFUNCTION(BlueprintCallable)
	void AddDish(UDKDish* Dish);
	UFUNCTION(BlueprintCallable)
	void RemoveDish(UDKDish* Dish);


	UFUNCTION(BlueprintCallable)
	void StartTriggerDishes(FString Condition, class ADKCharacterBase* Target);


	UFUNCTION(BlueprintCallable)
    void PrepareForRestart();

protected:
	UPROPERTY()
	TWeakObjectPtr<class ADKPlayerCharacter> PlayerCharacter;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RateConditionTag;
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer KillCountConditionTag;
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer PlayerTargetTag;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ExplosionActorClass;
private:
	void FindCharacter();
	void AddCharacterEffect(UDKDish* Dish, class ADKCharacterBase* Target);
	void RemoveCharacterEffect(UDKDish* Dish,class ADKCharacterBase* Target);
	bool CheckTriggerDishes(UDKDish* Dish);
	bool CheckTargetDishEffect(UDKDish* Dish,ADKCharacterBase* Target) const;
	bool CheckSpicalDishEffect(UDKDish* Dish,ADKCharacterBase* Target) const;
private:
	TMap<FGameplayTag, TArray<UDKDish*>> TriggerDishes;
};
