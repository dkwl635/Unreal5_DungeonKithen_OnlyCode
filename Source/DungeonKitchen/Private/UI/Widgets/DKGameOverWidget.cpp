// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKGameOverWidget.h"
#include "UI/Widgets/DKButtonBase.h" 
#include "Components/Button.h"
#include "UI/ViewModel/DKMenuViewModel.h"

void UDKGameOverWidget::NativeConstruct()
{
	// ViewModel 생성
	if (ViewModelClass) {
		ViewModel = NewObject<UDKMenuViewModel>(this, ViewModelClass);
	}

	Button_Restart->Button->OnClicked.AddDynamic(this, &UDKGameOverWidget::OnRestartClicked);
	Button_MainMenu->Button->OnClicked.AddDynamic(this, &UDKGameOverWidget::OnMainMenuClicked);
	Button_ExitGame->Button->OnClicked.AddDynamic(this, &UDKGameOverWidget::OnExitGameClicked);
}

void UDKGameOverWidget::OnRestartClicked()
{
	if (ViewModel)
	{
		ViewModel->HandleStartButton();
	}
}

void UDKGameOverWidget::OnMainMenuClicked()
{
	if (ViewModel)
	{
		ViewModel->HandleMainMenuButton();
	}
}

void UDKGameOverWidget::OnExitGameClicked()
{
	if (ViewModel)
	{
		ViewModel->HandleExitButton();
	}
}
