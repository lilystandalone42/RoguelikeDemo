#include "EnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"

AEnemyProjectile::AEnemyProjectile()
{
	// Collision sphere
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(10.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SetRootComponent(CollisionComp);

	// Visual - small sphere
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
		MeshComp->SetRelativeScale3D(FVector(0.15f));
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 5.f;
}

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::OnOverlap);
	CollisionComp->OnComponentHit.AddDynamic(this, &AEnemyProjectile::OnHit);
}

void AEnemyProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;

	// Only damage player-controlled pawns
	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (!HitPawn || !HitPawn->IsPlayerControlled()) return;

	FDamageEvent DamageEvent;
	OtherActor->TakeDamage(Damage, DamageEvent, OwnerController, GetInstigator());
	Destroy();
}

void AEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Hit a wall — destroy
	Destroy();
}
