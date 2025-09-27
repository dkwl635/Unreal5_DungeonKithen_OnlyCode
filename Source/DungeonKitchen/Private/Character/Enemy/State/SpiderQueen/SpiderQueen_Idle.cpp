// Copyright © 2025 Tartare Studio


#include "Character/Enemy/State/SpiderQueen/SpiderQueen_Idle.h"

#include "AIController.h"

void USpiderQueen_Idle::StartState()
{
	Super::StartState();
	OwnerBoss->GetAIController()->SetFocus(OwnerBoss->GetTarget());

	Timer = 0.0f;
}

void USpiderQueen_Idle::Update(float DeltaTime)
{
	Super::Update(DeltaTime);

	Timer += DeltaTime;
	if (Timer > IdleDelay)
	{
		NotifyPatternCompleted();
	}
}

float USpiderQueen_Idle::CalculatePatternWeight(EBossStateType PreviousPattern) const
{



	return 10.0f;
}
