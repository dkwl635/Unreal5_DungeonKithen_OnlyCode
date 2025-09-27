// Copyright © 2025 Tartare Studio

#include "Actor/Weapon/ForkWeapon.h"
#include "Actor/Weapon/WeaponBase.h"
#include "Actor/DKEffectActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AForkWeapon::AForkWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // Set default values for fork weapon
    ThrowForce = 1500.0f;
    ForkDamage = 25.0f;
    AttackCooldown = 0.5f;
    ForkSocketName = TEXT("ForkSocket");
}

void AForkWeapon::BeginPlay()
{
    Super::BeginPlay();

    // Initialize attack cooldown
    bCanAttack = true;

    // Set initial position to ready position
    if (WeaponMesh)
    {
        WeaponMesh->SetRelativeLocation(ReadyPosition);
        OriginalRelativeLocation = ReadyPosition;
    }
}

void AForkWeapon::Attack()
{
    // This will be called on mouse press
    OnMousePressed();
}

void AForkWeapon::OnAttackCompleted()
{
    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: OnAttackCompleted called"));

    // This will be called on mouse release
    OnMouseReleased();
}

void AForkWeapon::OnMousePressed()
{
    if (!bCanAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForkWeapon: Cannot attack - cooldown active"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Mouse pressed - starting charge"));

    // If fork is hidden (from previous throw), make it visible immediately
    if (WeaponMesh && !WeaponMesh->IsVisible())
    {
        UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Fork was hidden, making visible immediately"));
        WeaponMesh->SetVisibility(true);
        WeaponMesh->SetRelativeLocation(ReadyPosition);
    }
    else if (WeaponMesh)
    {
        // Always start from ReadyPosition for visual feedback
        WeaponMesh->SetRelativeLocation(ReadyPosition);
    }

    // Start charging the throw (move to throw position)
    bIsCharging = true;
    StartLerpToThrowPosition();
}

void AForkWeapon::OnMouseReleased()
{
    if (!bCanAttack || !bIsCharging)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForkWeapon: Cannot release - not charging or cooldown active"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Mouse released - executing throw"));

    // Execute the throw
    ExecuteThrow();
}

