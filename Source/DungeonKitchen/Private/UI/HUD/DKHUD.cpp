// Copyright © 2025 Tartare Studio


#include "UI/HUD/DKHUD.h"

#include "UI/WidgetController/DKOverlayWidgetController.h"
#include "UI/Widgets/DKPlayWidget.h"

UDKOverlayWidgetController* ADKHUD::GetOverlayWidgetController(const FwidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UDKOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);

		OverlayWidgetController->BindCallbacksToDependencies();

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

void ADKHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized"));

	UUserWidget* Widget =  CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UDKPlayWidget>(Widget);

	const FwidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UDKOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);

	WidgetController->BroadcastInitialValues();

	Widget->AddToViewport();
}

void ADKHUD::SetVisibleOverlay(bool bShow)
{
	if (!OverlayWidget)
		return;

	if (bShow)
	{
		OverlayWidget->AddToViewport();
	}
	else
	{
		OverlayWidget->RemoveFromParent();
	}
}


