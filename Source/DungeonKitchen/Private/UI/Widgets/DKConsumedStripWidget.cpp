// Copyright © 2025 Tartare Studio


#include "UI/Widgets/DKConsumedStripWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "Data/ItemDataStructures.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"


void UDKConsumedStripWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Inv = GetGameInstance()->GetSubsystem<UPlayerInventoryManager>();
	if (Inv)
	{
		Inv->OnPantryChanged.RemoveDynamic(this, &UDKConsumedStripWidget::OnFoodCame);
		Inv->OnPantryChanged.AddDynamic(this, &UDKConsumedStripWidget::OnFoodCame);

		// 초기 히스토리
		//for(auto* Icon : Inv->GetRecentFoodIcons())
		//{ PushIncon(Icon); }
	}
}

void UDKConsumedStripWidget::NativeDestruct()
{
	if (Inv)
	{
		Inv->OnPantryChanged.RemoveDynamic(this, &UDKConsumedStripWidget::OnFoodCame);
		Inv = nullptr;
	}
	Super::NativeDestruct();
}

void UDKConsumedStripWidget::OnFoodCame(const TArray<UDKFood*>& Foods)
{
	ClearStrip();

	if (!StripCanvas) return;

	// Foods 배열 그대로 사용해서 스트립 다시 그리기
	// (최근순으로 깔고 싶으면 뒤에서부터, 기본은 앞에서부터)
	for (int32 i = 0; i < Foods.Num(); ++i)
	{
		const UDKFood* Food = Foods[i];
		if (!Food) continue;

		if (UTexture2D* Icon = Food->GetItemIcon())
		{
			PushIcon(Icon);
		}
	}
}

void UDKConsumedStripWidget::PushIcon(UTexture2D* Icon)
{
	if(!StripCanvas || !Icon) return;

	// WidgetTree로 아이콘 위젯 생성
	UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	FSlateBrush Brush;
	Brush.SetResourceObject(Icon);
	Brush.ImageSize = FVector2D(IconSize, IconSize);
	Img->SetBrush(Brush);

	UCanvasPanelSlot* CanvasSlot = StripCanvas->AddChildToCanvas(Img);
	CanvasSlot->SetAutoSize(true);

	const int32 Index = IconWidgets.Num();
	const int32 Col = Index % ItemsPerRow;
	const int32 Row = Index / ItemsPerRow;

	const float X = Col * (IconSize - OverlapX);
	const float Y = Row * (IconSize + RowSpacing);

	CanvasSlot->SetPosition(FVector2D(X, Y));
	CanvasSlot->SetZOrder(Index);

	IconWidgets.Add(Img);

	// 아이콘이 여러줄일 때 strip 높이 자동 갱신
	if (USizeBox* RootSize = Cast<USizeBox>(GetRootWidget()))
	{
		const int32 Rows = Row + 1;
		const float NewHeight = Rows * IconSize + (Rows - 1) * RowSpacing;
		RootSize->SetHeightOverride(NewHeight);
	}
}

void UDKConsumedStripWidget::ClearStrip()
{
	if(!StripCanvas) return;
	for (UWidget* W : IconWidgets)
	{
		StripCanvas->RemoveChild(W);
	}
	IconWidgets.Reset();

	// strip 높이 원상복구
	if (USizeBox* RootSize = Cast<USizeBox>(GetRootWidget()))
	{
		RootSize->ClearHeightOverride();
	}
}

//void UDKConsumedStripWidget::SetWidgetController(UObject* InWidgetController)
//{
//	WidgetController = InWidgetController;
//	WidgetControllerSet();
//}
