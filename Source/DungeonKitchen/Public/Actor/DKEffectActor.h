// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DKEffectActor.generated.h"

class UGameplayEffect;

UCLASS()
class DUNGEONKITCHEN_API ADKEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADKEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;


};
