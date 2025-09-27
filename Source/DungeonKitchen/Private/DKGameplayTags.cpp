// Copyright © 2025 Tartare Studio


#include "DKGameplayTags.h"
#include "GameplayTagsManager.h"

FDKGameplayTags FDKGameplayTags::GameplayTags;

void FDKGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.Attributes_Primary_DEF = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.DEF"), FString("Reduces damage Taken"));
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"), FString("Reduces damage Taken"));
	GameplayTags.Attributes_Secondary_CriticalRate = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalRate"), FString("Critical Rate"));

	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage"), FString("Damage"));
}
