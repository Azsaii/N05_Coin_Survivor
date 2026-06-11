#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ABaseItem::ABaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
    
	// 루트 컴포넌트 생성 및 설정
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	// 충돌 컴포넌트 생성 및 설정
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	// 겹침만 감지하는 프로파일 설정
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	// 루트 컴포넌트로 설정
	Collision->SetupAttachment(Scene);
    
	// 스태틱 메시 컴포넌트 생성 및 설정
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);
	
	// 메시가 불필요하게 충돌을 막지 않도록 하기 위해, 별도로 NoCollision 등으로 설정할 수 있음.
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Overlap 이벤트 바인딩
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnInteractableObjectOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnInteractableObjectEndOverlap);
}

void ABaseItem::OnInteractableObjectOverlap(
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

void ABaseItem::OnInteractableObjectEndOverlap(
			UPrimitiveComponent* OverlappedComp, 
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComp, 
			int32 OtherBodyIndex)
{
}

void ABaseItem::ActivateInteractableObject(AActor* Activator)
{
	UParticleSystemComponent* Particle = nullptr;
	
	if (PickupParticle)
	{
		Particle = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			PickupParticle,
			GetActorLocation(),
			GetActorRotation(),
			true
		);
	}
	
	if (Particle)
	{
		FTimerHandle DestroyParticleTimerHandle;
		TWeakObjectPtr<UParticleSystemComponent> WeakParticle = Particle;
						
		GetWorld()->GetTimerManager().SetTimer(
			DestroyParticleTimerHandle,
			[WeakParticle]
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
	
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			PickupSound,
			GetActorLocation()
		);
	}
}

FName ABaseItem::GetInteractableObjectType() const
{
	return ItemType;
}

void ABaseItem::DestroyItem()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
    
	// Destroy 와 아이템 스폰이 거의 동시에 일어나면 언리얼 내부의 아이템 인스턴스 관리에서 충돌 생겨서 아이템 스폰에 실패한다.
	// 따라서 Destroy 를 지연시켜야 한다.
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&ABaseItem::ActualDestroy,
		0.1f,
		false
	);
}

void ABaseItem::ActualDestroy()
{
	Destroy();
}
