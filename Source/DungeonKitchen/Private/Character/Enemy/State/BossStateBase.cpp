// Copyright © 2025 Tartare Studio


#include "Character/Enemy/State/BossStateBase.h"

UBossStateBase::UBossStateBase()
{
}

void UBossStateBase::SetOwner(ABossBase* NewOwner)
{
	OwnerBoss = NewOwner;
}

void UBossStateBase::InitState()
{
}

void UBossStateBase::StartState()
{
}

void UBossStateBase::Update(float DeltaTime)
{
}



void UBossStateBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
}

void UBossStateBase::NotifyPatternCompleted()
{
	if (OwnerBoss)
	{
		OwnerBoss->OnPatternCompleted(OwnerBoss->CurrentState);
	}
}

float UBossStateBase::CalculatePatternWeight(EBossStateType PreviousPattern) const
{
	return  0.0f;
}

void UBossStateBase::OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{

}

FVector UBossStateBase::FindGroundLocationWithSweep(const FVector& StartLocation, UWorld* InWorld)
{
	FVector TraceStart = StartLocation + FVector(0, 0, 1000.0f); // 위에서 시작
	FVector TraceEnd = StartLocation - FVector(0, 0, 1000.0f);   // 아래로 끝

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.AddIgnoredActor(OwnerBoss);
	QueryParams.AddIgnoredActor(OwnerBoss->GetTarget());

	bool bHit = InWorld->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_WorldStatic, // 땅은 보통 WorldStatic 채널
		QueryParams
	);

	if (bHit)
	{
		return HitResult.Location;
	}

	return StartLocation; // 실패시 원래 위치 반환
}

