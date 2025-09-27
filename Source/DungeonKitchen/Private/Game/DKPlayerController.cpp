// Copyright © 2025 Tartare Studio


#include "Game/DKPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/DKWeaponAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widgets/DKPauseMenuWidget.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "UI/Widgets/DKInventoryWidget.h"
#include "UI/Widgets/DKCookingUIWidget.h"
#include "Actor/Weapon/GunBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Interface/DKInteractInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DKBlockDragDropOp.h"
#include "UI/Widgets/DamageTextComponent.h"
#include "UI/Widgets/DKGameOverWidget.h"

ADKPlayerController::ADKPlayerController()
{
	bReplicates = true;
}

void ADKPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	ADKPlayerCharacter* DKPlayer = Cast<ADKPlayerCharacter>(GetCharacter());
	if (DKPlayer->GetVelocity().Length() > 0.1 && !bIsDashing && !DKPlayer->GetCharacterMovement()->IsFalling())
	{
		DKPlayer->CurSoundTime += DeltaTime;
		if (DKPlayer->CurSoundTime > DKPlayer->WalkSoundDelay) DKPlayer->PlayWalkSound();
	}
}

void ADKPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount);
	}
}

void ADKPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(DKContext);

	UEnhancedInputLocalPlayerSubsystem* ss = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(ss)
	{
		ss->AddMappingContext(DKContext, 0);
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	ADKPlayerCharacter* DKPlayer = Cast<ADKPlayerCharacter>(GetCharacter());
	if(DKPlayer)
	{
		DKPlayer->OnPlayerDieEvent.AddDynamic(this, &ADKPlayerController::ShowGameOverUI);
	}
}

void ADKPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ADKPlayerController::Move);
	Input->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ADKPlayerController::Turn);
	Input->BindAction(IA_Jump, ETriggerEvent::Started, this, &ADKPlayerController::InputJump);
	Input->BindAction(IA_Dash, ETriggerEvent::Started, this, &ADKPlayerController::Dash);
	Input->BindAction(IA_Attack, ETriggerEvent::Triggered, this, &ADKPlayerController::Attack);
	Input->BindAction(IA_Attack, ETriggerEvent::Started, this, &ADKPlayerController::Attack);
	Input->BindAction(IA_Attack, ETriggerEvent::Completed, this, &ADKPlayerController::OnAttackReleased);
	Input->BindAction(IA_Reload, ETriggerEvent::Started, this, &ADKPlayerController::Reload);
	Input->BindAction(IA_Interact, ETriggerEvent::Started, this, &ADKPlayerController::Interact);
	Input->BindAction(IA_Drop, ETriggerEvent::Started, this, &ADKPlayerController::DropWeapon);
	Input->BindAction(IA_Swap1, ETriggerEvent::Started, this, &ADKPlayerController::Swap1);
	Input->BindAction(IA_Swap2, ETriggerEvent::Started, this, &ADKPlayerController::Swap2);

	Input->BindAction(IA_PauseMenu, ETriggerEvent::Started, this, &ADKPlayerController::TogglePauseMenu);
	Input->BindAction(IA_Inventory, ETriggerEvent::Started, this, &ADKPlayerController::ToggleInventory);
	//Input->BindAction(IA_CookingTest, ETriggerEvent::Started, this, &ADKPlayerController::ToggleCookingTest);
	Input->BindAction(IA_RotationBlock, ETriggerEvent::Completed, this, &ADKPlayerController::RotateBlock);
}

void ADKPlayerController::Move(const FInputActionValue& Val)
{
	//if (bIsDashing) return;
	FVector2D InputAxisVector = Val.Get<FVector2D>();
	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0, Rotation.Yaw, 0);

	FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void ADKPlayerController::Turn(const FInputActionValue& Val)
{
	FVector2D LookAxisVector = Val.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddControllerPitchInput(LookAxisVector.Y);
		ControlledPawn->AddControllerYawInput(LookAxisVector.X);
	}
}

