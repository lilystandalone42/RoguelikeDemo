#include "EnemyAIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/BaseCharacter.h"
#include "Combat/CombatComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// AI Perception - sight sense with 360 degree awareness
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 360.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AEnemyAIController::OnTargetPerceptionUpdated);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* DetectedPawn = Cast<APawn>(Actor);
	if (!DetectedPawn || !DetectedPawn->IsPlayerControlled()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		TargetActor = Actor;
		if (CurrentState == EEnemyAIState::Idle)
		{
			CurrentState = EEnemyAIState::Chase;
		}
	}
	else
	{
		TargetActor = nullptr;
		StopMovement();
		CurrentState = EEnemyAIState::Idle;
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Stop if dead
	ABaseCharacter* Self = Cast<ABaseCharacter>(ControlledPawn);
	if (Self && !Self->IsAlive())
	{
		StopMovement();
		return;
	}

	// Stop if target died
	if (TargetActor)
	{
		ABaseCharacter* TargetChar = Cast<ABaseCharacter>(TargetActor);
		if (TargetChar && !TargetChar->IsAlive())
		{
			TargetActor = nullptr;
			StopMovement();
			CurrentState = EEnemyAIState::Idle;
		}
	}

	// Safety net: perception events only fire on change, so a brief line-of-sight
	// break can drop us into Idle and never recover. If the player is currently in
	// sight, re-acquire and resume the chase.
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
				CurrentState = EEnemyAIState::Chase;
				break;
			}
		}
	}

	switch (CurrentState)
	{
	case EEnemyAIState::Idle:
		break;
	case EEnemyAIState::Chase:
		UpdateChase(DeltaTime);
		break;
	case EEnemyAIState::Attack:
		UpdateAttack(DeltaTime);
		break;
	case EEnemyAIState::Cooldown:
		UpdateCooldown(DeltaTime);
		break;
	}
}

void AEnemyAIController::UpdateChase(float DeltaTime)
{
	if (!TargetActor || !GetPawn())
	{
		CurrentState = EEnemyAIState::Idle;
		return;
	}

	float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance <= AttackRange)
	{
		StopMovement();
		CurrentState = EEnemyAIState::Attack;
		return;
	}

	EPathFollowingRequestResult::Type MoveResult = MoveToActor(TargetActor, AttackRange * 0.7f);

	// If the target is off the NavMesh (e.g. player ran onto a ledge/edge), pathing
	// fails and the enemy would freeze. Instead, head to the nearest reachable point.
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		FNavLocation NavLoc;
		if (NavSys && NavSys->ProjectPointToNavigation(
				TargetActor->GetActorLocation(), NavLoc, FVector(800.f, 800.f, 600.f)))
		{
			MoveToLocation(NavLoc.Location, AttackRange * 0.7f);
		}
	}
}

void AEnemyAIController::UpdateAttack(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !TargetActor)
	{
		CurrentState = EEnemyAIState::Idle;
		return;
	}

	// Face the target
	FVector Dir = (TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal2D();
	if (!Dir.IsNearlyZero())
	{
		ControlledPawn->SetActorRotation(Dir.Rotation());
	}

	// Swing
	UCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		CombatComp->MeleeAttack();
	}

	// Always go to cooldown — gives player a reaction window
	CurrentState = EEnemyAIState::Cooldown;
	CooldownTimer = PostAttackCooldown;
}

void AEnemyAIController::UpdateCooldown(float DeltaTime)
{
	CooldownTimer -= DeltaTime;
	if (CooldownTimer <= 0.f)
	{
		CurrentState = TargetActor ? EEnemyAIState::Chase : EEnemyAIState::Idle;
	}
}
