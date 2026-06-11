#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableObjectInterface : public UInterface {
	GENERATED_BODY()
};

class SPARTAPROJECT_API IInteractableObjectInterface {
	GENERATED_BODY()

protected:
	UFUNCTION()
	virtual void OnInteractableObjectOverlap(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult) = 0;
	
	UFUNCTION()
	virtual void OnInteractableObjectEndOverlap(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex) = 0;
	
	// 아이템이 사용되었을 때 호출
	virtual void ActivateInteractableObject(AActor* Activator) = 0;
	
	// 이 아이템의 유형(타입)을 반환 (예: "Coin", "Mine" 등)
	virtual FName GetInteractableObjectType() const = 0;
};
