// Copyright © 2025 Tartare Studio


#include "Character/DKCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "PaperSpriteComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Data/ItemDataStructures.h"
#include "DishEffect/DishEffectBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"


ADKCharacterBase::ADKCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MinimapSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MinimapSprite"));
	MinimapSprite->SetupAttachment(RootComponent);
	MinimapSprite->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 200.0f), FRotator(0.f,90.f,-90.f));
	MinimapSprite->bVisibleInSceneCaptureOnly = true;
}

UAbilitySystemComponent* ADKCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADKCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	ADKPlayerCharacter* player = Cast<ADKPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
	if(player)
	{
		if (MinimapSprite) player->SceneCapture->ShowOnlyComponent(MinimapSprite);
	}


	InitMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (GetAttributeSet())
	{
		Cast<UDKCharacterAttributeSet>(GetAttributeSet())->OnChangeValueAttributes.AddDynamic(this, &ADKCharacterBase::SetChangeAttribute);
	}

}

void ADKCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	EffectDishTick(DeltaSeconds);
}

void ADKCharacterBase::InitAbilityActorInfo()
{
}

void ADKCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	if(!GameplayEffectClass) return;
	const FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ADKCharacterBase::ApplyEffectToWeapon(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{



}

void ADKCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1);
	ApplyEffectToWeapon(EffectWeaponAttributes, 1);
}

void ADKCharacterBase::SetChangeAttribute(const FGameplayAttribute& GameplayAttributeData, float Value)
{

	if (GameplayAttributeData == UDKCharacterAttributeSet::GetMoveSpeedPctAttribute())
	{
		float NewSpeed = InitMoveSpeed *Value;
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}

}

void ADKCharacterBase::PlayWalkSound()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WalkSound, GetActorLocation());
	CurSoundTime = 0;
}

void ADKCharacterBase::EffectDishTick(float DeltaSeconds)
{
	TArray<UDishEffectBase*> EndEffects;

	for (auto DishEffect : DishEffects)
	{
		if (DishEffect->TickEffect(DeltaSeconds))
		{
			EndEffects.Add(DishEffect);
		}
	}

	for (auto DishEffect : EndEffects)
	{
		DishEffects.Remove(DishEffect);
	}

}

void ADKCharacterBase::AddDishEffect(UDishEffectBase* DishEffect)
{
	DishEffect->StartEffect(this);
	DishEffects.Add(DishEffect);
}

void ADKCharacterBase::RemoveDishEffect(UDKDish* TargetDish)
{
	TArray<UDishEffectBase*> EndEffects;

	for (auto DishEffect : DishEffects)
	{
		if (DishEffect->EffectMakerDish == TargetDish)
		{
			DishEffect->EndEffect();
			EndEffects.Add(DishEffect);
		}
	}

	for (auto DishEffect : EndEffects)
	{
		DishEffects.Remove(DishEffect);
	}
}


bool ADKCharacterBase::CheckDishEffect(class UDKDish* TargetDish) const
{
	//이미 적용 중인지
	for (auto DishEffect : DishEffects)
	{
		if (DishEffect->EffectMakerDish == TargetDish)
		{
			DishEffect->ResetEffect();
			return  true;
		}
	}

	return  false;
}


