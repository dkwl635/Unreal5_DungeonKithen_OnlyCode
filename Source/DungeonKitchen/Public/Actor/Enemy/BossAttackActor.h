// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossAttackActor.generated.h"

UCLASS()
class DUNGEONKITCHEN_API ABossAttackActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABossAttackActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void SetSetBoss(class ABossBase* NewBoss);
	UFUNCTION(BlueprintCallable)
	void AttackTarget(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void OnAnimNotifyName(FName NotifyName);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class ABossBase> Boss;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UGameplayEffect> EffectClass;


};
