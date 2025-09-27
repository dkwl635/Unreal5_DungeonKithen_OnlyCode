// Copyright © 2025 Tartare Studio


#include "Game/DKMainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/DKMainMenuWidget.h"
#include "UI/Widgets/DKOptionWidget.h"

void ADKMainMenuGameMode::BeginPlay()
{
	MainMenuWidgetClass = LoadClass<UDKMainMenuWidget>(nullptr, TEXT("/Game/05_UI/MainMenu/Widgets/WBP_MainMenu.WBP_MainMenu_C"));

	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UDKMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			PushWidget(MainMenuWidget);

			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
			}
		}
	}
}

void ADKMainMenuGameMode::HandleStartGame(TSoftObjectPtr<UWorld> LevelToOpen)
{
	if (LevelToOpen.IsNull() == false)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
		
		/*UWorld* Level = LevelToOpen.LoadSynchronous();
		if (Level)
		{
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
		}*/
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Cyan, TEXT("BattleLevel is not set!"));
	}
}

void ADKMainMenuGameMode::HandleContinue()
{
	GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Cyan, TEXT("MainMenuGB Continue Click"));
}

void ADKMainMenuGameMode::HandleOption()
{
	GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Cyan, TEXT("MainMenuGB Option Click"));

	OptionWidgetClass = LoadClass<UDKOptionWidget>(nullptr, TEXT("/Game/05_UI/Option/WBP_OptionWidget.WBP_OptionWidget_C"));

	if (OptionWidgetClass)
	{
		OptionWidget = CreateWidget<UDKOptionWidget>(GetWorld(), OptionWidgetClass);
		if (OptionWidget)
		{
			PushWidget(OptionWidget);
		}
	}
}

void ADKMainMenuGameMode::HandleExitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void ADKMainMenuGameMode::HandleBack()
{
	PopWidget();
}

void ADKMainMenuGameMode::PushWidget(class UUserWidget* NewWidget)
{
	if(!NewWidget) return;

	if (WidgetStack.Num() > 0)
	{
		WidgetStack.Last()->SetVisibility(ESlateVisibility::Collapsed);
	}

	WidgetStack.Add(NewWidget);
	NewWidget->AddToViewport();
	NewWidget->SetVisibility(ESlateVisibility::Visible);
}

void ADKMainMenuGameMode::PopWidget()
{
	if(WidgetStack.Num() == 0) return;

	UUserWidget* Top = WidgetStack.Last();
	Top->RemoveFromParent();
	WidgetStack.Pop();

	if (WidgetStack.Num() > 0)
	{
		WidgetStack.Last()->SetVisibility(ESlateVisibility::Visible);
	}
}
