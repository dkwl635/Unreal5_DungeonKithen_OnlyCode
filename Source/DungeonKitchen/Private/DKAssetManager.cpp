// Copyright © 2025 Tartare Studio


#include "DKAssetManager.h"
#include "DKGameplayTags.h"

UDKAssetManager& UDKAssetManager::Get()
{
	check(GEngine);

	UDKAssetManager* DKAssetManager = Cast<UDKAssetManager>(GEngine->AssetManager);
	return *DKAssetManager;
}

void UDKAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FDKGameplayTags::InitializeNativeGameplayTags();
}
