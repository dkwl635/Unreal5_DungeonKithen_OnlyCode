// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Game/Time/DKTime.h"
#include "DKTimeManager.generated.h" 
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdatedSignature, const FDKTime&, GameTime);


UCLASS(Blueprintable)
class DUNGEONKITCHEN_API UDKTimeManager : public UObject
{
	GENERATED_BODY()

	UDKTimeManager();
public:
	UFUNCTION(BlueprintCallable)
	void Init(UWorld* InWorld);
	UFUNCTION(BlueprintCallable)
	void SetHourMinute(int32 InHour, int32 InMinute);
	UFUNCTION(BlueprintCallable)
	void SetSeasonDay(int32 InSeason, int32 InDay);
	UFUNCTION(BlueprintCallable)
	void StartTime();
	UFUNCTION(BlueprintCallable)
	void PauseTime();
	UFUNCTION(BlueprintCallable)
	void ResumeTime();
	UFUNCTION(BlueprintCallable)
	void TickTime();
	UFUNCTION(BlueprintCallable)
	void ClearTime();
	UFUNCTION(BlueprintCallable)
	void NextDay();
	UFUNCTION(BlueprintCallable)
	const FDKTime& GetDKTime() const {return DKGameTime;}
	
	UPROPERTY(BlueprintAssignable, Category="Time")
	FOnTimeUpdatedSignature OnTimeUpdated;
	UPROPERTY(BlueprintAssignable, Category="Time")
	FOnTimeUpdatedSignature OnDayUpdated;
	
private:
	UPROPERTY()
	FDKTime DKGameTime;
	FTimerHandle TickHandle;
	bool bPaused = false;
	UPROPERTY()
	TWeakObjectPtr<UWorld> WorldRef;


};
