// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKPlayWidget.h"

void UDKPlayWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
