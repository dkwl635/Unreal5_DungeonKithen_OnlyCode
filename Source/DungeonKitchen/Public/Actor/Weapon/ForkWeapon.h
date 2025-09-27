// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Actor/Weapon/GunBase.h"
#include "ForkWeapon.generated.h"

UCLASS()
class DUNGEONKITCHEN_API AForkWeapon : public AGunBase
{
    GENERATED_BODY()

public:
    AForkWeapon();

    virtual void Attack() override;

    virtual void OnAttackCompleted() override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void ThrowFork();

    // Mouse press/release functions
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void OnMousePressed();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void OnMouseReleased();

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float RapidFireAnimationTime = 0.1f;  // 연타 시 애니메이션 시간

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float ThrowForce = 3000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float ForkDamage = 25.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    TSubclassOf<class ADKEffectActor> ForkProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    FName ForkSocketName = TEXT("ForkSocket");

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float AttackCooldown = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    FVector ReadyPosition = FVector(50, 30, -20);  // 오른쪽 아래

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon") 
    FVector ThrowPosition = FVector(50, 30, 20);   // 오른쪽 위

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float ThrowAnimationTime = 0.2f;  // 던지기 자세 시간

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float ForkReturnTime = 0.3f;  // 포크 복귀 시간 (연타를 위해 단축)

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    float RapidFireReturnTime = 0.1f;  // 연타 시 포크 복귀 시간

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon")
    FVector ProjectileScale = FVector(1.0f, 1.0f, 1.0f);  // 프로젝타일 스케일

    float ShakeStartTime = 0.0f;    // 진동 시작 시간
    float CurrentDamageMultiplier = 1.0f;  // 현재 데미지 배수

protected:
    // ... existing code ...

private:
    FTimerHandle AttackCooldownTimer;
    FTimerHandle ThrowAnimationTimer;
    FTimerHandle ForkReturnTimer;
    FTimerHandle LerpTimer;
    FTimerHandle ShakeTimer;
    FTimerHandle ReappearTimer;
    bool bCanAttack = true;
    bool bIsCharging = false;
    bool bAtThrowPose = false;
    FVector OriginalRelativeLocation;
    
    // Lerp animation variables
    FVector LerpStartLocation;
    FVector LerpTargetLocation;
    float LerpAlpha;
    float LerpDuration;

    // Shake variables
    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Shake")
    float ShakeAmplitudePos = 1.0f; // uu (최대 진동 폭)

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Shake")
    float ShakeAmplitudeRot = 1.0f; // degrees (최대 진동 폭)

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Shake")
    float ShakeFrequency = 12.0f;   // Hz

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Shake")
    float ShakeDelayTime = 0.7f;    // 진동 시작 전 대기 시간

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Shake")
    float ShakeRampTime = 2.0f;     // 진동이 최대로 커지는 시간 (1초~3초)

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Damage")
    float DamageIncreaseInterval = 0.5f;  // 데미지 증가 간격 (초)

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Damage")
    float DamageIncreaseAmount = 1.0f;   // 데미지 증가량 (%)

    FVector BaseThrowLocation;
    FRotator BaseThrowRotation;
    float ShakeTime = 0.0f;

    // Reappear animation variables
    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Reappear")
    FVector ReappearOffset = FVector(-10.f, 0.f, -2.f);

    UPROPERTY(EditDefaultsOnly, Category = "Fork Weapon|Reappear")
    float ReappearTime = 0.18f;

    float ReappearAlpha = 0.0f;
    FVector ReappearStartLocation;

    UFUNCTION()
    void ResetAttackCooldown();

    UFUNCTION()
    void ThrowForkSequence();

    UFUNCTION()
    void StartLerpToThrowPosition();

    UFUNCTION()
    void UpdateLerpAnimation();

    UFUNCTION()
    void ExecuteThrow();

    UFUNCTION()
    void StartShakeAtThrowPose();

    UFUNCTION()
    void StopShake();

    UFUNCTION()
    void UpdateShake();

    UFUNCTION()
    void StartReappearFromBehind();

    UFUNCTION()
    void UpdateReappear();

    UFUNCTION()
    void OnForkReturn();
};