void ADKPlayerController::InputJump(const FInputActionValue& Val)
{

	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void ADKPlayerController::Dash()
{
	// 대시 쿨다운 중이거나 이미 대시 중이면 리턴
	if (!bCanDash || bIsDashing)
	{
		return;
	}

	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		FVector velo = ControlledCharacter->GetVelocity();
		velo.Z = 0;
		FVector force;
		if (velo.Length() > 0.1)
			force = velo.GetSafeNormal();
		else
			force = ControlledCharacter->GetActorForwardVector();

		// 대시 사운드
		ADKPlayerCharacter* DKPlayer = Cast<ADKPlayerCharacter>(ControlledCharacter);
		if (DKPlayer->DashSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DKPlayer->DashSound, DKPlayer->GetActorLocation());
		}

		// launchcharacter (기존 방식)
		if (bDashType)
		{
			if (ControlledCharacter->GetCharacterMovement()->IsFalling())
				ControlledCharacter->LaunchCharacter(force*DashAmount/5, true, true);
			else
				ControlledCharacter->LaunchCharacter(force*DashAmount, true, true);
		}
		// 겐지 스타일 대시 - 부드러운 이동
		else
		{
			// 대시 시작
			bIsDashing = true;
			bCanDash = false;
			DashDirection = force;

			TargetLoc = ControlledCharacter->GetActorLocation() + DashDirection*DashAmount;
			// 대시
			GetWorldTimerManager().SetTimer(DashTimerHandle, this, &ADKPlayerController::CheckDash,0.02f,true);
		}
	}
}

void ADKPlayerController::FinishDash()
{
	bIsDashing = false;
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		// 대시 완료 후 속도 초기화 (선택사항)
		ControlledCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}
}

void ADKPlayerController::ResetDashCooldown()
{
	bCanDash = true;
}

void ADKPlayerController::Attack()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		//AWeaponBase* Weapon = ControlledCharacter->bIsWeapon1 ? ControlledCharacter->Weapon1 : ControlledCharacter-> Weapon2;
		if(!ControlledCharacter->Weapon || !ControlledCharacter->bCanAttack || ControlledCharacter->GetCurPlayerState() == EDKPlayerState::Reloading) return;

		// 총이라면
		if (AGunBase* Gun = Cast<AGunBase>(ControlledCharacter->Weapon))
		{
			// 총알수 확인
			if(ControlledCharacter->Weapon->CurAmmo <= 0)
			{
				Reload();
				return;
			}

			// 총알 감소
			--ControlledCharacter->Weapon->CurAmmo;
			ControlledCharacter->Weapon->SetCurAmmo();
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Reloaded! Ammo: %.0f/%.0f"), ControlledCharacter->Weapon->CurAmmo, ControlledCharacter->Weapon->MaxAmmo));
		}

		// 공격
		ControlledCharacter->Attack();
		ControlledCharacter->bCanAttack = false;

		UDKWeaponAttributeSet* watt = Cast<UDKWeaponAttributeSet>(ControlledCharacter->Weapon->GetAttributeSet());
		// attack delay
		FTimerHandle AttackTimer;
		GetWorldTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda(
		[this, ControlledCharacter]()
		{
			ControlledCharacter->bCanAttack = true;
		}), watt->GetFireDelay(),false);

	}
}

void ADKPlayerController::Reload()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		if (ControlledCharacter->GetCurPlayerState() == EDKPlayerState::Reloading) return;
		//AWeaponBase* Weapon = ControlledCharacter->bIsWeapon1 ? ControlledCharacter->Weapon1 : ControlledCharacter-> Weapon2;
		if (ControlledCharacter->Weapon)
		{
			// GunBase로 캐스팅하여 Reload 함수 호출
			if (AGunBase* Gun = Cast<AGunBase>(ControlledCharacter->Weapon))
			{
				UDKWeaponAttributeSet* GunAttributeSet = Cast<UDKWeaponAttributeSet>(Gun->GetAttributeSet());

				if (GunAttributeSet->GetCurAmmo() == GunAttributeSet->GetAmmo()) return;

				ControlledCharacter->SetPlayerState(EDKPlayerState::Reloading);

				GetWorldTimerManager().SetTimer(ReloadTimer, FTimerDelegate::CreateLambda(
					[this, ControlledCharacter, Gun, GunAttributeSet]()
					{
						CurReloadTime += 0.05;
						OnReloading.Broadcast(CurReloadTime / GunAttributeSet->GetReloadTime());
						if (CurReloadTime >= GunAttributeSet->GetReloadTime())
						{
							Gun->Reload();
							ControlledCharacter->SetPlayerState(EDKPlayerState::Idle);
							CurReloadTime = 0;
							GetWorldTimerManager().ClearTimer(ReloadTimer);
						}
					}), 0.05,true);
			}
		}
	}
}

