#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawnRow.h"
#include "WaveInfoRow.h"
#include "SpawnVolume.generated.h"

class UBoxComponent;
class ASpike;
class AMineItem;

UCLASS()
class SPARTAPROJECT_API ASpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ASpawnVolume();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnRandomItems(int32 SpawnAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 GetSpawnedCoinAmount() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* ItemDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	UDataTable* WaveInfoTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	TSubclassOf<ASpike> SpikeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	TSubclassOf<AMineItem> MineItemClass;

	FVector GetRandomPointInVolume() const;
	FVector GetGroundLocation(const FVector& Location) const;
	
	FItemSpawnRow* GetRandomItem() const;
	FWaveInfoRow* GetWaveInfoRow(int32 waveIndex) const;
	
	// 아이템 스폰 / 제거
	AActor* SpawnItem(TSubclassOf<AActor> ItemClass);
	void ClearSpawnedActors();
	
	// 스파이크 생성 / 재배치
	void SpawnSpike(int32 SpawnAmount);
	int32 ReplaceSpike(int32 waveIndex);
	
	// 랜덤 생성 폭탄 스폰 시작
	void StartSpawnMine(int32 waveIndex);
	void SpawnMine(int32 SpawnAmount);
	
	UPROPERTY()
	TArray<AActor*> SpawnedActors;
	
	UPROPERTY()
	TArray<ASpike*> SpawnedSpikes;
	
	UPROPERTY()
	TArray<AMineItem*> SpawnedMines;
	
private:
	int32 SpawnedCoinAmount;
	FTimerHandle SpawnMineTimerHandle;
};