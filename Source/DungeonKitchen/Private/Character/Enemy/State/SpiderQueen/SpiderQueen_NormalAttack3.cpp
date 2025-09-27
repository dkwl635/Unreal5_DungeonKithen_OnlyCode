// Copyright © 2025 Tartare Studio


#include "Character/Enemy/State/SpiderQueen/SpiderQueen_NormalAttack3.h"

#include "Actor/Enemy/BossAttackActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USpiderQueen_NormalAttack3::StartState()
{
	Super::StartState();
	OwnerBoss->GetAnimInstance()->Montage_Play(PlayMontage);

	FVector TargetLocation = OwnerBoss->GetTarget()->GetActorLocation();
	FVector SpawnLocation = FindGroundLocationWithSweep(TargetLocation, OwnerBoss->GetWorld());

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(FQuat(FRotator(0,0,0)));
	ABossAttackActor* AttackActor =GetWorld()->SpawnActorDeferred<ABossAttackActor>(BossSpawnActorClass,SpawnTransform);
	if (AttackActor)
	{
		AttackActor->SetSetBoss(OwnerBoss);
		UGameplayStatics::FinishSpawningActor(AttackActor, SpawnTransform);
	}
}

void USpiderQueen_NormalAttack3::Update(float DeltaTime)
{
	Super::Update(DeltaTime);
}



void USpiderQueen_NormalAttack3::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PlayMontage && !bInterrupted)
	{
		NotifyPatternCompleted();
	}
}

float USpiderQueen_NormalAttack3::CalculatePatternWeight(EBossStateType PreviousPattern) const
{
	float weight = 10;

	if (OwnerBoss->AttackCount >= 3 || OwnerBoss->AttackCount < 2 )
		return 0.0f;

	if ((PreviousPattern == EBossStateType::NormalAttack1 || PreviousPattern == EBossStateType::NormalAttack2)
		&& OwnerBoss->AttackCount >= 2)

	{
		weight+=500;
	}

	if (OwnerBoss->GetTarget())
	{
		float TargetDis = OwnerBoss->GetTargetDistance();
		if (TargetDis <= 1000)
		{
			weight += 100;
		}
	}


	return weight;
}

void USpiderQueen_NormalAttack3::OnAnimNotifyBegin(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (!OwnerBoss->GetTarget())
		return;

	if (NotifyName == "Attack")
	{


	}
}
