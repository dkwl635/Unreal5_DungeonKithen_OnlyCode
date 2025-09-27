// Copyright © 2025 Tartare Studio
#include "Game/Time/DKTimeManager.h"


UDKTimeManager::UDKTimeManager()
{
}

void UDKTimeManager::Init(UWorld* InWorld)
{
	check(InWorld);

	OnTimeUpdated.Clear();
	OnDayUpdated.Clear();
	
	DKGameTime.Hour = 9;
	DKGameTime.Minute = 0;
	DKGameTime.Day = 1;
	DKGameTime.Season = 0;

	WorldRef = InWorld;
}

void UDKTimeManager::SetHourMinute(int32 InHour, int32 InMinute)
{
	DKGameTime.Hour = InHour;
	DKGameTime.Minute = InMinute;
	OnTimeUpdated.Broadcast(DKGameTime);
}

void UDKTimeManager::SetSeasonDay(int32 InSeason, int32 InDay)
{
	DKGameTime.Season = InSeason;
	DKGameTime.Day = InDay;
	OnDayUpdated.Broadcast(DKGameTime);
}

void UDKTimeManager::StartTime()
{
	if (WorldRef.IsValid())
	{
		WorldRef->GetTimerManager().SetTimer(
			TickHandle, this, &UDKTimeManager::TickTime, 1.0f, true);
	}

}

void UDKTimeManager::PauseTime()
{
	if (WorldRef.IsValid())
	{
		WorldRef->GetTimerManager().PauseTimer(TickHandle);
		bPaused = true;
	}
}

void UDKTimeManager::ResumeTime()
{
	if (WorldRef.IsValid())
	{
		WorldRef->GetTimerManager().UnPauseTimer(TickHandle);
		bPaused = false;
	}
}

void UDKTimeManager::TickTime()
{
	int32 PrevDay = DKGameTime.Day;
	DKGameTime.Minute++;
	if (DKGameTime.Minute >= 60)
	{
		DKGameTime.Minute = 0;
		DKGameTime.Hour++;
	}

	OnTimeUpdated.Broadcast(DKGameTime);
	
	if (DKGameTime.Hour >= 24)
	{
		NextDay();
	}

	
}

void UDKTimeManager::ClearTime()
{
	if (WorldRef.IsValid())
	{
		WorldRef->GetTimerManager().ClearTimer(TickHandle);
	}
}

void UDKTimeManager::NextDay()
{
	DKGameTime.Day++;
	if (DKGameTime.Day >= 31)
	{
		DKGameTime.Season++;
		if (DKGameTime.Season >= 3)
		{
			DKGameTime.Season = 0;
		}
	}
	DKGameTime.Hour = 9;
	DKGameTime.Minute = 0;
	
	OnTimeUpdated.Broadcast(DKGameTime);
	ClearTime();
}
