// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DKHUD.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
struct FwidgetControllerParams;
class UDKOverlayWidgetController;
class UDKPlayWidget;
/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API ADKHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UDKPlayWidget> OverlayWidget;

	UDKOverlayWidgetController* GetOverlayWidgetController(const FwidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	UFUNCTION(BlueprintCallable)
	void SetVisibleOverlay(bool bShow);

protected:


private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UDKPlayWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UDKOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UDKOverlayWidgetController> OverlayWidgetControllerClass;
};
