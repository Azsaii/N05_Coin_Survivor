#include "Spike.h"

#include "SpartaGameInstance.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ASpike::ASpike()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MoveDirection = {1.0f, 0.0f, 0.0f};
	MoveSpeed = 300.f;
	MaxMoveRange = 500.f;
	moveSign = 1.f;
	
	Damage = 10.0f;
	ObstacleType = "Spike";
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(SceneRoot);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 충돌 컴포넌트 생성 및 설정
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	// 겹침만 감지하는 프로파일 설정
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	// 루트 컴포넌트로 설정
	Collision->SetupAttachment(SceneRoot);
	
	// Overlap 이벤트 바인딩
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ASpike::OnInteractableObjectOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ASpike::OnInteractableObjectEndOverlap);

	// 비활성화. 스포너가 시작위치 랜덤으로 정해준 이후 활성화되어 움직인다.
	bIsActive = false;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void ASpike::InitSpike(FWaveInfoRow* WaveInfoRow)
{
	StartLocation = GetActorLocation();
	
	MoveDirection = FVector(FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f), 0.f);
	if (MoveDirection == FVector::ZeroVector) MoveDirection = FVector(1.0f, 0.f, 0.f);
	MoveDirection = MoveDirection.GetSafeNormal();
	
	MoveSpeed = WaveInfoRow->WaveSpikeMoveSpeed;
	MaxMoveRange = WaveInfoRow->WaveSpikeMaxMoveRange;
}

void ASpike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsActive)
	{
		FVector MoveDir = MoveDirection * moveSign;
		FVector CurrentLocation = GetActorLocation();
	
		if (FVector::Dist(CurrentLocation, StartLocation) >= MaxMoveRange)
		{
			SetActorLocation(StartLocation + MoveDir * MaxMoveRange);
			moveSign *= -1.f;
			MoveDir *= -1.f;
		}
		AddActorWorldOffset(MoveDir * MoveSpeed * DeltaTime);

	}
}

void ASpike::ActivateSpike()
{
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	bIsActive = true;
}

void ASpike::OnInteractableObjectOverlap(
	UPrimitiveComponent* OverlappedComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// OtherActor가 플레이어인지 확인 ("Player" 태그 활용)
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		// 아이템 사용 (획득) 로직 호출
		ActivateInteractableObject(OtherActor);
	}
}

void ASpike::OnInteractableObjectEndOverlap(
	UPrimitiveComponent* OverlappedComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
}

void ASpike::ActivateInteractableObject(AActor* Activator)
{
	// 데미지 부여
	TArray<AActor*> OverlappingActors;
	Collision->GetOverlappingActors(OverlappingActors);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				Damage,
				nullptr,
				this,
				UDamageType::StaticClass()
			);
		}
	}
	
	// 사운드 재생
	if (OverlapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			OverlapSound,
			GetActorLocation()
		);
	}
	
	// 파티클 스폰 / 제거
	UParticleSystemComponent* Particle = nullptr;
	if (OverlapParticle)
	{
		Particle = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			OverlapParticle,
			GetActorLocation(),
			GetActorRotation(),
			false
		);
		
		if (Particle)
		{
			FTimerHandle DestroyParticleTimerHandle;
			TWeakObjectPtr<UParticleSystemComponent> WeakParticle = Particle;
						
			GetWorld()->GetTimerManager().SetTimer(
				DestroyParticleTimerHandle,
				[WeakParticle]()
				{
					if (WeakParticle.IsValid())
					{
						WeakParticle->DestroyComponent();
					}
				},
				2.0f,
				false
			);
		}
	}

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	bIsActive = false;
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
		{
			SpartaGameInstance->UpdateCurrentSpikeAmount(-1);
		}
	}
}

FName ASpike::GetInteractableObjectType() const
{
	return ObstacleType;
}
