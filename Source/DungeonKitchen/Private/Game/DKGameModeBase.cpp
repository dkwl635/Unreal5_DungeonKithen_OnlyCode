// Copyright © 2025 Tartare Studio



#include "Game/DKGameModeBase.h"
#include "Game/DKPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/DKOptionWidget.h"
#include "UI/Widgets/DKPopupWidget.h"
#include "Game/Subsystem/DigestiveSystem.h"
#include "Game/Subsystem/CookingSystem.h"
#include "Game/Subsystem/PlayerInventoryManager.h"

void ADKGameModeBase::HandleStartGame(TSoftObjectPtr<UWorld> LevelToOpen)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Digest = GI->GetSubsystem<UDigestiveSystem>())
			Digest->PrepareForRestart();

		if (auto* Cooking = GI->GetSubsystem<UCookingSystem>())
		{
			for (int i = 0; i < 3; i++)
			{
				Cooking->RemoveSlotFood(i);
			}
		}

		if (auto* Inv = GI->GetSubsystem<UPlayerInventoryManager>())
		{
			Inv->Foods.Empty();
			Inv->Foods.Init(nullptr, 25);
		}
			
	}

	if (LevelToOpen.IsNull() == false)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Cyan, TEXT("BattleLevel is not set!"));
	}
}

void ADKGameModeBase::HandleContinue()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ADKPlayerController* DKPC = Cast<ADKPlayerController>(PC))
	{
		DKPC->TogglePauseMenu();
	}
}

void ADKGameModeBase::HandleOption()
{
	GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Magenta, TEXT("BattleGB Option Click"));

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ADKPlayerController* DKPC = Cast<ADKPlayerController>(PC))
	{
		OptionWidgetClass = LoadClass<UDKOptionWidget>(nullptr, TEXT("/Game/05_UI/Option/WBP_OptionWidget.WBP_OptionWidget_C"));

		if (OptionWidgetClass)
		{
			OptionWidget = CreateWidget<UDKOptionWidget>(GetWorld(), OptionWidgetClass);
			if (OptionWidget)
			{
				DKPC->PushWidget(OptionWidget);
			}
		}
	}
}

void ADKGameModeBase::HandleMainMenu(TSoftObjectPtr<UWorld> LevelToOpen)
{
	PendingAction = EPendingUIAction::GoToMainMenu;
	PendingLevel = LevelToOpen;

	ShowConfirmPopup(
		FText::FromString(TEXT("메인 메뉴로 돌아갈까요?")),
		FText::FromString(TEXT(""))
		//FText::FromString(TEXT("저장되지 않은 진행 상황은 사라집니다."))
	);
}

void ADKGameModeBase::HandleExitGame()
{
	PendingAction = EPendingUIAction::ExitGame;
	PendingLevel = nullptr;

	ShowConfirmPopup(
		FText::FromString(TEXT("게임을 종료할까요?")),
		FText::FromString(TEXT(""))
		//FText::FromString(TEXT("저장되지 않은 진행 상황은 사라집니다."))
	);
}

void ADKGameModeBase::HandleBack()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ADKPlayerController* DKPC = Cast<ADKPlayerController>(PC))
	{
		DKPC->PopWidget();
	}
}

void ADKGameModeBase::HandleYes()
{
	switch (PendingAction)
	{
	case EPendingUIAction::GoToMainMenu:
		if (!PendingLevel.IsNull())
		{
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, PendingLevel);
		}
		break;
	case EPendingUIAction::ExitGame:
		UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
		break;
	default:
		break;
	}

	PendingAction = EPendingUIAction::None;
	PendingLevel = nullptr;
	CloseConfirmPopup();
}

void ADKGameModeBase::HandleNo()
{
	// 팝업 No → 취소하고 팝업만 닫음
	PendingAction = EPendingUIAction::None;
	PendingLevel = nullptr;
	CloseConfirmPopup();
}

void ADKGameModeBase::ShowConfirmPopup(const FText& Title, const FText& Body)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ADKPlayerController* DKPC = Cast<ADKPlayerController>(PC))
	{
		if (!PopupWidgetClass)
		{
			PopupWidgetClass = LoadClass<UDKPopupWidget>(nullptr, TEXT("/Game/05_UI/Popup/WBP_Popup.WBP_Popup_C"));
		}
		if (PopupWidgetClass)
		{
			PopupWidget = CreateWidget<UDKPopupWidget>(GetWorld(), PopupWidgetClass);
			if (PopupWidget)
			{
				PopupWidget->SetTexts(Title, Body);

				DKPC->PushWidget(PopupWidget); 
			}
		}
	}
}

void ADKGameModeBase::CloseConfirmPopup()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ADKPlayerController* DKPC = Cast<ADKPlayerController>(PC))
	{
		if (PopupWidget)
		{
			DKPC->PopWidget();
			PopupWidget = nullptr;
		}
	}
}
