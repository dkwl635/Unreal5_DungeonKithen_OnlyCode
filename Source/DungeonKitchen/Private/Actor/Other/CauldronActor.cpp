// Copyright © 2025 Tartare Studio


#include "Actor/Other/CauldronActor.h"
#include "Components/BoxComponent.h"
#include "Game/DKPlayerController.h"
#include "Kismet/GameplayStatics.h"


ACauldronActor::ACauldronActor()
{

	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(BoxComp);
	BoxComp->SetCollisionProfileName(TEXT("Item"));
	BoxComp->InitBoxExtent(FVector(100.f, 100.f, 50.f));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
}


void ACauldronActor::BeginPlay()
{
	Super::BeginPlay();

	SetShow(bShow);

}


void ACauldronActor::Interact()
{
	// 열린거면 리턴
	//if (bIsOpen) return;
	// 닫혀있으면 열려있게
	//if (!bIsOpen) bIsOpen = true;

	ADKPlayerController* PC = Cast<ADKPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (PC)
	{
		PC->ToggleCookingTest();
	}
}

void ACauldronActor::SetShow(bool bShows)
{
	BoxComp->SetCollisionEnabled( bShows ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision );
	MeshComp->SetVisibility(bShows);
}

