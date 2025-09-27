// Copyright © 2025 Tartare Studio

#include "Actor/Weapon/WeaponBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "DKGameplayTags.h"
#include "AbilitySystem/DKAbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "AbilitySystem/DKWeaponAttributeSet.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Game/Subsystem/DataManager.h"
#include "Kismet/GameplayStatics.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComp = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComp);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	WeaponMesh->SetupAttachment(SceneComp);

	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(WeaponMesh);
	WeaponBox->SetCollisionProfileName(TEXT("Weapon"));

	AbilitySystemComponent = CreateDefaultSubobject<UDKAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UDKWeaponAttributeSet>("AttributeSet");
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	//InitDefaultWeaponAttributes();

	SetWeaponData();
}

UAbilitySystemComponent* AWeaponBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AWeaponBase::GetAttributeSet() const
{
	return AttributeSet;
}

void AWeaponBase::Interact()
{
	ADKPlayerCharacter* PlayerCharacter = Cast<ADKPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	PlayerCharacter->EquipWeapon(this);
}

void AWeaponBase::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	ADKPlayerCharacter* PlayerCharacter = Cast<ADKPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	UDKWeaponAttributeSet* WeaponAttributeSet = Cast<UDKWeaponAttributeSet>(GetAttributeSet());
	UAbilitySystemComponent* PlayerASC = PlayerCharacter->GetAbilitySystemComponent();

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = PlayerASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(PlayerCharacter);
	FGameplayEffectSpecHandle EffectSpecHandle = PlayerASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContextHandle);

	FDKGameplayTags GameplayTags = FDKGameplayTags::Get();
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Damage, WeaponAttributeSet->GetDamage());

	PlayerASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),TargetASC);
}

void AWeaponBase::InitDefaultWeaponAttributes()
{
	check(IsValid(GetAbilitySystemComponent()));
	check(DefaultWeaponAttributes);
	const FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(DefaultWeaponAttributes, 1, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AWeaponBase::SetWeaponData()
{
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (!DataManager->GetWeaponData(WeaponID, WeaponData))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed Load WeaponData"));
			return;
		}

		float Damage = WeaponData.Damage;
		float FireDelay = WeaponData.FireDelay;
		float ReloadTime = WeaponData.ReloadTime;
		float Ammo = WeaponData.Ammo;
		float Rounds = WeaponData.Rounds;

		UDKWeaponAttributeSet* WeaponAttributeSet = Cast<UDKWeaponAttributeSet>(GetAttributeSet());
		WeaponAttributeSet->InitDamage(Damage);
		WeaponAttributeSet->InitFireDelay(FireDelay);
		WeaponAttributeSet->InitReloadTime(ReloadTime);
		WeaponAttributeSet->InitAmmo(Ammo);
		WeaponAttributeSet->InitCurAmmo(Ammo);
		WeaponAttributeSet->InitRounds(Rounds);
		WeaponAttributeSet->InitFinalDamage(Damage);

		MaxAmmo = Ammo;
		CurAmmo = MaxAmmo;
	}
}

void AWeaponBase::SetCurAmmo()
{
	UDKWeaponAttributeSet* WeaponAttributeSet = Cast<UDKWeaponAttributeSet>(GetAttributeSet());
	WeaponAttributeSet->SetCurAmmo(CurAmmo);
	WeaponAttributeSet->OnChangeValueWeaponAttributes.Broadcast(CurAmmo);
	//UE_LOG(LogTemp,Warning,TEXT("%f"),WeaponAttributeSet->GetCurAmmo());
}

void AWeaponBase::Attack()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation());
	}
}

void AWeaponBase::OnAttackCompleted()
{
	// 기본적으로는 아무것도 하지 않음
	// ForkWeapon에서 오버라이드하여 사용
}
