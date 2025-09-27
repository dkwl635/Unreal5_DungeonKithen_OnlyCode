// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DKGameOverWidget.generated.h"

class UDKButtonBase;
class UDKMenuViewModel;
/**
 * 
 */
UCLASS()
class DUNGEONKITCHEN_API UDKGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UDKButtonBase* Button_Restart;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UDKButtonBase* Button_MainMenu;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UDKButtonBase* Button_ExitGame;

	UPROPERTY()
	UDKMenuViewModel* ViewModel;

	UPROPERTY(EditAnywhere, Category="VM")
	TSubclassOf<UDKMenuViewModel> ViewModelClass;

	UFUNCTION()
    void OnRestartClicked();

	UFUNCTION()
	void OnMainMenuClicked();

    UFUNCTION()
    void OnExitGameClicked();
	
};
