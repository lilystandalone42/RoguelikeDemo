#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(Abstract)
class ROGUELIKEDEMO_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddAttackPower(float Amount);

	// Buff hooks used by pickups
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddMoveSpeed(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddMaxHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddCritChance(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddLifesteal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetCritChance() const { return CritChance; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetLifesteal() const { return LifestealFraction; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetAttackPower() const { return AttackPower; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnDeath OnDeath;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackPower = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Defense = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeedMultiplier = 1.f;

	// Chance (0..1) for a melee hit to deal double damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CritChance = 0.f;

	// Fraction (0..1) of damage dealt returned to the attacker as health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float LifestealFraction = 0.f;

	// Whether this character is invincible (e.g. during dash)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInvincible = false;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnDied();
};
