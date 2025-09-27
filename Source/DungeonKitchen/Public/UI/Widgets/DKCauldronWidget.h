// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DKCauldronWidget.generated.h"

class UCookingSystem;
class UImage;
class UTexture2D;
class UDKDish;

UCLASS()
class DUNGEONKITCHEN_API UDKCauldronWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// WBP_Cauldron 안의 Image 위젯(이 이름으로 바인딩하거나 Details에서 변수명 맞춰줘)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Cauldron = nullptr;

	// 팔레트에 블록(요리)이 "있을 때" 보여줄 텍스처(= 1번 이미지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cauldron")
	TObjectPtr<UTexture2D> Image_WhenHasDish = nullptr;

	// 팔레트가 "비었을 때" 보여줄 텍스처(= 2번 이미지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cauldron")
	TObjectPtr<UTexture2D> Image_WhenEmpty = nullptr;

	// 브러시 이미지 크기(선택)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cauldron")
	FVector2D BrushImageSize = FVector2D(512.f, 512.f);

	// CookingSystem 핸들(구독/해제용)
	UPROPERTY()
	TObjectPtr<UCookingSystem> CookingSystem = nullptr;

	// Dish 생성/해제 이벤트를 받는 콜백
	UFUNCTION()
	void HandleDishChanged(UDKDish* Dish);

	// 실제로 이미지 교체
	void ApplyTexture(UTexture2D* Tex);
};
