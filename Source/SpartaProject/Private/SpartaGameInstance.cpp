#include "SpartaGameInstance.h"
#include "SpartaGameState.h"

USpartaGameInstance::USpartaGameInstance()
{
	TotalScore = 0;
	CurrentLevelIndex = 0;
}

void USpartaGameInstance::AddToScore(int32 Amount)
{
	TotalScore += Amount;
}

void USpartaGameInstance::UpdateCurrentSpikeAmount(int32 Amount, bool bIsInit)
{
	if (bIsInit) CurrentSpikeAmount = Amount;
	else CurrentSpikeAmount += Amount;

	if (ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr)
	{
		SpartaGameState->UpdateHUD_Spike();
	}
}
