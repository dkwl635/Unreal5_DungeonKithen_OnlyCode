// Copyright © 2025 Tartare Studio


#include "Actor/Weapon/Gun/Skewer.h"

#include "AbilitySystem/DKWeaponAttributeSet.h"
#include "Actor/DKEffectActor.h"

void ASkewer::Attack()
{
	Super::Attack();
	FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);

	UDKWeaponAttributeSet* WeaponAttributeSet = Cast<UDKWeaponAttributeSet>(GetAttributeSet());

	if (bATK1)
	{
		for (int i=0; i<WeaponAttributeSet->GetRounds(); ++i)
		{
			FRotator rot = MuzzleRotation;
			rot.Yaw += (i-1)*Angle;
			if (BulletClass) GetWorld()->SpawnActor(BulletClass, &MuzzleLocation, &rot);
		}
	}
	else
	{
		// 3연 발사
		/*GetWorldTimerManager().SetTimer(SkewerTimerHandle, FTimerDelegate::CreateLambda(
			[this, WeaponAttributeSet, MuzzleLocation, MuzzleRotation]()
			{
				if (BulletClass) GetWorld()->SpawnActor(BulletClass, &MuzzleLocation, &MuzzleRotation);
				++RoundCount;

				if (RoundCount >= WeaponAttributeSet->GetRounds())
				{
					RoundCount = 0;
					GetWorldTimerManager().ClearTimer(SkewerTimerHandle);
				}

			}), 0.1, true);*/

		// 폭발
		if (BulletClass) GetWorld()->SpawnActor(BulletClass, &MuzzleLocation, &MuzzleRotation);
	}

	Recoil();
}
