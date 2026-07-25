#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROGUELIKEDEMO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Attempt a melee attack; returns true if attack was initiated
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool MeleeAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanAttack() const { return !bIsAttacking && AttackCooldownRemaining <= 0.f; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackStarted OnAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackEnded OnAttackEnded;

	// Melee parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float MeleeRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float MeleeRadius = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float MeleeAngle = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float AttackDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float AttackCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
	float KnockbackForce = 800.f;

	// Combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	int32 MaxComboSteps = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	float ComboWindow = 0.6f;

private:
	void PerformMeleeHitDetection();
	void OnAttackFinished();

	bool bIsAttacking = false;
	float AttackCooldownRemaining = 0.f;
	float AttackTimeRemaining = 0.f;
	bool bHitDetected = false; // prevent multi-hit in one swing

	int32 CurrentComboStep = 0;
	float ComboWindowRemaining = 0.f;
};
