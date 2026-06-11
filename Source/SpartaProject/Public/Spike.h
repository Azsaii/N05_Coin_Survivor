// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObjectInterface.h"
#include "GameFramework/Actor.h"
#include "WaveInfoRow.h"
#include "Spike.generated.h"

class USphereComponent;

UCLASS()
class SPARTAPROJECT_API ASpike : public AActor, public IInteractableObjectInterface {
	GENERATED_BODY()

public:
	ASpike();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category = "Move")
	FVector MoveDirection;
	
	UPROPERTY(EditAnywhere, Category = "Move")
	float MoveSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Move")
	float MaxMoveRange;
	
	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	float Damage;
	
	void InitSpike(FWaveInfoRow* WaveInfoRow);
	
	UFUNCTION(BlueprintCallable)
	void ActivateSpike();
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ObstacleType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USceneComponent* SceneRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USphereComponent* Collision;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* OverlapParticle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* OverlapSound;
	
	virtual void OnInteractableObjectOverlap(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult) override;
	
	virtual void OnInteractableObjectEndOverlap(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex) override;
	
	virtual void ActivateInteractableObject(AActor* Activator) override;
	virtual FName GetInteractableObjectType() const override;

private:
	bool bIsActive;
	FVector StartLocation;
	float moveSign;
};
