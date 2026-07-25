#include "RangedEnemy.h"
#include "AI/RangedEnemyAIController.h"
#include "Combat/EnemyProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

ARangedEnemy::ARangedEnemy()
{
	// Override AI controller
	AIControllerClass = ARangedEnemyAIController::StaticClass();

	// Blue cube to distinguish from red melee enemies
	EnemyColor = FLinearColor(0.1f, 0.3f, 0.8f);

	// Squishier, slower
	MaxHealth = 40.f;
	AttackPower = 0.f;
	Defense = 0.f;
	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	// Smaller visual
	if (TempMesh)
	{
		TempMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.8f));
	}
}

void ARangedEnemy::FireProjectile()
{
	if (!GetWorld()) return;

	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 80.f;
	FRotator SpawnRot = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	AEnemyProjectile* Proj = GetWorld()->SpawnActor<AEnemyProjectile>(
		AEnemyProjectile::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);

	if (Proj)
	{
		Proj->SetOwnerController(GetController());
	}
}
