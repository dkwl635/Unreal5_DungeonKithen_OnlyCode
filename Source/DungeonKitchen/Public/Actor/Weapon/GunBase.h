// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Actor/Weapon/WeaponBase.h"
#include "GunBase.generated.h"

UCLASS()
class DUNGEONKITCHEN_API AGunBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    AGunBase();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Attack() override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Reload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Recoil();

	UPROPERTY(EditDefaultsOnly, Category = "CameraShake")
	TSubclassOf<class UCameraShakeBase> FireShake;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<class ADKEffectActor> BulletClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil Movement")
	float RecoilKickback = 50.0f;        // 뒤로 밀리는 거리

	UPROPERTY(EditDefaultsOnly, Category = "Recoil Movement")
	float RecoilUpward = 30.0f;          // 위로 올라가는 거리

	UPROPERTY(EditDefaultsOnly, Category = "Recoil Movement")
	float RecoilRight = 0.0f;       // 좌우로 흔들리는 거리

	UPROPERTY(EditDefaultsOnly, Category = "Recoil Movement")
	float RecoilRecoverySpeed = 8.0f;    // 원래 위치로 돌아가는 속도

	void ApplyWeaponRecoilMovement();

	void RecoverFromRecoil();

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	FVector MuzzleFlashScale = FVector(1.0f, 1.0f, 1.0f); // MuzzleFlash 스케일

	TObjectPtr<class UNiagaraComponent> MuzzleFlashComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> HitEffect;

	TObjectPtr<class UNiagaraComponent> HitFXComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> TrailEffect;

	TObjectPtr<class UNiagaraComponent> TrailFXComponent;

	void PlayMuzzleFlashEffect(FVector MuzzleLocation, FRotator MuzzleRotation);

	void PlayTrailEffect(FVector StartLocation, FVector EndLocation);

	virtual void PlayHitEffect(FVector HitLocation, FRotator HitRotation);
private:
    FVector OriginalLocation;
    FVector RecoilOffset;
    bool bIsRecoiling = false;
    FTimerHandle RecoilRecoveryTimer;
};
