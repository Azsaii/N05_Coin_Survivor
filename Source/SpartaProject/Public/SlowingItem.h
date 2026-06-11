#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowingItem.generated.h"

UCLASS()
class SPARTAPROJECT_API ASlowingItem : public ABaseItem {
	GENERATED_BODY()

public:
	ASlowingItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Duration;
	
	virtual void ActivateInteractableObject(AActor* Activator) override;
};
