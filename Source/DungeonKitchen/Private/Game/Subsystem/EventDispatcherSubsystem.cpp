// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/EventDispatcherSubsystem.h"
#include "Game/Event/GameplayTagEventBinder.h"


void UEventDispatcherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEventDispatcherSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ClearMap();
}

UGameplayTagEventBinder* UEventDispatcherSubsystem::GetGameTagEventBinder(UObject* WorldObject, FGameplayTag EventTag)
{
	UEventDispatcherSubsystem* Subsystem = WorldObject->GetWorld()->GetSubsystem<UEventDispatcherSubsystem>();
	check(Subsystem);
	
	return Subsystem->GetEventBinder(EventTag);
}

void UEventDispatcherSubsystem::OnGameplayTagEvent(UObject* WorlDKontextObject, FGameplayTag EventTag)
{
	UEventDispatcherSubsystem* Subsystem = WorlDKontextObject->GetWorld()->GetSubsystem<UEventDispatcherSubsystem>();
	check(Subsystem);

	Subsystem->OnEventGameplayTag(EventTag);
}

UGameplayTagEventBinder* UEventDispatcherSubsystem::GetEventBinder(FGameplayTag EventTag)
{
	if (!EventMap.Contains(EventTag))
	{
		UGameplayTagEventBinder* Tag = NewObject<UGameplayTagEventBinder>();
		EventMap.Add(EventTag, Tag);
	}
	
	return EventMap[EventTag];
}

void UEventDispatcherSubsystem::OnEventGameplayTag(FGameplayTag EventTag)
{
	UGameplayTagEventBinder* Binder = GetEventBinder(EventTag);

	if (Binder && IsValid(Binder))
	{
		Binder->EventDelegate.Broadcast(EventTag);
	}
	
}

void UEventDispatcherSubsystem::ClearMap()
{
	for (auto& Elem : EventMap)
	{
		if (Elem.Value)
		{
			Elem.Value->EventDelegate.Clear();
		}
	}
	EventMap.Empty();
}

