#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EDebuffType.h"
#include "SpartaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;

struct FInputActionValue;

UCLASS()
class SPARTAPROJECT_API ASpartaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpartaCharacter();
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	USoundBase* OnHitSound;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);
	
	// 슬로우 디버프
	UFUNCTION(BlueprintCallable, Category = "Move")
	void ApplySlowDebuff(float InSlowMultiplier, float Duration);
	
	UFUNCTION(BlueprintCallable, Category = "Move")
	void RemoveSlowDebuff();
	
	// 화면 가리기 디버프
	UFUNCTION(BlueprintCallable, Category = "Move")
	void ApplyBlindDebuff(float Duration);
	
	UFUNCTION(BlueprintCallable, Category = "Move")
	void RemoveBlindDebuff();
	
	// 디버프 목록
	TSet<EDebuffType> ActiveDebuffs;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float Health;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);

	void OnDeath();
	void UpdateOverheadHP();
	bool UpdateMoveSpeed(float InSlowMultiplier);
private:
	float BaseSpeed;
	float SlowMultiplier;
	float SprintSpeedMultiplier;
	
	FTimerHandle SlowDebuffTimerHandle;
	FTimerHandle BlindDebuffTimerHandle;
};