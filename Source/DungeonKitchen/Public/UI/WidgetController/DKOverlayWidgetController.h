// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/DKPlayWidgetController.h"
#include "DKOverlayWidgetController.generated.h"

class UDKPlayWidget;
struct FOnAttributeChangeData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHPChangedSignature, float, NewHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHPChangedSignature, float, NewMaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurAmmoChangedSignature, float, NewCurAmmo);

USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDKPlayWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Texture = nullptr;
};

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API UDKOverlayWidgetController : public UDKPlayWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;

	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHPChangedSignature OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxHPChangedSignature OnMaxHPChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	void HPChanged(const FOnAttributeChangeData& Data) const;
	void MaxHPChanged(const FOnAttributeChangeData& Data) const;
};
