#include "BigCoinItem.h"

ABigCoinItem::ABigCoinItem()
{
	PointValue = 50;
	ItemType = "BigCoin";
}

void ABigCoinItem::ActivateInteractableObject(AActor* Activator)
{
	Super::ActivateInteractableObject(Activator);
}