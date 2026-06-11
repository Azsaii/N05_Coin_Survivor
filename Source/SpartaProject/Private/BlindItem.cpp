#include "BlindItem.h"
#include "SpartaCharacter.h"

ABlindItem::ABlindItem()
{
	Duration = 3.0f;
	ItemType = "Blind";
}

void ABlindItem::ActivateInteractableObject(AActor* Activator)
{
	Super::ActivateInteractableObject(Activator);
	
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ApplyBlindDebuff(Duration);
		}
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

