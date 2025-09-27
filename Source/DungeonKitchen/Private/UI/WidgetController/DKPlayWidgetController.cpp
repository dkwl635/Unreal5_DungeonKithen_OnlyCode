// Copyright © 2025 Tartare Studio


#include "UI/WidgetController/DKPlayWidgetController.h"

void UDKPlayWidgetController::SetWidgetControllerParams(const FwidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UDKPlayWidgetController::BroadcastInitialValues()
{
}

void UDKPlayWidgetController::BindCallbacksToDependencies()
{
}
