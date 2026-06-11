#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

ASpartaPlayerController::ASpartaPlayerController()
	: InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr),
	GameOverMenuWidgetClass(nullptr),
	GameOverMenuWidgetInstance(nullptr)
{
}

void ASpartaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
	{
		ShowMainMenu();
	}
}

UUserWidget* ASpartaPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void ASpartaPlayerController::ShowMainMenu()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ASpartaPlayerController::ShowGameOverMenu(bool bIsCleared)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (GameOverMenuWidgetInstance)
	{
		GameOverMenuWidgetInstance->RemoveFromParent();
		GameOverMenuWidgetInstance = nullptr;
	}
	
	if (GameOverMenuWidgetClass)
	{
		GameOverMenuWidgetInstance = CreateWidget<UUserWidget>(this, GameOverMenuWidgetClass);
		if (GameOverMenuWidgetInstance)
		{
			GameOverMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
		
		UFunction* PlayAnimFunc = GameOverMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
		if (PlayAnimFunc)
		{
			GameOverMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
		}
		
		if (UTextBlock* ButtonText = Cast<UTextBlock>(GameOverMenuWidgetInstance->GetWidgetFromName(TEXT("GameOverText"))))
		{
			if (bIsCleared)
			{
				ButtonText->SetText(FText::FromString(TEXT("GAME CLEAR!")));
				ButtonText->SetColorAndOpacity(GameClearTextColor);
			}
			else
			{
				ButtonText->SetText(FText::FromString(TEXT("GAME OVER!")));
				ButtonText->SetColorAndOpacity(GameOverTextColor);
			}
		}
		
		if (UTextBlock* TotalScoreText = Cast<UTextBlock>(GameOverMenuWidgetInstance->GetWidgetFromName("TotalScoreText")))
		{
			if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				TotalScoreText->SetText(FText::FromString(
					FString::Printf(TEXT("Total Score: %d"), SpartaGameInstance->TotalScore)
				));
			}
		}
	}
}

void ASpartaPlayerController::SetBlindWidget(bool bIsActive)
{
	if (bIsActive)
	{
		// 이미 활성화 되어있으면 취소.
		if (BlindWidgetInstance) return;
		
		if (BlindWidgetClass)
		{
			BlindWidgetInstance = CreateWidget<UUserWidget>(this, BlindWidgetClass);
			if (BlindWidgetInstance)
			{
				BlindWidgetInstance->AddToViewport();
			}
		}
	}
	else
	{
		if (BlindWidgetInstance)
		{
			BlindWidgetInstance->RemoveFromParent();
			BlindWidgetInstance = nullptr;
		}
	}
}

void ASpartaPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
			HUDWidgetInstance->RemoveFromParent();
			HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
			MainMenuWidgetInstance->RemoveFromParent();
			MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
			HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			if (HUDWidgetInstance)
			{
					HUDWidgetInstance->AddToViewport(100);
		
					bShowMouseCursor = false;
					SetInputMode(FInputModeGameOnly());
			}
	
			ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
			if (SpartaGameState)
			{
					SpartaGameState->UpdateHUD();
			}
	}
}

void ASpartaPlayerController::StartGame()
{
	USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (SpartaGameInstance)
	{
			SpartaGameInstance->CurrentLevelIndex = 0;
			SpartaGameInstance->TotalScore = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}