// Copyright © 2025 Tartare Studio


#include "Character/Player/DKPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/DKAbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Character/Player/DKPlayerState.h"
#include "Game/DKPlayerController.h"
#include "UI/HUD/DKHUD.h"
#include "Actor/Weapon/WeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"


ADKPlayerCharacter::ADKPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//캡슐 설정
	GetCapsuleComponent()->SetCapsuleHalfHeight(120.f);

	// 카메라 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(RootComponent);
	FollowCamera->bUsePawnControlRotation = true;
	FollowCamera->SetRelativeLocation(FVector(-10.f, 0.f, 120.f)); // Position the camera
	FollowCamera->bUsePawnControlRotation = true;

	// 메시 설정
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FollowCamera);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// 무기 위치 설정
	WeaponPos = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponPos"));
	WeaponPos->SetupAttachment(FollowCamera);
	WeaponPos->SetRelativeLocation(FVector(50.f, 30.f, -10.f));

	// 이동속도 설정
	GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;

	// 씬 캡쳐 설정
	SceneSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SceneSpringArm"));
	SceneSpringArm->SetupAttachment(RootComponent);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(SceneSpringArm);

	SceneSpringArm->SetRelativeRotation(FRotator(-90.f, 0.f,0.f));
	SceneSpringArm->TargetArmLength = 1000.f;

	SceneCapture->ProjectionType = ECameraProjectionMode::Orthographic;
}

void ADKPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle CheckHandle;
	GetWorld()->GetTimerManager().SetTimer(CheckHandle, this, &ADKPlayerCharacter::checkTrace, 0.1, true);

	if (WeaponClass)
	{
		Weapon1 = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
		EquipWeapon(Weapon1);
		Weapon = Weapon1;
		ApplyEffectToWeapon(EffectWeaponAttributes, 1);
	}
}

void ADKPlayerCharacter::checkTrace()
{
	FVector StartPos = FollowCamera->GetComponentLocation();
	FVector EndPos = StartPos + FollowCamera->GetForwardVector() * FindDistance;
	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceMultiByChannel(Hits, StartPos, EndPos, ECC_GameTraceChannel5, Params);
	//DrawDebugLine(GetWorld(), StartPos, EndPos, FColor(255, 0, 0), false, 0.2);
	if (bHit)
	{
		for (auto hit : Hits)
		{
			CheckActor = hit.GetActor();
			break;
		}
	}
	else CheckActor = nullptr;
	OnChangeCheckWeaponEvent.Broadcast(CheckActor);
}

void ADKPlayerCharacter::ApplyEffectToWeapon(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	Super::ApplyEffectToWeapon(GameplayEffectClass, Level);

	if (!Weapon || !GameplayEffectClass) return;

	UAbilitySystemComponent* WeaponASC = Weapon->GetAbilitySystemComponent();
	FGameplayEffectContextHandle ContextHandle = WeaponASC->MakeEffectContext();
	ContextHandle.AddSourceObject(Weapon);
	FGameplayEffectSpecHandle SpecHandle = WeaponASC->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	WeaponASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ADKPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the server
	InitAbilityActorInfo();
}

void ADKPlayerCharacter::InitAbilityActorInfo()
{
	ADKPlayerState* DKPlayerState = GetPlayerState<ADKPlayerState>();
	check(DKPlayerState);
	DKPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(DKPlayerState, this);
	Cast<UDKAbilitySystemComponent>(DKPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = DKPlayerState->GetAbilitySystemComponent();
	AttributeSet = DKPlayerState->GetAttributeSet();

	const UDKCharacterAttributeSet* DKAttributeSet = CastChecked<UDKCharacterAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		DKAttributeSet->GetHPAttribute()).AddUObject(this, &ADKPlayerCharacter::OnHPAttributeChanged);

	if (ADKPlayerController* DKPlayerController = Cast<ADKPlayerController>(GetController()))
	{
		if (ADKHUD* DKHUD = Cast<ADKHUD>(DKPlayerController->GetHUD()))
		{
			DKHUD->InitOverlay(DKPlayerController, DKPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeDefaultAttributes();
}

void ADKPlayerCharacter::Attack()
{
	if (Weapon)
	{
		Weapon->Attack();
	}
}

void ADKPlayerCharacter::OnAttackReleased()
{
	if (Weapon)
	{
		Weapon->OnAttackCompleted();
	}
}

EDKPlayerState ADKPlayerCharacter::GetCurPlayerState()
{
	return PlayerState;
}

void ADKPlayerCharacter::SetPlayerState(EDKPlayerState NewState)
{
	PlayerState = NewState;
}

void ADKPlayerCharacter::EquipWeapon(AWeaponBase* NewWeapon)
{
	// 무기가 있으면
	if (Weapon)
	{
		// 무기 unequip
		UnEquipWeapon();
	}
	Weapon = NewWeapon;
	Weapon->AttachToComponent(WeaponPos, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Weapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetOwner(this);
	if(bIsWeapon1) Weapon1 = NewWeapon;
	else Weapon2 = NewWeapon;
	ApplyEffectToWeapon(EffectWeaponAttributes, 1);
	OnChangeWeaponEvent.Broadcast(Weapon);
}

void ADKPlayerCharacter::UnEquipWeapon()
{
	if(!Weapon) return;
	Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Weapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FVector SpawnLocation = GetActorLocation();
	FVector AddVector = GetActorForwardVector();
	AddVector.Z = 0;
	SpawnLocation += AddVector*100;
	Weapon->SetActorLocationAndRotation( SpawnLocation,GetActorForwardVector().Rotation() + FRotator(0.f, 90.f, 0.f));
	if(bIsWeapon1)  Weapon1 = nullptr;
	else Weapon2 = nullptr;
	Weapon = nullptr;

	OnChangeWeaponEvent.Broadcast(Weapon);
}

void ADKPlayerCharacter::ChangeWeapon()
{
	AWeaponBase* ActiveWeapon = bIsWeapon1 ? Weapon1 : Weapon2;
	AWeaponBase* DeactiveWeapon = bIsWeapon1 ? Weapon2 : Weapon1;
	Weapon = ActiveWeapon;
	ApplyEffectToWeapon(EffectWeaponAttributes, 1);

	OnChangeWeaponEvent.Broadcast(Weapon);

	if (ActiveWeapon) ActiveWeapon->WeaponMesh->SetVisibility(true);
	if (DeactiveWeapon) DeactiveWeapon->WeaponMesh->SetVisibility(false);
}

void ADKPlayerCharacter::OnHPAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		OnDeath();
	}
}

void ADKPlayerCharacter::OnDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Player has died."));
	OnPlayerDieEvent.Broadcast();
}
