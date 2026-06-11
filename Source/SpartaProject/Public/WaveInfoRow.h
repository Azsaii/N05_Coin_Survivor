#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WaveInfoRow.generated.h"

USTRUCT(BlueprintType)
struct FWaveInfoRow : public FTableRowBase
{
	GENERATED_BODY()
	
	// 웨이브 제한시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveDuration;
	
	// 웨이브 아이템 스폰 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaveItemSpawnAmount;
	
	// 스파이크 스폰 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaveSpikeSpawnAmount;
	
	// 스파이크 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveSpikeMoveSpeed;
	
	// 스파이크 이동 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveSpikeMaxMoveRange;
	
	// 랜덤 생성 폭탄 생성 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnMineInterval;
	
	// 랜덤 생성 폭탄 동시 생성 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnMineAmount;
};