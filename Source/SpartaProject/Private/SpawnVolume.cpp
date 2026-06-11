#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "BaseItem.h"
#include "CoinItem.h"
#include "MineItem.h"
#include "SpartaGameInstance.h"
#include "Spike.h"
#include "Components/TextBlock.h"

ASpawnVolume::ASpawnVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    SetRootComponent(Scene);
    
    SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
    SpawningBox->SetupAttachment(Scene);
    
    ItemDataTable = nullptr;
    SpawnedCoinAmount = 0;
}

int32 ASpawnVolume::GetSpawnedCoinAmount() const
{
    return SpawnedCoinAmount;
}

void ASpawnVolume::SpawnRandomItems(int32 SpawnAmount)
{
    int CoinCount = 0;
    for (int32 i = 0; i < SpawnAmount; i++)
    {
        if (FItemSpawnRow* SelectedRow = GetRandomItem())
        {
            if (UClass* ActualClass = SelectedRow->ItemClass.Get())
            {
                AActor* SpawnedActor = SpawnItem(ActualClass);
                if (SpawnedActor->IsA(ACoinItem::StaticClass()))
                {
                    CoinCount++;
                }
            }
        }
    }
    SpawnedCoinAmount = CoinCount;
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
    const FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
    const FVector BoxOrigin = SpawningBox->GetComponentLocation();

    return BoxOrigin + FVector(
        FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
        FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
    );
}

FVector ASpawnVolume::GetGroundLocation(const FVector& Location) const
{
    FHitResult Hit;
    FVector End = Location - FVector(0.f, 0.f, 5000.f);
    FCollisionQueryParams Parms;
    Parms.bTraceComplex = false;
    
    if (bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECC_Visibility, Parms))
    {
        return Hit.Location;
    }
    
    else return FVector::ZeroVector;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
    if (!ItemDataTable) return nullptr;

    // 1) 모든 Row(행) 가져오기
    TArray<FItemSpawnRow*> AllRows;
    static const FString ContextString(TEXT("ItemSpawnContext"));
    ItemDataTable->GetAllRows(ContextString, AllRows);

    if (AllRows.IsEmpty()) return nullptr;

    // 2) 전체 확률 합 구하기
	float TotalChance = 0.0f; // 초기화
	for (const FItemSpawnRow* Row : AllRows) // AllRows 배열의 각 Row를 순회
	{
		if (Row) // Row가 유효한지 확인
		{
		    TotalChance += Row->SpawnChance; // SpawnChance 값을 TotalChance에 더하기
		}
	}

    // 3) 0 ~ TotalChance 사이 랜덤 값
    const float RandValue = FMath::FRandRange(0.0f, TotalChance);
    float AccumulateChance = 0.0f;

    // 4) 누적 확률로 아이템 선택
    for (FItemSpawnRow* Row : AllRows)
    {
        AccumulateChance += Row->SpawnChance;
        if (RandValue <= AccumulateChance)
        {
            return Row;
        }
    }

    return nullptr;
}

FWaveInfoRow* ASpawnVolume::GetWaveInfoRow(int32 waveIndex) const
{
    if (!WaveInfoTable) return nullptr;
    
    // 모든 Row(행) 가져오기
    TArray<FWaveInfoRow*> AllRows;
    static const FString ContextString(TEXT("WaveInfoContext"));
    WaveInfoTable->GetAllRows(ContextString, AllRows);
    
    if (AllRows.IsEmpty()) return nullptr;
    if (AllRows.IsValidIndex(waveIndex))
    {
        return AllRows[waveIndex];
    }
    
    return nullptr;
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
    if (!ItemClass) return nullptr;
    
    FVector SpawnLocation = GetRandomPointInVolume();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
            ItemClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            Params
    );
    
    if (SpawnedActor)
    {
        SpawnedActors.Add(SpawnedActor);
    }
    
    return SpawnedActor;
}

void ASpawnVolume::SpawnSpike(int32 SpawnAmount)
{
    if (SpikeClass)
    {
        for (int32 i = 0; i < SpawnAmount; i++)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
                    SpikeClass,
                    FVector::ZeroVector,
                    FRotator::ZeroRotator,
                    Params
            );
    
            if (SpawnedActor)
            {
                SpawnedSpikes.Add(Cast<ASpike>(SpawnedActor));
            }
        }
    }
}
int32 ASpawnVolume::ReplaceSpike(int32 waveIndex)
{
    int32 SpawnAmount = 0;
    
    if (FWaveInfoRow* WaveInfoRow = GetWaveInfoRow(waveIndex))
    {
        // 부족한 만큼 생성
        SpawnAmount = WaveInfoRow->WaveSpikeSpawnAmount;
        int32 gap = SpawnAmount - SpawnedSpikes.Num();
        if (gap > 0)
        {
            SpawnSpike(gap);
        }
        
        // 모든 스파이크 랜덤 재배치
        for (ASpike* Spike : SpawnedSpikes)
        {
            Spike->SetActorLocation(GetGroundLocation(GetRandomPointInVolume()) + FVector(0.f, 0.f, 50.f));
            Spike->InitSpike(WaveInfoRow);
            Spike->ActivateSpike(); // 활성화
        }
    }
    
    return SpawnAmount;
}

void ASpawnVolume::StartSpawnMine(int32 waveIndex)
{
    if (FWaveInfoRow* WaveInfoRow = GetWaveInfoRow(waveIndex))
    {
        FTimerManager& TimerManager = GetWorldTimerManager();
        TimerManager.ClearTimer(SpawnMineTimerHandle);
        
        const float Interval = WaveInfoRow->SpawnMineInterval;
        const int32 Amount = WaveInfoRow->SpawnMineAmount;
        
        if (Interval > 0.f && Amount > 0)
        {
            TimerManager.SetTimer(
            SpawnMineTimerHandle,
            [this, Amount]()
            {
                ASpawnVolume::SpawnMine(Amount);
            }, 
            Interval, 
            true);
        }
    }
}

void ASpawnVolume::SpawnMine(int32 SpawnAmount)
{
    if (MineItemClass)
    {
        for (int32 i = 0; i < SpawnAmount; i++)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
            if (UWorld* World = GetWorld())
            {
                AActor* SpawnedActor = World->SpawnActor<AActor>(
                    MineItemClass,
                    GetRandomPointInVolume(),
                    FRotator::ZeroRotator,
                    Params
            );
                if (SpawnedActor)
                {
                    AMineItem* MineItem = Cast<AMineItem>(SpawnedActor);
                    SpawnedMines.Add(MineItem);
                    MineItem->ActivationMine();
                }
            }
        }
    }
}

void ASpawnVolume::ClearSpawnedActors()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            if (ABaseItem* BaseItem = Cast<ABaseItem>(Actor))
            {
                BaseItem->DestroyItem();
            }
        }
    }

    SpawnedActors.Empty();
    
    for (AMineItem* MineItem : SpawnedMines)
    {
        if (IsValid(MineItem))
        {
            MineItem->DeActivationMine();
        }
    }
}
