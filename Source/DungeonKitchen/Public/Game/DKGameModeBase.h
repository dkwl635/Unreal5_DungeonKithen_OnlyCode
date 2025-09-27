// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/DKUIHandlerInterface.h"
#include "DKGameModeBase.generated.h"

UENUM()
enum class EPendingUIAction : uint8
{
	None,
	ExitGame,
	GoToMainMenu
};
/**
 * 
 */
UCLASS()
class DUNGEONKITCHEN_API ADKGameModeBase : public AGameModeBase, public IDKUIHandlerInterface
{
	GENERATED_BODY()

public:
	virtual void HandleStartGame(TSoftObjectPtr<UWorld> LevelToOpen) override;
	virtual void HandleContinue() override;
	virtual void HandleOption() override;
	virtual void HandleMainMenu(TSoftObjectPtr<UWorld> LevelToOpen) override;
	virtual void HandleExitGame() override;
	virtual void HandleBack() override;
	virtual void HandleYes() override;
	virtual void HandleNo() override;


protected:
	EPendingUIAction PendingAction = EPendingUIAction::None;
	TSoftObjectPtr<UWorld> PendingLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UDKOptionWidget> OptionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UDKPopupWidget> PopupWidgetClass;

private:
	UPROPERTY()
	class UDKOptionWidget* OptionWidget;
	UPROPERTY()
	class UDKPopupWidget* PopupWidget;

	void ShowConfirmPopup(const FText& Title, const FText& Body);
	void CloseConfirmPopup();
};