void AForkWeapon::ThrowFork()
{
    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: ThrowFork called"));

    if (!BulletClass || !WeaponMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForkWeapon: BulletClass or WeaponMesh is null"));
        return;
    }

    // Get spawn location and rotation
    FVector SpawnLocation = WeaponMesh->GetSocketLocation(ForkSocketName);

	ADKPlayerCharacter* player = Cast<ADKPlayerCharacter>(GetOwner());
	FVector destloc = player->FollowCamera->GetComponentLocation() + player->FollowCamera->GetForwardVector()*1000;

    FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, destloc);
    //FRotator SpawnRotation = WeaponMesh->GetSocketRotation(ForkSocketName);

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Spawning projectile at location %s"), *SpawnLocation.ToString());

    // Spawn fork projectile
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    ADKEffectActor* ForkProjectile = GetWorld()->SpawnActor<ADKEffectActor>(
        BulletClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (ForkProjectile)
    {
        UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Projectile spawned successfully"));

        // Copy the original fork mesh to the projectile
        USkeletalMeshComponent* ProjectileMesh = ForkProjectile->FindComponentByClass<USkeletalMeshComponent>();
        if (!ProjectileMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Creating new SkeletalMeshComponent for projectile"));
            ProjectileMesh = NewObject<USkeletalMeshComponent>(ForkProjectile);
            ForkProjectile->AddInstanceComponent(ProjectileMesh);
            ProjectileMesh->RegisterComponent();
        }

        if (ProjectileMesh && WeaponMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Copying mesh to projectile"));

            // Copy the skeletal mesh from the weapon
            ProjectileMesh->SetSkeletalMesh(WeaponMesh->GetSkeletalMeshAsset());
            ProjectileMesh->SetMaterial(0, WeaponMesh->GetMaterial(0));

            // Set projectile scale to match weapon's world scale with additional scaling
            FVector WeaponWorldScale = WeaponMesh->GetComponentScale();
            FVector FinalScale = WeaponWorldScale * ProjectileScale;
            ProjectileMesh->SetWorldScale3D(FinalScale);

            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Weapon scale: %s, Projectile scale: %s, Final scale: %s"),
                *WeaponWorldScale.ToString(), *ProjectileScale.ToString(), *FinalScale.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ForkWeapon: WeaponMesh not found"));
        }

        // Add projectile movement component if it doesn't exist
        UProjectileMovementComponent* ProjectileMovement = ForkProjectile->FindComponentByClass<UProjectileMovementComponent>();
        if (!ProjectileMovement)
        {
            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Adding ProjectileMovementComponent"));
            ProjectileMovement = NewObject<UProjectileMovementComponent>(ForkProjectile);
            ForkProjectile->AddInstanceComponent(ProjectileMovement);
        }

        // Set projectile properties
        if (ProjectileMovement)
        {
            ProjectileMovement->InitialSpeed = ThrowForce;
            ProjectileMovement->MaxSpeed = ThrowForce;
            ProjectileMovement->ProjectileGravityScale = 0.5f; // Reduced gravity for fork
            ProjectileMovement->bRotationFollowsVelocity = true; // Fork rotates as it flies

            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Projectile movement configured"));
        }

        // Set damage with multiplier
        float FinalDamage = ForkDamage * CurrentDamageMultiplier;
        // TODO: Set damage on projectile when DKEffectActor is compiled
        // ForkProjectile->Damage = FinalDamage;
        UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Threw fork with damage %f (base: %f, multiplier: %.2f)"),
            FinalDamage, ForkDamage, CurrentDamageMultiplier);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ForkWeapon: Failed to spawn projectile"));
    }
}

void AForkWeapon::ThrowForkSequence()
{
    if (!WeaponMesh)
    {
        return;
    }

    // Store original position
    OriginalRelativeLocation = WeaponMesh->GetRelativeLocation();

    // Start lerp animation to throw position
    StartLerpToThrowPosition();
}

void AForkWeapon::StartLerpToThrowPosition()
{
    if (!WeaponMesh)
    {
        return;
    }

    // Set up lerp variables
    LerpStartLocation = WeaponMesh->GetRelativeLocation();
    LerpTargetLocation = ThrowPosition;
    LerpAlpha = 0.0f;

    // Use faster animation for rapid fire
    float AnimationTime = bAtThrowPose ? RapidFireAnimationTime : ThrowAnimationTime;
    LerpDuration = AnimationTime * 0.5f; // Half the time for moving to throw position

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Starting lerp with duration %f (rapid fire: %s)"),
        LerpDuration, bAtThrowPose ? TEXT("true") : TEXT("false"));

    // Enable tick for lerp animation
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // ~60 FPS

    // Start lerp timer
    GetWorldTimerManager().SetTimer(LerpTimer, this, &AForkWeapon::UpdateLerpAnimation, 0.016f, true);
}

void AForkWeapon::UpdateLerpAnimation()
{
    if (!WeaponMesh)
    {
        return;
    }

    // Update lerp alpha
    LerpAlpha += 0.016f / LerpDuration;

    if (LerpAlpha >= 1.0f)
    {
        // Lerp complete: sit at ThrowPosition and wait for release
        LerpAlpha = 1.0f;
        WeaponMesh->SetRelativeLocation(LerpTargetLocation);

        // Stop lerp. Do NOT auto-throw here.
        GetWorldTimerManager().ClearTimer(LerpTimer);
        PrimaryActorTick.bCanEverTick = false;

        // Mark we are at the pose and start shake if still charging
        bAtThrowPose = true;
        if (bIsCharging)
        {
            StartShakeAtThrowPose();
        }
    }
    else
    {
        // Continue lerp
        FVector NewLocation = FMath::Lerp(LerpStartLocation, LerpTargetLocation, LerpAlpha);
        WeaponMesh->SetRelativeLocation(NewLocation);
    }
}

