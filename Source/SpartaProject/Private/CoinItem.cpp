#include "CoinItem.h"
#include "Engine/World.h"
#include "SpartaGameState.h"

ACoinItem::ACoinItem()
{
	PointValue = 0;
	ItemType = "DefaultCoin";
	
	RotateTimerInterval = 0.1f;
	RotationSpeed = 10.f;
}

void ACoinItem::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(
		RotateTimerHandle,
		this,
		&ACoinItem::RotateCoin,
		RotateTimerInterval,
		true
	);

}

void ACoinItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearTimer(RotateTimerHandle);
}

void ACoinItem::ActivateInteractableObject(AActor* Activator)
{
	Super::ActivateInteractableObject(Activator);
	
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (UWorld* World = GetWorld())
		{
			if (ASpartaGameState* GameState = World->GetGameState<ASpartaGameState>())
			{
				GameState->AddScore(PointValue);
				GameState->OnCoinCollected();
			}
		}
		
		DestroyItem();
	}
}

void ACoinItem::RotateCoin()
{
	AddActorWorldRotation(FRotator(0.f, RotationSpeed, 0.f));
}
