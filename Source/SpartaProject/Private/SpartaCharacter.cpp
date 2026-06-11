#include "SpartaCharacter.h"

#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ProgressBar.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

ASpartaCharacter::ASpartaCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	
	BaseSpeed = 600.f;
	SprintSpeedMultiplier = 1.7f;
	SlowMultiplier = 1.0f;

	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void ASpartaCharacter::BeginPlay()
{
		Super::BeginPlay();
		UpdateOverheadHP();
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
						PlayerController->MoveAction,
						ETriggerEvent::Triggered,
						this,
						&ASpartaCharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
						PlayerController->JumpAction,
						ETriggerEvent::Triggered,
						this,
						&ASpartaCharacter::StartJump
				);

				EnhancedInput->BindAction(
						PlayerController->MoveAction,
						ETriggerEvent::Completed,
						this,
						&ASpartaCharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
						PlayerController->LookAction,
						ETriggerEvent::Triggered,
						this,
						&ASpartaCharacter::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
						PlayerController->SprintAction,
						ETriggerEvent::Triggered,
						this,
						&ASpartaCharacter::StartSprint
				);

				EnhancedInput->BindAction(
						PlayerController->SprintAction,
						ETriggerEvent::Completed,
						this,
						&ASpartaCharacter::StopSprint
				);
			}
		}
	}
}

void ASpartaCharacter::Move(const FInputActionValue& value)
{
		if (!Controller) return;
	
		const FVector2D MoveInput = value.Get<FVector2D>();
	
		if (!FMath::IsNearlyZero(MoveInput.X))
		{
				AddMovementInput(GetActorForwardVector(), MoveInput.X);
		}
	
		if (!FMath::IsNearlyZero(MoveInput.Y))
		{
				AddMovementInput(GetActorRightVector(), MoveInput.Y);
		}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value)
{
		if (value.Get<bool>())
		{
				Jump();
		}
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
		if (!value.Get<bool>())
		{
				StopJumping();
		}
}

void ASpartaCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * SlowMultiplier * SprintSpeedMultiplier;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * SlowMultiplier;
	}
}

float ASpartaCharacter::GetHealth() const
{
	return Health;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}


bool ASpartaCharacter::UpdateMoveSpeed(float InSlowMultiplier)
{
	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		bool check = CharacterMovementComponent->MaxWalkSpeed == BaseSpeed * SlowMultiplier;
		
		SlowMultiplier = InSlowMultiplier;
		if (check)
		{
			CharacterMovementComponent->MaxWalkSpeed = BaseSpeed * SlowMultiplier;
		}
		else
		{
			CharacterMovementComponent->MaxWalkSpeed = BaseSpeed * SlowMultiplier * SprintSpeedMultiplier;
		}
		
		return true;
	}
	
	return false;
}

void ASpartaCharacter::ApplySlowDebuff(float InSlowMultiplier, float Duration)
{
	if (UpdateMoveSpeed(InSlowMultiplier))
	{
		ActiveDebuffs.Add(EDebuffType::Slow);
		
		GetWorldTimerManager().ClearTimer(SlowDebuffTimerHandle);
		GetWorldTimerManager().SetTimer(
			SlowDebuffTimerHandle,
			this,
			&ASpartaCharacter::RemoveSlowDebuff,
			Duration,
			false
		);
		
		if (ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr)
		{
			SpartaGameState->UpdateDebuffUI();
		}
	}
}

void ASpartaCharacter::RemoveSlowDebuff()
{
	if (UpdateMoveSpeed(1.0f))
	{
		ActiveDebuffs.Remove(EDebuffType::Slow);
		
		if (ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr)
		{
			SpartaGameState->UpdateDebuffUI();
		}
	}
}

void ASpartaCharacter::ApplyBlindDebuff(float Duration)
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetBlindWidget(true);
			ActiveDebuffs.Add(EDebuffType::Blind);
			
			GetWorldTimerManager().ClearTimer(BlindDebuffTimerHandle);
			GetWorldTimerManager().SetTimer(
				BlindDebuffTimerHandle,
				this,
				&ASpartaCharacter::RemoveBlindDebuff,
				Duration,
				false
			);
			
			if (ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr)
			{
				SpartaGameState->UpdateDebuffUI();
			}
		}	
	}
}

void ASpartaCharacter::RemoveBlindDebuff()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetBlindWidget(false);
			ActiveDebuffs.Remove(EDebuffType::Blind);
			
			if (ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr)
			{
				SpartaGameState->UpdateDebuffUI();
			}
		}	
	}
}

float ASpartaCharacter::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();

	if (Health <= 0.0f)
	{
		OnDeath();
	}
	
	if (OnHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			OnHitSound,
			GetActorLocation()
		);
	}

	return ActualDamage;
}

void ASpartaCharacter::OnDeath()
{
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver(false);
	}
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
	
	float PrevPersent = 0.f;
	if (UProgressBar* HPBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName(TEXT("PbHpBar"))))
	{
		PrevPersent = HPBar->GetPercent();
		HPBar->SetPercent(Health / MaxHealth);
	}
	
	// HP 감소 시에만 애니메이션 재생
	if (PrevPersent > Health / MaxHealth)
	{
		if (UFunction* PlayAnimFunc = OverheadWidgetInstance->FindFunction(FName("PlayShakeAnim")))
		{
			OverheadWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
		}
	}
}

