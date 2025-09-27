// Copyright © 2025 Tartare Studio


#include "Character/Enemy/State/SpiderQueen/SpiderQueen_NormalAttack1.h"

#include "Actor/Enemy/BossAttackActor.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USpiderQueen_NormalAttack1::StartState()
{
	Super::StartState();
	OwnerBoss->GetAnimInstance()->Montage_Play(PlayMontage);
}

void USpiderQueen_NormalAttack1::Update(float DeltaTime)
{
	Super::Update(DeltaTime);
}



void USpiderQueen_NormalAttack1::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PlayMontage && !bInterrupted)
	{
		NotifyPatternCompleted();
	}
}

float USpiderQueen_NormalAttack1::CalculatePatternWeight(EBossStateType PreviousPattern) const
{
	float weight = 10;
	if (PreviousPattern == EBossStateType::NormalAttack1
		|| OwnerBoss->AttackCount >= 3)
		return 0.0f;

	if (PreviousPattern == EBossStateType::NormalAttack2)
	{
		weight+=100;
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

void USpiderQueen_NormalAttack1::OnAnimNotifyBegin(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (!OwnerBoss->GetTarget())
		return;

	FVector BossLocation = OwnerBoss->GetMesh()->GetBoneLocation(TEXT("FrontLeg3_L"));
	FVector TargetLocation = OwnerBoss->GetTarget()->GetActorLocation();

	if (NotifyName == TEXT("Attack_1"))
	{
		if (BossSpawnActorNormal)
		{
			FTransform SpawnTransform;

			FVector SpawnLocation = FindGroundLocationWithSweep(BossLocation, OwnerBoss->GetWorld());
			SpawnTransform.SetLocation(SpawnLocation);

			SpawnTransform.SetRotation(FQuat(FRotator(0,0,0)));
			ABossAttackActor* AttackActor =GetWorld()->SpawnActorDeferred<ABossAttackActor>(BossSpawnActorNormal,SpawnTransform);
			if (AttackActor)
			{
				AttackActor->SetSetBoss(OwnerBoss);
				UGameplayStatics::FinishSpawningActor(AttackActor, SpawnTransform);
			}
		}
	}
	else if (NotifyName == TEXT("Attack_2"))
	{

		//추가 원거리 소환
		if (!BossSpawnActorClass)
			return;


		FVector Dir = TargetLocation - BossLocation;
		Dir.Normalize();

		FVector SpawnEffectLocation = BossLocation;

		int32 EffectCount = 4;
		for (int32 i = 0; i < EffectCount; i++)
		{
			SpawnEffectLocation += (Dir * 400.0f);
			FVector SpawnLocation = FindGroundLocationWithSweep(SpawnEffectLocation, OwnerBoss->GetWorld());

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

	}

}
