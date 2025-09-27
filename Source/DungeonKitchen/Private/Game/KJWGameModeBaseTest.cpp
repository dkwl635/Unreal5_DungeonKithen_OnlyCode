// Copyright © 2025 Tartare Studio


#include "Game/KJWGameModeBaseTest.h"

AKJWGameModeBaseTest::AKJWGameModeBaseTest()
{

}

void AKJWGameModeBaseTest::BeginPlay()
{
	Super::BeginPlay();

	if (!DKTimeManager)
	{
		DKTimeManager = NewObject<UDKTimeManager>(this);
		DKTimeManager->Init(this->GetWorld());
		AddDKManagerEvent();
		DKTimeManager->StartTime();

		
	}
}

UDKTimeManager* AKJWGameModeBaseTest::GetDKTimeManager_Implementation()
{
	return DKTimeManager;
}

void AKJWGameModeBaseTest::TestFunc()
{
}


void AKJWGameModeBaseTest::AddDKManagerEvent_Implementation()
{
}