void AForkWeapon::StartShakeAtThrowPose()
{
    if (!WeaponMesh)
    {
        return;
    }
    // Cache base pose
    BaseThrowLocation = WeaponMesh->GetRelativeLocation();
    BaseThrowRotation = WeaponMesh->GetRelativeRotation();
    ShakeTime = 0.0f;
    ShakeStartTime = GetWorld()->GetTimeSeconds(); // 진동 시작 시간 기록
    CurrentDamageMultiplier = 1.0f; // 데미지 배수 초기화

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Started shake at time %f"), ShakeStartTime);

    // Start timer update ~60 FPS
    const float Interval = 1.0f / 60.0f;
    GetWorldTimerManager().SetTimer(ShakeTimer, this, &AForkWeapon::UpdateShake, Interval, true);
}

void AForkWeapon::StopShake()
{
    GetWorldTimerManager().ClearTimer(ShakeTimer);
    if (WeaponMesh)
    {
        // Reset to base pose
        WeaponMesh->SetRelativeLocation(BaseThrowLocation);
        WeaponMesh->SetRelativeRotation(BaseThrowRotation);
    }
    bAtThrowPose = false;
}

void AForkWeapon::UpdateShake()
{
    if (!WeaponMesh || !bIsCharging || !bAtThrowPose)
    {
        StopShake();
        return;
    }

    // Calculate time since shake started
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceShakeStart = CurrentTime - ShakeStartTime;

    // Advance shake time
    const float Dt = 1.0f / 60.0f;
    ShakeTime += Dt;

    // Calculate shake intensity based on time
    float ShakeIntensity = 0.0f;

    if (TimeSinceShakeStart < ShakeDelayTime)
    {
        // 0~1초: 진동 없음
        ShakeIntensity = 0.0f;
    }
    else if (TimeSinceShakeStart < ShakeDelayTime + ShakeRampTime)
    {
        // 1~3초: 진동이 점진적으로 커짐
        float RampProgress = (TimeSinceShakeStart - ShakeDelayTime) / ShakeRampTime;
        ShakeIntensity = FMath::Clamp(RampProgress, 0.0f, 1.0f);
    }
    else
    {
        // 3초 이후: 최대 진동
        ShakeIntensity = 1.0f;
    }

    // Angular frequency
    const float W = 2.0f * PI * ShakeFrequency;

    // Compute small offsets
    const float sx = FMath::Sin(W * ShakeTime);
    const float sy = FMath::Cos(W * 1.31f * ShakeTime);
    const float sz = FMath::Sin(W * 0.77f * ShakeTime);

    // Apply intensity to amplitude
    FVector PosOffset = FVector(sx, sy, sz) * ShakeAmplitudePos * ShakeIntensity;
    FRotator RotOffset = FRotator(sx, sy, 0.0f) * ShakeAmplitudeRot * ShakeIntensity;

    WeaponMesh->SetRelativeLocation(BaseThrowLocation + PosOffset);
    WeaponMesh->SetRelativeRotation((BaseThrowRotation + RotOffset).Quaternion());

    // Update damage multiplier based on time
    if (TimeSinceShakeStart >= ShakeDelayTime)
    {
        // 1초 이후부터 데미지 증가 시작
        float DamageTime = TimeSinceShakeStart - ShakeDelayTime;
        int DamageSteps = FMath::FloorToInt(DamageTime / DamageIncreaseInterval);
        float NewDamageMultiplier = 1.0f + (DamageSteps * DamageIncreaseAmount / 100.0f);

        // 데미지 배수가 변경되었을 때만 로그 출력
        if (NewDamageMultiplier != CurrentDamageMultiplier)
        {
            CurrentDamageMultiplier = NewDamageMultiplier;
            UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Damage multiplier increased to %.2f"), CurrentDamageMultiplier);
        }
    }

    // Debug log every second
    static float LastLogTime = 0.0f;
    if (CurrentTime - LastLogTime >= 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Shake time: %.1f, Intensity: %.2f, Damage: %.2f"),
            TimeSinceShakeStart, ShakeIntensity, CurrentDamageMultiplier);
        LastLogTime = CurrentTime;
    }
}

