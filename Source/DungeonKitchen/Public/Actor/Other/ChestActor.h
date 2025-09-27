// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DKInteractInterface.h"
#include "ChestActor.generated.h"

class ADropItemBox;
class AWeaponBase;

UCLASS()
class DUNGEONKITCHEN_API AChestActor : public AActor , public IDKInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChestActor();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UBoxComponent> BoxComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class USkeletalMeshComponent> MeshComp;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayOpenMontage();

	UFUNCTION()
	virtual void Interact() override;

	UFUNCTION(BlueprintCallable)
	void SetShow(bool bInShow);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsOpen = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<AWeaponBase>> WeaponList;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<int32> ItemIDList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	TObjectPtr<class UAnimMontage> OpenMontage;

private:
	UPROPERTY(EditDefaultsOnly)
	bool bShow = false;
};
