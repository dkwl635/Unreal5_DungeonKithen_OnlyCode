// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DKInteractInterface.h"
#include "CauldronActor.generated.h"

UCLASS()
class DUNGEONKITCHEN_API ACauldronActor : public AActor, public IDKInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACauldronActor();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UBoxComponent> BoxComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	UFUNCTION()
	virtual void Interact() override;

	UFUNCTION(BlueprintCallable)
	void SetShow(bool bInShow);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsOpen = false;

private:
	UPROPERTY(EditDefaultsOnly)
	bool bShow = false;
};
