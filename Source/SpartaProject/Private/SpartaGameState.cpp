#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpartaCharacter.h"
#include "SpawnVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
	SpawnedCoinAmount = 0;
	CollectedCoinCount = 0;

	CurrentLevelIndex = 0;
	MaxLevels = 3;
	
	CurrentWaveIndex = 0;
	MaxWaves = 3;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();
	
	FString MapName = GetWorld()->GetMapName();

	if (MapName.Contains(TEXT("MenuLevel")))
	{
		return;
	}
	
	UE_LOG(LogTemp, Error,
	   TEXT("GameState=%p BeginPlay Map=%s"),
	   this,
	   *GetWorld()->GetMapName());
	
	StartLevel();
	
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);
	
	if (BackgroundSound)
	{
		UGameplayStatics::PlaySound2D(
			GetWorld(),
			BackgroundSound
		);
	}
}

void ASpartaGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

void ASpartaGameState::StartLevel()
{
	
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}
	
	StartWave();
}

void ASpartaGameState::StartWave()
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Wave %d Started!!!"), CurrentWaveIndex + 1));
	UE_LOG(LogTemp, Error, TEXT("Wave %d start"), CurrentWaveIndex + 1);
	
	bEndingWave = false;
	
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	ASpawnVolume* SpawnVolume = nullptr;
	FWaveInfoRow* WaveInfoRow = nullptr;
	
	if (FoundVolumes.Num() > 0)
	{
		SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
		if (SpawnVolume)
		{
			WaveInfoRow = SpawnVolume->GetWaveInfoRow(CurrentWaveIndex);
			if (WaveInfoRow)
			{
				// 아이템 세팅
				SpawnVolume->SpawnRandomItems(WaveInfoRow->WaveItemSpawnAmount);
				SpawnedCoinAmount = SpawnVolume->GetSpawnedCoinAmount();
				
				// 스파이크 세팅. 1개 이상 스폰 시 UI 업데이트
				int32 SpawnedSpikeAmount = SpawnVolume->ReplaceSpike(CurrentWaveIndex);
				if (SpawnedSpikeAmount >= 0)
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
						{
							SpartaGameInstance->UpdateCurrentSpikeAmount(SpawnedSpikeAmount, true);
						}
					}
				}
				
				// 랜덤 폭탄 세팅
				SpawnVolume->StartSpawnMine(CurrentWaveIndex);
			}
		}
	}
	if (FoundVolumes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SpawnVolume Found. Skip StartLevel"));
		return;
	}
	
	if (SpawnVolume && WaveInfoRow)
	{
		GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveInfoRow->WaveDuration,
		false
		);
	}
}

void ASpartaGameState::OnWaveTimeUp()
{
	OnGameOver(false);
}

void ASpartaGameState::OnCoinCollected()
{
	if (bEndingWave) return;
	
	CollectedCoinCount++;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("coin %d / %d"), CollectedCoinCount, SpawnedCoinAmount));
	
	if (SpawnedCoinAmount > 0 && CollectedCoinCount >= SpawnedCoinAmount)
	{
		bEndingWave = true;
		GetWorldTimerManager().SetTimer(
			WaveTransitionTimerHandle,
			this,
			&ASpartaGameState::EndWave,
			0.1f,
			false
		);
	}
}

void ASpartaGameState::EndWave()
{
	UE_LOG(LogTemp, Error, TEXT("EndWave Called. Wave=%d"), CurrentWaveIndex + 1);
	
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	SpawnedCoinAmount = 0;
	CollectedCoinCount = 0;
	
	// 스폰된 액터들 레벨에서 제거
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	if (FoundVolumes.Num() > 0)
	{
		if (ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]))
		{
			SpawnVolume->ClearSpawnedActors();
		}
	}
	
	if (CurrentWaveIndex + 1 >= MaxWaves)
	{
		EndLevel();
	}
	else
	{
		CurrentWaveIndex++;
		StartWave();
	}
}

void ASpartaGameState::EndLevel()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			if (CurrentLevelIndex + 1 >= MaxLevels)
			{
				OnGameOver(true);
				return;
			}
			SpartaGameInstance->CurrentLevelIndex = ++CurrentLevelIndex;

			if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
			{
				UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
			}
			else
			{
				OnGameOver(false);
			}
		}
	}
}

void ASpartaGameState::OnGameOver(bool bIsCleared)
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetPause(true);
			SpartaPlayerController->ShowGameOverMenu(bIsCleared);
		}
	}
}

void ASpartaGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (UUserWidget* HUDWidget = Cast<ASpartaPlayerController>(PlayerController)->GetHUDWidget())
		{
			if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
			{
				float RemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
				TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
			}

			if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
			{
				if (UGameInstance* GameInstance = GetGameInstance())
				{
					if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
					{
						ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
					}
				}
			}
				
			if (UTextBlock* CoinText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Coin"))))
			{
				if (UGameInstance* GameInstance = GetGameInstance())
				{
					if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
					{
						CoinText->SetText(FText::FromString(FString::Printf(TEXT("Coin: %d / %d"), CollectedCoinCount, SpawnedCoinAmount)));
					}
				}
			}

			if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
			{
				LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d / Wave: %d"), CurrentLevelIndex + 1, CurrentWaveIndex + 1)));
			}
		}
	}
}

void ASpartaGameState::UpdateDebuffUI()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (UUserWidget* HUDWidget = Cast<ASpartaPlayerController>(PlayerController)->GetHUDWidget())
		{
			if (ASpartaCharacter* SpartaCharacter = Cast<ASpartaCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				const UEnum* EnumPtr = StaticEnum<EDebuffType>();
				FString Text;
				for (EDebuffType Debuff : SpartaCharacter->ActiveDebuffs)
				{
					Text += EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Debuff)).ToString();
					Text += TEXT("\n");
				}
					
				if (UTextBlock* DebuffText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Debuff"))))
				{
					DebuffText->SetText(FText::FromString(Text));
						
					UFunction* PlayAnimFunc = HUDWidget->FindFunction(FName("PlayShakeAnim"));
					if (PlayAnimFunc)
					{
						HUDWidget->ProcessEvent(PlayAnimFunc, nullptr);
					}
				}
			}
		}
	}
}

void ASpartaGameState::UpdateHUD_Spike()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (UUserWidget* HUDWidget = Cast<ASpartaPlayerController>(PlayerController)->GetHUDWidget())
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
				{
					if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Spike"))))
					{
						TimeText->SetText(FText::FromString(FString::Printf(TEXT("Spike: %d"), SpartaGameInstance->CurrentSpikeAmount)));
					}
				}
			}
		}
	}
}
