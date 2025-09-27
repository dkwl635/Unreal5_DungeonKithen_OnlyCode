// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/DKGameTimeInterface.h"
#include "KJWGameModeBaseTest.generated.h"

/**
 * 
 */

UCLASS()
class DUNGEONKITCHEN_API AKJWGameModeBaseTest : public AGameModeBase , public IDKGameTimeInterface
{
	GENERATED_BODY()

	AKJWGameModeBaseTest();
private:
	virtual void BeginPlay() override;

public:

	virtual UDKTimeManager* GetDKTimeManager_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddDKManagerEvent();
	UFUNCTION(BlueprintCallable)
	void TestFunc();
	
private:
	TObjectPtr<UDKTimeManager> DKTimeManager;
	
};
