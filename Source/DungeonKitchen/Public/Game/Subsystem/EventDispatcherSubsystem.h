// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "EventDispatcherSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameplayTageEventDelegate);
DECLARE_DYNAMIC_DELEGATE(FGameplayTagCallback); 

UCLASS(Blueprintable, BlueprintType)
class DUNGEONKITCHEN_API UEventDispatcherSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual  void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual  void Deinitialize() override;
public:
    
	UFUNCTION(BlueprintCallable, Category = "DKGameplayTagEvent", meta=(WorlDKontext="WorlDKontextObject", DefaultToSelf="WorlDKontextObject"))
	static class UGameplayTagEventBinder* GetGameTagEventBinder(UObject* WorlDKontextObject, FGameplayTag EventTag);
	UFUNCTION(BlueprintCallable, Category = "DKGameplayTagEvent", meta=(WorlDKontext="WorlDKontextObject", DefaultToSelf="WorlDKontextObject"))
	static void OnGameplayTagEvent(UObject* WorlDKontextObject, FGameplayTag EventTag);
	
	UFUNCTION(BlueprintCallable)
	class UGameplayTagEventBinder* GetEventBinder(FGameplayTag EventTag);
	UFUNCTION(BlueprintCallable)
	void OnEventGameplayTag(FGameplayTag EventTag); 
	
private :
	TMap<FGameplayTag, TObjectPtr<UGameplayTagEventBinder>> EventMap;

	void ClearMap();
};
