#include "SlowingItem.h"
#include "SpartaCharacter.h"

ASlowingItem::ASlowingItem()
{
	SlowMultiplier = 0.5f;
	Duration = 3.0f;
	ItemType = "Slowing";
}

void ASlowingItem::ActivateInteractableObject(AActor* Activator)
{
	Super::ActivateInteractableObject(Activator);
	
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ApplySlowDebuff(SlowMultiplier, Duration);
		}

		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}