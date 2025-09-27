// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DKGameInstance.generated.h"

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API UDKGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDataManager> DataMangerClass;


	//메모리 포함을 위한
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UObject>> ManagerList;

};
