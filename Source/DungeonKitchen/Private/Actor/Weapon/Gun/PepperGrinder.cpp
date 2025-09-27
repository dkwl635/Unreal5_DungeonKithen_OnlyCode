// Copyright © 2025 Tartare Studio


#include "Actor/Weapon/Gun/PepperGrinder.h"

#include "NiagaraComponent.h"
#include "AbilitySystem/DKWeaponAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

void APepperGrinder::Attack()
{
	Super::Attack();
	FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);

	UDKWeaponAttributeSet* WeaponAttributeSet = Cast<UDKWeaponAttributeSet>(GetAttributeSet());

	for (int i=0; i<WeaponAttributeSet->GetRounds(); ++i)
	{
		FVector Rot = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetActorForwardVector(), Spread);
		FVector EndLocation = MuzzleLocation + (Rot*Range);
		FHitResult HitRes;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);

		bool hit = GetWorld()->LineTraceSingleByChannel(HitRes, MuzzleLocation, EndLocation, ECC_GameTraceChannel6, params);
		PlayTrailEffect(MuzzleLocation, EndLocation);
		//DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + (Rot*Range), FColor::Red, false, 0.5f);
		if (hit)
		{
			ApplyEffectToTarget(HitRes.GetActor(), InstantDamageGameplayEffectClass);
			FRotator rot = HitRes.ImpactNormal.Rotation();
			rot += FRotator(-90.f, 0.f, 0.f);

			// 나이아가라 재생
			PlayHitEffect(HitRes.Location, rot);
		}
	}

	Recoil();
}


