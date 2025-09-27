// Copyright © 2025 Tartare Studio


#include "Actor/Other/ChestActor.h"

#include "Actor/Weapon/WeaponBase.h"
#include "Components/BoxComponent.h"
#include "Game/Subsystem/PoolWorldSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AChestActor::AChestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(BoxComp);
	BoxComp->SetCollisionProfileName(TEXT("Item"));

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AChestActor::BeginPlay()
{
	Super::BeginPlay();

	SetShow(bShow);
}

void AChestActor::Interact()
{
	// 열린거면 리턴
	if (bIsOpen) return;
	// 닫혀있으면 열려있게
	if (!bIsOpen) bIsOpen = true;

	//if (OpenMontage) MeshComp->PlayAnimation(OpenMontage, 0);
	PlayOpenMontage();

	// 무기 소환
	int32 WeaponAmount = FMath::RandRange(1, 2);

	for (int32 i = 0; i < WeaponAmount; ++i)
	{
		int32 index = FMath::RandRange(1, WeaponList.Num()) - 1;
		FVector loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation() + FVector(0.0, 0, 80), FVector(0, 100, 0));
		GetWorld()->SpawnActor<AWeaponBase>(WeaponList[index], loc, FRotator(0, 90, 0));
	}


	int32 ItemAmount = FMath::RandRange(1, 2);

	for (int32 i = 0; i < ItemAmount; ++i)
	{
		int32 index = FMath::RandRange(1, ItemIDList.Num()) - 1;
		int32 ItemID = ItemIDList[index];
		UPoolWorldSubsystem* Pool =	GetWorld()->GetSubsystem<UPoolWorldSubsystem>();
		if (Pool)
		{
			Pool->SpawnDropItemBox(GetActorLocation(), ItemID, 1);
		}
	}
}

void AChestActor::SetShow(bool bShows)
{
	BoxComp->SetCollisionEnabled( bShows ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision );
	MeshComp->SetVisibility(bShows);
}




