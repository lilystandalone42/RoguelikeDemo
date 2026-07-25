#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "RangedEnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class ERangedAIState : uint8
{
	Idle,
	Engage,		// Move to maintain preferred distance
	Attack,		// Fire projectile
	Cooldown	// Wait between shots
};

UCLASS()
class ROGUELIKEDEMO_API ARangedEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARangedEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateEngage(float DeltaTime);
	void UpdateAttack(float DeltaTime);
	void UpdateCooldown(float DeltaTime);

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	ERangedAIState CurrentState = ERangedAIState::Idle;

	// Distance management
	UPROPERTY(EditAnywhere, Category = "AI")
	float PreferredDistance = 600.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MinDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxDistance = 900.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float ShotCooldown = 2.0f;

	float CooldownTimer = 0.f;
};
