// Copyright © 2025 Tartare Studio


#include "Character/Enemy/State/SpiderQueen/SpiderQueen_SpecialAttack1.h"

#include "AIController.h"
#include "Actor/Enemy/BossAttackActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


void USpiderQueen_SpecialAttack1::StartState()
{
	Super::StartState();
	OwnerBoss->GetAIController()->SetFocus(OwnerBoss->GetTarget());
	OwnerBoss->GetAnimInstance()->Montage_Play(PlayMontage);
}

void USpiderQueen_SpecialAttack1::Update(float DeltaTime)
{
	Super::Update(DeltaTime);
}

void USpiderQueen_SpecialAttack1::NotifyPatternCompleted()
{
	Super::NotifyPatternCompleted();
}

void USpiderQueen_SpecialAttack1::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PlayMontage && !bInterrupted)
	{
		NotifyPatternCompleted();
	}
}

float USpiderQueen_SpecialAttack1::CalculatePatternWeight(EBossStateType PreviousPattern) const
{
	if (PreviousPattern == EBossStateType::SpecialAttack1)
		return 0.0f;

	if (OwnerBoss->AttackCount== 1)
		return 0.0f;

	float weight = 10;
	if (OwnerBoss->GetTarget())
	{
		float TargetDis = OwnerBoss->GetTargetDistance();
		if (TargetDis > 2000)
		{
			weight = 10000;
		}
		else if (TargetDis > 1800)
		{
			weight = 3000;
		}

	}

	return weight;
}

void USpiderQueen_SpecialAttack1::OnAnimNotifyBegin(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == TEXT("Attack"))
	{
		UCharacterMovementComponent* MoveComp =Cast<UCharacterMovementComponent>(OwnerBoss->GetTarget()->GetMovementComponent());

		FVector Dir = OwnerBoss->GetActorLocation() - OwnerBoss->GetTarget()->GetActorLocation();
		Dir.Normalize();

		float Power = 1000.0f;
		float TargetDis = OwnerBoss->GetTargetDistance();
		if (TargetDis >= 2000)
		{
			Power *= 1.5f ;
		}
		else if (TargetDis >= 2500)
		{
			Power *= 2.5f ;
		}
		else if (TargetDis < 1500)
		{
			Power  = 0.0f;
		}


		OwnerBoss->GetTarget()->LaunchCharacter(Dir * Power, true, true);
	}
	else if (NotifyName == TEXT("Spawn"))
	{
		FVector SpawnLocation = OwnerBoss->GetMesh()->GetBoneLocation(TEXT("Tail4_M"));

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		SpawnTransform.SetRotation(FQuat(FRotator(0,0,0)));
		AttackActor =GetWorld()->SpawnActorDeferred<ABossAttackActor>(BossSpawnActorClass,SpawnTransform);
		if (AttackActor.Get())
		{
			AttackActor->SetSetBoss(OwnerBoss);
			UGameplayStatics::FinishSpawningActor(AttackActor.Get(), SpawnTransform);

			// 스폰된 엑터를 소켓에 붙이기
			AttackActor->AttachToComponent(OwnerBoss->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("Tail4_MSocket"));
		}
	}
	else if (NotifyName == TEXT("Move"))
	{
		AttackActor->AttachToComponent(OwnerBoss->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("MiddleFinger_RSocket"));
	}
	else if (NotifyName == TEXT("Fire"))
	{
		AttackActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		AttackActor->OnAnimNotifyName(NotifyName);
	}

}


