#include "ExitPortal.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

AExitPortal::AExitPortal()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(120.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(CollisionComp);

	// Green sphere visual
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
		MeshComp->SetRelativeScale3D(FVector(1.5f));
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AExitPortal::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AExitPortal::OnOverlap);

	// Green tint
	if (MeshComp)
	{
		UMaterialInstanceDynamic* DynMat = MeshComp->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.9f, 0.3f));
		}
	}
}

void AExitPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Bob up and down
	FVector Loc = BaseLocation;
	Loc.Z += FMath::Sin(GetGameTimeSinceCreation() * 2.f) * 30.f;
	SetActorLocation(Loc);

	// Debug visuals — green sphere + label
	DrawDebugSphere(GetWorld(), GetActorLocation(), 100.f, 16, FColor::Green, false, 0.f, 0, 3.f);
	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 100.f),
		TEXT(">> NEXT ROOM >>"), nullptr, FColor::Green, 0.f, true, 1.5f);
}

void AExitPortal::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bActivated) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		bActivated = true;
		OnPortalActivated.Broadcast();
	}
}
