#include "RangedEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/BaseCharacter.h"
#include "Character/RangedEnemy.h"

ARangedEnemyAIController::ARangedEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2500.f;
	SightConfig->PeripheralVisionAngleDegrees = 360.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void ARangedEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &ARangedEnemyAIController::OnTargetPerceptionUpdated);
}

void ARangedEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* DetectedPawn = Cast<APawn>(Actor);
	if (!DetectedPawn || !DetectedPawn->IsPlayerControlled()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		TargetActor = Actor;
		if (CurrentState == ERangedAIState::Idle)
		{
			CurrentState = ERangedAIState::Engage;
		}
	}
	else
	{
		TargetActor = nullptr;
		StopMovement();
		CurrentState = ERangedAIState::Idle;
	}
}

void ARangedEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	ABaseCharacter* Self = Cast<ABaseCharacter>(ControlledPawn);
	if (Self && !Self->IsAlive())
	{
		StopMovement();
		return;
	}

	if (TargetActor)
	{
		ABaseCharacter* TargetChar = Cast<ABaseCharacter>(TargetActor);
		if (TargetChar && !TargetChar->IsAlive())
		{
			TargetActor = nullptr;
			StopMovement();
			CurrentState = ERangedAIState::Idle;
		}
	}

	// Safety net: re-acquire the player if perception dropped the target but the
	// player is still in sight (perception events only fire on change).
	if (!TargetActor && AIPerceptionComp)
	{
		TArray<AActor*> PerceivedActors;
		AIPerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
		for (AActor* Percieved : PerceivedActors)
		{
			APawn* Pawn = Cast<APawn>(Percieved);
			if (Pawn && Pawn->IsPlayerControlled())
			{
				TargetActor = Pawn;
				CurrentState = ERangedAIState::Engage;
				break;
			}
		}
	}

	switch (CurrentState)
	{
	case ERangedAIState::Idle:
		break;
	case ERangedAIState::Engage:
		UpdateEngage(DeltaTime);
		break;
	case ERangedAIState::Attack:
		UpdateAttack(DeltaTime);
		break;
	case ERangedAIState::Cooldown:
		UpdateCooldown(DeltaTime);
		break;
	}
}

void ARangedEnemyAIController::UpdateEngage(float DeltaTime)
{
	if (!TargetActor || !GetPawn())
	{
		CurrentState = ERangedAIState::Idle;
		return;
	}

	float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());

	// At good range — shoot
	if (Distance >= MinDistance && Distance <= MaxDistance)
	{
		StopMovement();
		CurrentState = ERangedAIState::Attack;
		return;
	}

	if (Distance < MinDistance)
	{
		// Too close — retreat
		FVector AwayDir = (GetPawn()->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
		FVector RetreatTarget = GetPawn()->GetActorLocation() + AwayDir * PreferredDistance;
		MoveToLocation(RetreatTarget);
	}
	else
	{
		// Too far — approach
		MoveToActor(TargetActor, PreferredDistance);
	}
}

void ARangedEnemyAIController::UpdateAttack(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !TargetActor)
	{
		CurrentState = ERangedAIState::Idle;
		return;
	}

	// Face target
	FVector Dir = (TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal2D();
	if (!Dir.IsNearlyZero())
	{
		ControlledPawn->SetActorRotation(Dir.Rotation());
	}

	// Fire
	if (ARangedEnemy* RangedPawn = Cast<ARangedEnemy>(ControlledPawn))
	{
		RangedPawn->FireProjectile();
	}

	CurrentState = ERangedAIState::Cooldown;
	CooldownTimer = ShotCooldown;
}

void ARangedEnemyAIController::UpdateCooldown(float DeltaTime)
{
	// Still retreat if player is too close during cooldown
	if (TargetActor && GetPawn())
	{
		float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
		if (Distance < MinDistance)
		{
			FVector AwayDir = (GetPawn()->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
			FVector RetreatTarget = GetPawn()->GetActorLocation() + AwayDir * 200.f;
			MoveToLocation(RetreatTarget);
		}
	}

	CooldownTimer -= DeltaTime;
	if (CooldownTimer <= 0.f)
	{
		CurrentState = TargetActor ? ERangedAIState::Engage : ERangedAIState::Idle;
	}
}
