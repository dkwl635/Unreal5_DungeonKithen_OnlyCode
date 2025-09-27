// Copyright 짤 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DKPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReloading, float, percent);

class UDamageTextComponent;
struct FInputActionValue;

UCLASS()
class DUNGEONKITCHEN_API ADKPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ADKPlayerController();

	UPROPERTY(BlueprintAssignable)
	FOnReloading OnReloading;

    virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter);
protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputMappingContext> DKContext;

    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputAction> IA_Move;

    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputAction> IA_Turn;

    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputAction> IA_Jump;

    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputAction> IA_Dash;


    UPROPERTY(EditAnywhere, Category = "input")
    TObjectPtr<class UInputAction> IA_Attack;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Reload;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Interact;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Drop;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Swap1;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Swap2;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_PauseMenu;

    UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_Inventory;

    UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_CookingTest;

	UPROPERTY(EditAnywhere, Category = "input")
	TObjectPtr<class UInputAction> IA_RotationBlock;


    UPROPERTY()
	TArray<UUserWidget*> WidgetStack;

    UPROPERTY()
    class UDKPauseMenuWidget* PauseMenuWidget;
	UPROPERTY()
    class UDKGameOverWidget* GameOverWidget;
    UPROPERTY()
    class UUserWidget* InventoryWidget;
    UPROPERTY()
    class UDKCookingUIWidget* CookingUIWidget;

    bool bIsPauseMenuOpen = false;
    bool bIsInventoryOpen = false;
    bool bIsCookingTestOpen = false;
	bool bCanRotateBlock = false;

    void Move(const FInputActionValue& Val);
    void Turn(const FInputActionValue& Val);
    void InputJump(const FInputActionValue& Val);
    void Dash();
	void FinishDash(); // 대시 완료 함수
	void ResetDashCooldown(); // 대시 쿨다운 리셋

    void Attack();

    void OnAttackReleased();

	void Reload();

	void Interact();

	void DropWeapon();

	void Swap1();
	void Swap2();


    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashAmount = 10000.f;
	FTimerHandle DashTimerHandle;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	bool bDashType = false;

	UPROPERTY(EditDefaultsOnly,  Category = "Dash")
	float DashSpeed = 1000;


	bool bIsDashing = false; // 대시 중인지 확인

	bool bCanDash = true; // 대시 가능 여부
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldown = 1.0f; // 대시 쿨다운 시간
	FVector DashDirection; // 대시 방향 저장
	FTimerHandle DashCooldownTimerHandle; // 대시 쿨다운 타이머


public:
    void TogglePauseMenu();
    void ToggleInventory();
    void ToggleCookingTest();
	void RotateBlock();

    UFUNCTION(BlueprintCallable)
	void PushWidget(class UUserWidget* NewWidget);

	UFUNCTION(BlueprintCallable)
	void PopWidget();

	UFUNCTION(BlueprintCallable)
	void ShowGameOverUI();

	bool bIsRotatingBlock = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bCanInteractWithCauldron = false;

	FTimerHandle ReloadTimer;

	float CurReloadTime = 0;
private:
	FVector TargetLoc;
	UFUNCTION()
	void CheckDash();

	float CurDashTime = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashTime = 1;

};



