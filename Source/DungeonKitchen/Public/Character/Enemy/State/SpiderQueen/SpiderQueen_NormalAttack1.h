// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/State/BossStateBase.h"
#include "SpiderQueen_NormalAttack1.generated.h"

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API USpiderQueen_NormalAttack1 : public UBossStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABossAttackActor> BossSpawnActorNormal;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABossAttackActor> BossSpawnActorClass;

	virtual void StartState() override;
	virtual void Update(float DeltaTime) override;
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual float CalculatePatternWeight(EBossStateType PreviousPattern) const override;
	virtual void OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) override;

};
