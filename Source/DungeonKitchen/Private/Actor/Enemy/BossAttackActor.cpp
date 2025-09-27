// Copyright © 2025 Tartare Studio


#include "Actor/Enemy/BossAttackActor.h"

#include "Character/Enemy/BossBase.h"

// Sets default values
ABossAttackActor::ABossAttackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABossAttackActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABossAttackActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABossAttackActor::SetSetBoss(class ABossBase* NewBoss)
{
	Boss = NewBoss;
}

void ABossAttackActor::AttackTarget(AActor* TargetActor)
{
	if (!Boss.IsValid() || !TargetActor)
		return;

	Boss->ApplyDamageToTarget(EffectClass, TargetActor);

}

void ABossAttackActor::OnAnimNotifyName_Implementation(FName NotifyName)
{
}