void ADKPlayerController::Interact()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		// 캐릭터의 체크액터가 있다면
		if(ControlledCharacter->CheckActor)
		{
			// Weapon 이라면
			/*if (AWeaponBase* Weapon = Cast<AWeaponBase>(ControlledCharacter->CheckActor))
			{
				ControlledCharacter->EquipWeapon(Weapon);
			}*/

			if ( IDKInteractInterface* InteractActor = Cast<IDKInteractInterface>(ControlledCharacter->CheckActor) )
			{
				InteractActor->Interact();
			}
		}
	}
}

void ADKPlayerController::DropWeapon()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		ControlledCharacter->UnEquipWeapon();
	}
}

void ADKPlayerController::Swap1()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		ControlledCharacter->bIsWeapon1 = true;
		ControlledCharacter->ChangeWeapon();
	}
}

void ADKPlayerController::Swap2()
{
	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		ControlledCharacter->bIsWeapon1 = false;
		ControlledCharacter->ChangeWeapon();
	}
}

void ADKPlayerController::OnAttackReleased()
{
	UE_LOG(LogTemp, Log, TEXT("DKPlayerController: OnAttackReleased called"));

	if (ADKPlayerCharacter* ControlledCharacter = Cast<ADKPlayerCharacter>(GetCharacter()))
	{
		ControlledCharacter->OnAttackReleased();
	}
}

void ADKPlayerController::TogglePauseMenu()
{
	if (bIsPauseMenuOpen)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, TEXT("PauseMenu close"));
		// PauseMenu 닫기
		SetPause(false);
		if (PauseMenuWidget)
		{
			// PauseMenuWidget->RemoveFromParent();
			PopWidget();
			PauseMenuWidget = nullptr;
		}

		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bIsPauseMenuOpen = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Blue, TEXT("PauseMenu open"));
		// PauseMenu 열기
		SetPause(true);

		static TSubclassOf<UDKPauseMenuWidget> PauseMenuClass = LoadClass<UDKPauseMenuWidget>(nullptr, TEXT("/Game/05_UI/PauseMenu/Widgets/WBP_PauseMenu.WBP_PauseMenu_C"));

		if (PauseMenuClass)
		{
			PauseMenuWidget = CreateWidget<UDKPauseMenuWidget>(this, PauseMenuClass);
			if (PauseMenuWidget)
			{
				PushWidget(PauseMenuWidget);

				FInputModeGameAndUI InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);

				PauseMenuWidget->SetKeyboardFocus();

				bShowMouseCursor = true;
				bIsPauseMenuOpen = true;
			}
		}
	}
}

void ADKPlayerController::ToggleInventory()
{
	if(bIsCookingTestOpen) return;

	if (bIsInventoryOpen)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, TEXT("Inventory close"));
		SetPause(false);
		if (InventoryWidget)
		{
			PopWidget();
			InventoryWidget = nullptr;
		}

		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bIsInventoryOpen = false;
		bCanRotateBlock = false;

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Blue, TEXT("Inventory open"));
		SetPause(true);

		static TSubclassOf<UUserWidget> InventoryClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/05_UI/Cooking/Widgets/Dish/WBP_InventoryUI.WBP_InventoryUI_C"));

		if (InventoryClass)
		{
			InventoryWidget = CreateWidget<UUserWidget>(this, InventoryClass);
			if (InventoryWidget)
			{
				PushWidget(InventoryWidget);

				FInputModeGameAndUI InputMode;
				// ★ 포커스 대상을 인벤토리 위젯으로 지정
				InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
				InputMode.SetHideCursorDuringCapture(false);
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);


				// 이 줄은 지우세요: 포커스를 뷰포트로 다시 빼앗음
				// UWidgetBlueprintLibrary::SetFocusToGameViewport();

				// 보조로 한 번 더(선택): Slate 레벨 포커스
				InventoryWidget->SetKeyboardFocus();

				bShowMouseCursor = true;
				bIsInventoryOpen = true;
				bCanRotateBlock = true;
			}
		}
	}
}

