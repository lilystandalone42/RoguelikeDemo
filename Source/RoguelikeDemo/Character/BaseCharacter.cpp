#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive()) return;

	// Debug health bar above head
	const FVector HeadLocation = GetActorLocation() + FVector(0.f, 0.f, 120.f);
	const float Percent = GetHealthPercent();

	// Bar made of characters: [████░░░░░░] 60/100
	const int32 BarLength = 10;
	const int32 Filled = FMath::RoundToInt(Percent * BarLength);
	FString Bar = TEXT("[");
	for (int32 i = 0; i < BarLength; i++)
	{
		Bar += (i < Filled) ? TEXT("|") : TEXT(".");
	}
	Bar += FString::Printf(TEXT("] %.0f/%.0f"), CurrentHealth, MaxHealth);

	DrawDebugString(GetWorld(), HeadLocation, Bar, nullptr,
		Percent > 0.5f ? FColor::Green : (Percent > 0.25f ? FColor::Yellow : FColor::Red),
		0.f, true, 1.2f);
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsAlive() || bIsInvincible)
	{
		return 0.f;
	}

	// Apply defense reduction
	const float ActualDamage = FMath::Max(DamageAmount - Defense, 1.f);
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.f, MaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (!IsAlive())
	{
		OnDied();
	}

	return ActualDamage;
}

void ABaseCharacter::Heal(float Amount)
{
	if (!IsAlive() || Amount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ABaseCharacter::AddAttackPower(float Amount)
{
	AttackPower = FMath::Max(0.f, AttackPower + Amount);
}

void ABaseCharacter::AddMoveSpeed(float Amount)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = FMath::Max(0.f, Move->MaxWalkSpeed + Amount);
	}
}

void ABaseCharacter::AddMaxHealth(float Amount)
{
	MaxHealth = FMath::Max(1.f, MaxHealth + Amount);
	// Grant the added capacity as current health too
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ABaseCharacter::AddCritChance(float Amount)
{
	CritChance = FMath::Clamp(CritChance + Amount, 0.f, 1.f);
}

void ABaseCharacter::AddLifesteal(float Amount)
{
	LifestealFraction = FMath::Clamp(LifestealFraction + Amount, 0.f, 1.f);
}

void ABaseCharacter::OnDied()
{
	OnDeath.Broadcast();
}
