#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle,
	Chase,
	Attack,
	Cooldown
};

UCLASS()
class ROGUELIKEDEMO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateChase(float DeltaTime);
	void UpdateAttack(float DeltaTime);
	void UpdateCooldown(float DeltaTime);

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	EEnemyAIState CurrentState = EEnemyAIState::Idle;

	// How close the enemy needs to be to attack
	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRange = 180.f;

	// Pause after attacking before chasing again
	UPROPERTY(EditAnywhere, Category = "AI")
	float PostAttackCooldown = 1.2f;

	float CooldownTimer = 0.f;
};