void ADKPlayerController::ToggleCookingTest()
{
	//if(!bCanInteractWithCauldron) return;

	if (bIsCookingTestOpen)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, TEXT("CookingUI Test close"));
		SetPause(false);
		if (CookingUIWidget)
		{
			PopWidget();
			CookingUIWidget = nullptr;
		}

		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bIsCookingTestOpen = false;
		bCanRotateBlock = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Blue, TEXT("CookingUI Test open"));
		SetPause(true);

		static TSubclassOf<UDKCookingUIWidget> CookingUITestClass = LoadClass<UDKCookingUIWidget>(nullptr, TEXT("/Game/05_UI/Cooking/Widgets/WBP_CookingUI.WBP_CookingUI_C"));

		if (CookingUITestClass)
		{
			CookingUIWidget = CreateWidget<UDKCookingUIWidget>(this, CookingUITestClass);
			if (CookingUIWidget)
			{
				PushWidget(CookingUIWidget);

				FInputModeGameAndUI InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);

				CookingUIWidget->SetKeyboardFocus();

				bShowMouseCursor = true;
				bIsCookingTestOpen = true;
				bCanRotateBlock = true;
			}
		}
	}
}

void ADKPlayerController::RotateBlock()
{
	UE_LOG(LogTemp, Log, TEXT("RotateBlock called"));

	if (!bCanRotateBlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rotate blocked: bCanRotateBlock=false"));
		return;
	}

	// 🔸 현재 드래그 오퍼레이션에서 바로 소유 위젯을 꺼내 호출
	if (UDragDropOperation* Op = UWidgetBlueprintLibrary::GetDragDroppingContent())
	{
		if (auto* BOp = Cast<UDKBlockDragDropOp>(Op))
		{
			if (BOp->SourceInventoryWidget.IsValid())
			{
				BOp->SourceInventoryWidget->RotateActiveDrag(+1);
				return;
			}
			UE_LOG(LogTemp, Error, TEXT("Rotate: SourceInventoryWidget invalid"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Rotate: DragDropOperation is not UDKBlockDragDropOp"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Rotate: No DragDroppingContent (not dragging)"));
	}

}

void ADKPlayerController::PushWidget(class UUserWidget* NewWidget)
{
	if (!NewWidget) return;

	if (WidgetStack.Num() > 0)
	{
		WidgetStack.Last()->SetVisibility(ESlateVisibility::Collapsed);
	}

	WidgetStack.Add(NewWidget);
	NewWidget->AddToViewport();
	NewWidget->SetVisibility(ESlateVisibility::Visible);
}

void ADKPlayerController::PopWidget()
{
	if (WidgetStack.Num() == 0) return;

	UUserWidget* Top = WidgetStack.Last();
	Top->RemoveFromParent();
	WidgetStack.Pop();

	if (WidgetStack.Num() > 0)
	{
		WidgetStack.Last()->SetVisibility(ESlateVisibility::Visible);
	}
}

void ADKPlayerController::ShowGameOverUI()
{
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Blue, TEXT("GameOver Open"));

	SetPause(true);

	static TSubclassOf<UDKGameOverWidget> GameOverClass = LoadClass<UDKGameOverWidget>(nullptr, TEXT("/Game/05_UI/Gameover/WBP_GameOver.WBP_GameOver_C"));

	if (GameOverClass)
	{
		GameOverWidget = CreateWidget<UDKGameOverWidget>(this, GameOverClass);
		if (GameOverWidget)
		{
			PushWidget(GameOverWidget);

			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);

			GameOverWidget->SetKeyboardFocus();

			bShowMouseCursor = true;
			bIsCookingTestOpen = true;
		}
	}
}

void ADKPlayerController::CheckDash()
{
	CurDashTime += GetWorld()->DeltaTimeSeconds;

	ACharacter* ControlledCharacter = GetCharacter();
	FVector CurLoc = ControlledCharacter->GetActorLocation();
	//FVector NewLoc = FMath::Lerp(CurLoc, TargetLoc, CurDashTime / DashTime);
	FVector NewLoc = FMath::VInterpTo(CurLoc, TargetLoc, GetWorld()->DeltaTimeSeconds, DashSpeed);

	ControlledCharacter->SetActorLocation(NewLoc, true);

	if (FVector::Dist(CurLoc, TargetLoc) < 100.0f || CurDashTime >= DashTime )
	{
		ControlledCharacter->SetActorLocation(TargetLoc, true);
		bIsDashing = false;
		CurDashTime = 0;
		GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);


		// 대시 쿨다운 시작
		 GetWorld()->GetTimerManager().SetTimer(DashCooldownTimerHandle, this, &ADKPlayerController::ResetDashCooldown, DashCooldown, false);

	}
}

