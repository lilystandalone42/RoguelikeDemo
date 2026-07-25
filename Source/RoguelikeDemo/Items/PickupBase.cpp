#include "PickupBase.h"
#include "Components/SphereComponent.h"
#include "Character/BaseCharacter.h"
#include "DrawDebugHelpers.h"

APickupBase::APickupBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(80.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
		MeshComp->SetRelativeScale3D(FVector(0.5f));
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnOverlap);

	// Tint the mesh with the pickup color
	if (MeshComp)
	{
		UMaterialInstanceDynamic* DynMat = MeshComp->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), PickupColor);
			DynMat->SetVectorParameterValue(TEXT("Color"), PickupColor);
		}
	}
}

void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Bob up and down + spin so it reads as a collectible
	FVector Loc = BaseLocation;
	Loc.Z += FMath::Sin(GetGameTimeSinceCreation() * 3.f) * 20.f;
	SetActorLocation(Loc);
	AddActorLocalRotation(FRotator(0.f, DeltaTime * 90.f, 0.f));

	// Floating label
	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 70.f),
		GetLabelText(), nullptr, LabelColor, 0.f, true, 1.3f);
}

void APickupBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;

	ABaseCharacter* Collector = Cast<ABaseCharacter>(OtherActor);
	if (!Collector) return;

	bCollected = true;
	ApplyEffect(Collector);
	Destroy();
}