void AForkWeapon::ExecuteThrow()
{
    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: ExecuteThrow called"));

    // Stop charging
    bIsCharging = false;

    // Stop any ongoing lerp and shake
    GetWorldTimerManager().ClearTimer(LerpTimer);
    StopShake();
    PrimaryActorTick.bCanEverTick = false;

    // Ensure we're at the throw pose before throwing
    if (WeaponMesh)
    {
        WeaponMesh->SetRelativeLocation(ThrowPosition);
        WeaponMesh->SetRelativeRotation(BaseThrowRotation); // keep orientation

        // Hide the fork mesh while projectile is in flight
        WeaponMesh->SetVisibility(false);
    }

    // Spawn fork projectile now
    ThrowFork();

    // Schedule mesh reappearance at ReadyPosition with faster time for rapid fire
    float ReturnTime = bAtThrowPose ? RapidFireReturnTime : ForkReturnTime;
    GetWorldTimerManager().SetTimer(ForkReturnTimer, this, &AForkWeapon::OnForkReturn, ReturnTime, false);

    UE_LOG(LogTemp, Log, TEXT("ForkWeapon: Scheduled return in %f seconds (rapid fire: %s)"),
        ReturnTime, bAtThrowPose ? TEXT("true") : TEXT("false"));

    // Start cooldown
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &AForkWeapon::ResetAttackCooldown, AttackCooldown, false);
}

void AForkWeapon::StartReappearFromBehind()
{
    if (!WeaponMesh)
    {
        return;
    }

    // Initialize reappear state
    ReappearAlpha = 0.0f;
    ReappearStartLocation = ReadyPosition + ReappearOffset;

    // Place mesh at start (behind) and make visible
    WeaponMesh->SetRelativeLocation(ReappearStartLocation);
    WeaponMesh->SetVisibility(true);

    // Tick reappear at ~60 FPS
    const float Interval = 1.0f / 60.0f;
    GetWorldTimerManager().SetTimer(ReappearTimer, this, &AForkWeapon::UpdateReappear, Interval, true);
}

static float EaseOutCubic(float t)
{
    float inv = 1.f - t;
    return 1.f - inv*inv*inv;
}

void AForkWeapon::UpdateReappear()
{
    if (!WeaponMesh)
    {
        GetWorldTimerManager().ClearTimer(ReappearTimer);
        return;
    }

    const float Dt = 1.0f / 60.0f;
    ReappearAlpha += Dt / ReappearTime;
    if (ReappearAlpha >= 1.0f)
    {
        ReappearAlpha = 1.0f;
    }

    const float a = EaseOutCubic(ReappearAlpha);

    const FVector NewLoc = FMath::Lerp(ReappearStartLocation, ReadyPosition, a);

    WeaponMesh->SetRelativeLocation(NewLoc);

    if (ReappearAlpha >= 1.0f)
    {
        // Done
        GetWorldTimerManager().ClearTimer(ReappearTimer);
        WeaponMesh->SetRelativeLocation(ReadyPosition);
    }
}

void AForkWeapon::OnForkReturn()
{
    if (!WeaponMesh)
    {
        return;
    }

    // Start reappear-from-behind animation instead of popping at ReadyPosition
    StartReappearFromBehind();
}

void AForkWeapon::ResetAttackCooldown()
{
    bCanAttack = true;
}
