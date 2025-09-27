// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKCauldronWidget.h"
#include "Components/Image.h"
#include "Game/Subsystem/CookingSystem.h"

void UDKCauldronWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// CookingSystem 구하고 이벤트 구독
	if (UGameInstance* GI = GetGameInstance())
	{
		CookingSystem = GI->GetSubsystem<UCookingSystem>();
		if (CookingSystem)
		{
			CookingSystem->OnDishCreated.AddDynamic(this, &UDKCauldronWidget::HandleDishChanged);

			// 초기 상태 동기화 (이미 만들어진 Dish가 있을 수 있음)
			HandleDishChanged(CookingSystem->GetMakedDish());
		}
	}
}

void UDKCauldronWidget::NativeDestruct()
{
	if (CookingSystem)
	{
		CookingSystem->OnDishCreated.RemoveDynamic(this, &UDKCauldronWidget::HandleDishChanged);
		CookingSystem = nullptr;
	}
	Super::NativeDestruct();
}

void UDKCauldronWidget::HandleDishChanged(UDKDish* Dish)
{
	// Dish != nullptr  => 팔레트에 블록 있음 → 1번 이미지
	// Dish == nullptr  => 팔레트 비었음       → 2번 이미지
	if (Dish) ApplyTexture(Image_WhenHasDish);
	else      ApplyTexture(Image_WhenEmpty);
}

void UDKCauldronWidget::ApplyTexture(UTexture2D* Tex)
{
	if (!IMG_Cauldron) return;

	if (Tex)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Tex);
		Brush.ImageSize = BrushImageSize;  
		Brush.DrawAs = ESlateBrushDrawType::Image;
		IMG_Cauldron->SetBrush(Brush);
		IMG_Cauldron->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 텍스처가 비어있으면 안전하게 숨김
		IMG_Cauldron->SetVisibility(ESlateVisibility::Collapsed);
	}
}
