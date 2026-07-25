#include "CombatComponent.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AttackCooldownRemaining > 0.f)
	{
		AttackCooldownRemaining -= DeltaTime;
	}

	if (bIsAttacking)
	{
		AttackTimeRemaining -= DeltaTime;
		if (AttackTimeRemaining <= 0.f)
		{
			OnAttackFinished();
		}
	}

	// Combo window expiry
	if (ComboWindowRemaining > 0.f && !bIsAttacking)
	{
		ComboWindowRemaining -= DeltaTime;
		if (ComboWindowRemaining <= 0.f)
		{
			CurrentComboStep = 0;
		}
	}
}

bool UCombatComponent::MeleeAttack()
{
	if (!CanAttack()) return false;

	bIsAttacking = true;
	bHitDetected = false;
	AttackTimeRemaining = AttackDuration;
	ComboWindowRemaining = 0.f;

	OnAttackStarted.Broadcast();

	// Hit detection at the moment of attack
	PerformMeleeHitDetection();

	return true;
}

void UCombatComponent::PerformMeleeHitDetection()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (!OwnerCharacter) return;

	// Combo step multipliers (step 0, 1, 2)
	static const float DamageMultipliers[] = { 1.0f, 1.2f, 1.5f };
	static const float RangeMultipliers[] = { 1.0f, 1.15f, 1.3f };
	static const float KnockbackMultipliers[] = { 1.0f, 1.0f, 2.0f };
	static const FColor ComboColors[] = { FColor::Red, FColor::Yellow, FColor::Cyan };

	const int32 Step = FMath::Clamp(CurrentComboStep, 0, 2);
	const float EffectiveRange = MeleeRange * RangeMultipliers[Step];
	const float EffectiveRadius = MeleeRadius * RangeMultipliers[Step];

	// Sphere overlap in front of the character
	const FVector Start = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector TraceCenter = Start + Forward * EffectiveRange * 0.5f;

	TArray<FOverlapResult> Overlaps;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(EffectiveRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHasOverlaps = Owner->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		TraceCenter,
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	// Debug visualization - color changes per combo step
	DrawDebugSphere(
		Owner->GetWorld(),
		TraceCenter,
		EffectiveRadius,
		12,
		ComboColors[Step],
		false,
		0.5f
	);

	if (!bHasOverlaps) return;

	ABaseCharacter* OwnerBase = Cast<ABaseCharacter>(OwnerCharacter);
	float BaseDamage = OwnerBase ? OwnerBase->GetAttackPower() : 20.f;
	BaseDamage *= DamageMultipliers[Step];

	const float CritChance = OwnerBase ? OwnerBase->GetCritChance() : 0.f;
	const float Lifesteal = OwnerBase ? OwnerBase->GetLifesteal() : 0.f;

	const float HalfAngleRad = FMath::DegreesToRadians(MeleeAngle * 0.5f);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Owner) continue;

		// Only damage pawns (characters) — skip pickups, props and other world actors
		if (!Cast<APawn>(HitActor)) continue;

		// Angle check - only hit targets within the melee cone
		FVector ToTarget = (HitActor->GetActorLocation() - Start).GetSafeNormal2D();
		float DotProduct = FVector::DotProduct(Forward.GetSafeNormal2D(), ToTarget);
		float AngleToTarget = FMath::Acos(DotProduct);

		if (AngleToTarget > HalfAngleRad) continue;

		// Roll crit per target
		float Damage = BaseDamage;
		const bool bCrit = FMath::FRand() < CritChance;
		if (bCrit)
		{
			Damage *= 2.f;
		}

		// Apply damage
		FDamageEvent DamageEvent;
		const float Dealt = HitActor->TakeDamage(Damage, DamageEvent, OwnerCharacter->GetController(), Owner);

		// Lifesteal — heal the attacker for a fraction of the damage dealt
		if (OwnerBase && Lifesteal > 0.f && Dealt > 0.f)
		{
			OwnerBase->Heal(Dealt * Lifesteal);
		}

		// Knockback - scales with combo step
		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
		{
			FVector KnockbackDir = (HitActor->GetActorLocation() - Start).GetSafeNormal2D();
			FVector Knockback = KnockbackDir * KnockbackForce * KnockbackMultipliers[Step];
			Knockback.Z = 100.f + Step * 75.f;
			HitCharacter->LaunchCharacter(Knockback, true, true);
		}

		UE_LOG(LogTemp, Log, TEXT("Combo %d hit: %s (%.0f dmg%s)"),
			Step + 1, *HitActor->GetName(), Damage, bCrit ? TEXT(" CRIT!") : TEXT(""));
	}
}

void UCombatComponent::OnAttackFinished()
{
	bIsAttacking = false;
	CurrentComboStep++;

	if (CurrentComboStep >= MaxComboSteps)
	{
		// Combo complete or single-hit — full cooldown
		CurrentComboStep = 0;
		AttackCooldownRemaining = AttackCooldown;
	}
	else
	{
		// Can chain next hit — short pause + combo window
		AttackCooldownRemaining = 0.1f;
		ComboWindowRemaining = ComboWindow;
	}

	OnAttackEnded.Broadcast();
}
