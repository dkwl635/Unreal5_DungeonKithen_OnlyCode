// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKCookingUIWidget.h"
#include "Game/DKPlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/DKBlockDragDropOp.h"
#include "Components/Button.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Game/Subsystem/CookingSystem.h"

void UDKCookingUIWidget::NativeConstruct()
{
	Button_Close->OnClicked.AddDynamic(this, &UDKCookingUIWidget::OnCloseButtonClicked);
}

FReply UDKCookingUIWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("Escape Click"));
        // 컨트롤러 캐스팅 후, 메뉴 닫기 함수 호출
        ADKPlayerController* PlayerController = Cast<ADKPlayerController>(GetOwningPlayer());
        if (PlayerController)
        {
            PlayerController->ToggleCookingTest();
            return FReply::Handled(); // 입력이 처리되었음을 알림
        }
    }
	// 회전: E 키
	if (InKeyEvent.GetKey() == EKeys::E)
	{
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("E Click"));
		ADKPlayerController* PlayerController = Cast<ADKPlayerController>(GetOwningPlayer());
		UE_LOG(LogTemp, Warning, TEXT("[CookUI] OwningPlayer=%s Class=%s"),
			*GetNameSafe(PlayerController), *GetNameSafe(PlayerController->GetClass()));
		if (PlayerController)
		{
			PlayerController->RotateBlock();
			return FReply::Handled(); // 입력이 처리되었음을 알림
		}
	}
    return FReply::Unhandled(); // 처리가 안 된 입력은 다시 상위로 전달
}

void UDKCookingUIWidget::OnCloseButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Purple, TEXT("X Button Click"));
	ADKPlayerController* PlayerController = Cast<ADKPlayerController>(GetOwningPlayer());
	// AutoReturnPrepToPantry();
	if (PlayerController)
	{
		PlayerController->ToggleCookingTest();
	}
}

void UDKCookingUIWidget::AutoReturnPrepToPantry()
{
	auto* Inv = GetGameInstance()->GetSubsystem<UPlayerInventoryManager>();
	auto* Cook = GetGameInstance()->GetSubsystem<UCookingSystem>();
	if (!Cook) return;

	const int32 N = Cook->GetPrepSlotCount();
	for (int32 i = 0; i < N; ++i)
	{
		if (UDKFood* F = Cook->GetSlotFood(i))
		{
			if (Inv)
			{
				Inv->FirstAddFood(F); // 너희 쪽 시그니처에 맞춰서 사용
			}
			Cook->RemoveSlotFood(i);
		}
	}
	
}

