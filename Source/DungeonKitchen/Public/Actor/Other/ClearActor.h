// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClearActor.generated.h"

UCLASS()
class DUNGEONKITCHEN_API AClearActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AClearActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable )
	void RoomClear();

	virtual void RoomClear_Implementation();
};
