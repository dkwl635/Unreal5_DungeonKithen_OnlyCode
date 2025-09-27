// Copyright © 2025 Tartare Studio

#include "Actor/Weapon/GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/DKWeaponAttributeSet.h"

AGunBase::AGunBase()
{
    MuzzleSocketName = "Muzzle";
}

void AGunBase::Attack()
{
	Super::Attack();
	//UE_LOG(LogTemp, Warning, TEXT("AGunBase::Attack() called"));

	if (!WeaponMesh)
	{
		//UE_LOG(LogTemp, Error, TEXT("WeaponMesh is null!"));
		return;
	}

	if (!WeaponMesh->GetSkeletalMeshAsset())
	{
		//UE_LOG(LogTemp, Error, TEXT("SkeletalMesh is not set!"));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("WeaponMesh and SkeletalMesh are valid"));

	// 소켓이 존재하는지 확인
	if (WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
		//UE_LOG(LogTemp, Warning, TEXT("Socket found: %s at location: %s"), *MuzzleSocketName.ToString(), *MuzzleLocation.ToString());
		PlayMuzzleFlashEffect(MuzzleLocation, MuzzleRotation);
	}
	else
	{
		// 소켓이 없으면 무기 위치 + 오프셋 사용
		FVector MuzzleLocation = WeaponMesh->GetComponentLocation() + WeaponMesh->GetForwardVector() * 100.0f;
		FRotator MuzzleRotation = WeaponMesh->GetComponentRotation();
		//UE_LOG(LogTemp, Warning, TEXT("Socket not found: %s, using fallback location: %s"), *MuzzleSocketName.ToString(), *MuzzleLocation.ToString());
		PlayMuzzleFlashEffect(MuzzleLocation, MuzzleRotation);
	}

	//반동은 총이 발사되고 나서 작동해야하는데
	//Recoil();
}

void AGunBase::Reload()
{
	if (CurAmmo == MaxAmmo) return;
    CurAmmo = MaxAmmo;
	SetCurAmmo();
}

void AGunBase::Recoil()
{

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		if (FireShake) PC->PlayerCameraManager->StartCameraShake(FireShake);
	}
    ApplyWeaponRecoilMovement();
}

void AGunBase::ApplyWeaponRecoilMovement()
{
    if (bIsRecoiling) return; // 이미 반동 중이면 무시

	ADKPlayerCharacter* DKOwner = Cast<ADKPlayerCharacter>(GetOwner());

	auto WeaponPosition = DKOwner->WeaponPos;
    // 원래 위치 저장
    OriginalLocation = WeaponPosition->GetRelativeLocation();

    // 반동 방향 계산 (뒤로 + 위로)
    FVector RecoilDirection = FVector(-1, 0, 0) * RecoilKickback + DKOwner->GetActorUpVector() * RecoilUpward + DKOwner->GetActorRightVector() * RecoilRight;

    // 반동 위치로 이동
    FVector RecoilLocation = OriginalLocation + RecoilDirection;
    WeaponPosition->SetRelativeLocation(RecoilLocation);

    // 반동 상태 설정
    bIsRecoiling = true;
    RecoilOffset = RecoilDirection;

    // 원래 위치로 돌아가는 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(RecoilRecoveryTimer, this, &AGunBase::RecoverFromRecoil, 0.016f, true);
}

void AGunBase::RecoverFromRecoil()
{
    if (!bIsRecoiling) return;

	ADKPlayerCharacter* DKOwner = Cast<ADKPlayerCharacter>(GetOwner());

    FVector CurrentLocation = DKOwner->WeaponPos->GetRelativeLocation();
    FVector TargetLocation = OriginalLocation;

    // 부드럽게 원래 위치로 이동
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation,
                                          GetWorld()->GetDeltaSeconds(), RecoilRecoverySpeed);

	DKOwner->WeaponPos->SetRelativeLocation(NewLocation);

    // 충분히 가까워지면 복구 완료
    if (FVector::Dist(NewLocation, TargetLocation) < 1.0f)
    {
    	DKOwner->WeaponPos->SetRelativeLocation(TargetLocation);
        bIsRecoiling = false;
        GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
    }
}

void AGunBase::PlayMuzzleFlashEffect(FVector MuzzleLocation, FRotator MuzzleRotation)
{
	if (!MuzzleFlashEffect)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MuzzleFlashEffect is null!"));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Creating MuzzleFlash at socket: %s"), *MuzzleSocketName.ToString());

	// 기존 컴포넌트가 있으면 제거
	//if (MuzzleFlashComponent->IsValidLowLevel())
	//{
	//	MuzzleFlashComponent->DestroyComponent();
	//	MuzzleFlashComponent = nullptr;
	//}

	// SkeletalMesh의 소켓에 직접 붙이는 컴포넌트 생성
	MuzzleFlashComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		MuzzleFlashEffect,
		WeaponMesh,
		MuzzleSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		MuzzleFlashScale, // 에디터에서 설정 가능한 스케일
		EAttachLocation::SnapToTarget,
		true,
		ENCPoolMethod::None, // PoolMethod
		true, // bPreCullCheck
		true // bAutoDestroy
	);

	if (MuzzleFlashComponent)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MuzzleFlashComponent created successfully!"));
		//UE_LOG(LogTemp, Warning, TEXT("Component location: %s"), *MuzzleFlashComponent->GetComponentLocation().ToString());
		//UE_LOG(LogTemp, Warning, TEXT("Component active: %s"), MuzzleFlashComponent->IsActive() ? TEXT("Yes") : TEXT("No"));
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("Failed to create MuzzleFlashComponent!"));
	}
}

void AGunBase::PlayTrailEffect(FVector StartLocation, FVector EndLocation)
{
	if (!TrailEffect) return;

	//TrailFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TrailEffect, StartLocation);
	TrailFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		TrailEffect,
		WeaponMesh,
		MuzzleSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		MuzzleFlashScale, // 에디터에서 설정 가능한 스케일
		EAttachLocation::SnapToTarget,
		true,
		ENCPoolMethod::None, // PoolMethod
		true, // bPreCullCheck
		true // bAutoDestroy
	);
	if (TrailFXComponent)
	{
		//TrailFXComponent->SetNiagaraVariableVec3(TEXT("EndLocation"), EndLocation);
		TrailFXComponent->SetVariableVec3(TEXT("EndLocation"), EndLocation);
	}
}

void AGunBase::PlayHitEffect(FVector HitLocation, FRotator HitRotation)
{
	if (!HitEffect) return;
	HitFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, HitLocation, HitRotation);
}
