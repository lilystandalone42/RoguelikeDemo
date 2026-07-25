#include "RoomManager.h"
#include "ExitPortal.h"
#include "Items/PickupBase.h"
#include "Items/HealthPickup.h"
#include "Items/AttackPickup.h"
#include "Items/SpeedPickup.h"
#include "Items/LifestealPickup.h"
#include "Items/CritPickup.h"
#include "Items/MaxHealthPickup.h"
#include "Character/EnemyBase.h"
#include "Character/RangedEnemy.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/DamageEvents.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

ARoomManager::ARoomManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root component so the actor has a Transform and is movable in the editor
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
	EditorIcon->SetupAttachment(SceneRoot);
#endif

	// Default loot table — one is picked at random per room clear
	PossibleDrops.Add(AHealthPickup::StaticClass());
	PossibleDrops.Add(AAttackPickup::StaticClass());
	PossibleDrops.Add(ASpeedPickup::StaticClass());
	PossibleDrops.Add(ALifestealPickup::StaticClass());
	PossibleDrops.Add(ACritPickup::StaticClass());
	PossibleDrops.Add(AMaxHealthPickup::StaticClass());

	// Default 5-room progression
	RoomConfigs.Add({2, 0});  // Room 1: easy
	RoomConfigs.Add({3, 1});  // Room 2
	RoomConfigs.Add({3, 2});  // Room 3
	RoomConfigs.Add({4, 2});  // Room 4
	RoomConfigs.Add({5, 3});  // Room 5: hard
}

void ARoomManager::BeginPlay()
{
	Super::BeginPlay();

	ArenaCenter = GetActorLocation();

	// Clear any enemies left in the level (e.g. manually placed test dummies).
	// The RoomManager is the single source of truth for spawns — leftover placed
	// enemies aren't counted and would make the room "clear" while they're alive.
	TArray<AActor*> ExistingEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), ExistingEnemies);
	for (AActor* Existing : ExistingEnemies)
	{
		if (IsValid(Existing))
		{
			UE_LOG(LogTemp, Warning, TEXT("Clearing pre-placed enemy: %s"), *Existing->GetName());
			Existing->Destroy();
		}
	}

	// Start first room after a short delay so player can orient
	FTimerHandle StartTimer;
	GetWorldTimerManager().SetTimer(StartTimer, [this]()
	{
		StartRoom(0);
	}, 2.f, false);
}

void ARoomManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rescue check — enemies that fall off the arena (into holes / off ledges /
	// knocked out of bounds) would soft-lock the room. Kill them so it can progress.
	if (CurrentState == ERoomState::Combat)
	{
		for (AActor* EnemyActor : SpawnedEnemies)
		{
			ABaseCharacter* Enemy = Cast<ABaseCharacter>(EnemyActor);
			if (Enemy && Enemy->IsAlive())
			{
				const FVector Loc = Enemy->GetActorLocation();
				const bool bFellOff = Loc.Z < ArenaCenter.Z - 400.f;
				const bool bTooFar = FVector::Dist2D(Loc, ArenaCenter) > SpawnRadius * 3.f;
				if (bFellOff || bTooFar)
				{
					UE_LOG(LogTemp, Warning, TEXT("RESCUE: %s left the arena (%s) — counting as dead"),
						*Enemy->GetName(), bFellOff ? TEXT("fell off") : TEXT("too far"));
					FDamageEvent DmgEvent;
					Enemy->TakeDamage(1.0e6f, DmgEvent, nullptr, this);
				}
			}
		}
	}

	// Debug HUD — room info at top of arena
	FString StateStr;
	switch (CurrentState)
	{
	case ERoomState::WaitingToStart: StateStr = TEXT("PREPARING..."); break;
	case ERoomState::Combat: StateStr = FString::Printf(TEXT("FIGHT! Enemies: %d"), EnemiesAlive); break;
	case ERoomState::Cleared: StateStr = TEXT("CLEARED! Enter the portal"); break;
	case ERoomState::Transitioning: StateStr = TEXT("NEXT ROOM..."); break;
	}

	FString RoomInfo = FString::Printf(TEXT("Room %d/%d  %s"),
		FMath::Min(CurrentRoomIndex + 1, RoomConfigs.Num()),
		RoomConfigs.Num(), *StateStr);

	DrawDebugString(GetWorld(), ArenaCenter + FVector(0.f, 0.f, 300.f),
		RoomInfo, nullptr, FColor::White, 0.f, true, 2.f);

	// Show the spawn ring so the arena layout is easy to verify
	DrawDebugCircle(GetWorld(), ArenaCenter + FVector(0.f, 0.f, 20.f), SpawnRadius, 48,
		FColor::Cyan, false, 0.f, 0, 4.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
}

void ARoomManager::StartRoom(int32 RoomIndex)
{
	if (!RoomConfigs.IsValidIndex(RoomIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("*** ALL ROOMS CLEARED — VICTORY! ***"));
		CurrentState = ERoomState::Cleared;

		// Show victory text
		DrawDebugString(GetWorld(), ArenaCenter + FVector(0.f, 0.f, 200.f),
			TEXT("VICTORY!"), nullptr, FColor::Yellow, 10.f, true, 4.f);
		return;
	}

	CurrentRoomIndex = RoomIndex;
	CurrentState = ERoomState::Combat;

	UE_LOG(LogTemp, Warning, TEXT("=== Room %d/%d START ==="), RoomIndex + 1, RoomConfigs.Num());

	SpawnEnemiesForRoom(RoomConfigs[RoomIndex]);
}

void ARoomManager::SpawnEnemiesForRoom(const FRoomWaveConfig& Config)
{
	UWorld* World = GetWorld();
	if (!World) return;

	EnemiesAlive = 0;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn melee enemies
	for (int32 i = 0; i < Config.MeleeEnemyCount; i++)
	{
		FVector SpawnLoc = GetRandomSpawnPoint();
		AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(
			AEnemyBase::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Enemy)
		{
			SpawnedEnemies.Add(Enemy);
			Enemy->OnDeath.AddDynamic(this, &ARoomManager::OnEnemyDied);
			EnemiesAlive++;
		}
	}

	// Spawn ranged enemies
	for (int32 i = 0; i < Config.RangedEnemyCount; i++)
	{
		FVector SpawnLoc = GetRandomSpawnPoint();
		ARangedEnemy* Enemy = World->SpawnActor<ARangedEnemy>(
			ARangedEnemy::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Enemy)
		{
			SpawnedEnemies.Add(Enemy);
			Enemy->OnDeath.AddDynamic(this, &ARoomManager::OnEnemyDied);
			EnemiesAlive++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Spawned %d enemies (Melee:%d Ranged:%d)"),
		EnemiesAlive, Config.MeleeEnemyCount, Config.RangedEnemyCount);
}

FVector ARoomManager::GetRandomSpawnPoint() const
{
	// Ring distribution — don't spawn on top of player
	const float MinRadius = SpawnRadius * 0.5f;
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);

	// Try several candidates; require solid floor beneath the point so enemies
	// never spawn into holes or off the edge of the arena.
	for (int32 Attempt = 0; Attempt < 12; Attempt++)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(MinRadius, SpawnRadius);

		FVector Candidate = ArenaCenter;
		Candidate.X += FMath::Cos(Angle) * Radius;
		Candidate.Y += FMath::Sin(Angle) * Radius;

		// Trace down to find the floor at this XY
		const FVector TraceStart = Candidate + FVector(0.f, 0.f, 500.f);
		const FVector TraceEnd = Candidate - FVector(0.f, 0.f, 1000.f);
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (World && World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			const FVector GroundPoint = Hit.Location + FVector(0.f, 0.f, 100.f);

			// Prefer points that are also reachable on the NavMesh
			if (NavSys)
			{
				FNavLocation NavLoc;
				if (NavSys->ProjectPointToNavigation(GroundPoint, NavLoc, FVector(150.f, 150.f, 300.f)))
				{
					return NavLoc.Location + FVector(0.f, 0.f, 100.f);
				}
				continue; // solid ground but off the navmesh — keep looking
			}
			return GroundPoint;
		}
	}

	// Fallback — spawn on the manager itself (guaranteed on solid floor)
	return ArenaCenter + FVector(0.f, 0.f, 100.f);
}

void ARoomManager::OnEnemyDied()
{
	EnemiesAlive = FMath::Max(EnemiesAlive - 1, 0);
	UE_LOG(LogTemp, Warning, TEXT("Enemy killed! %d remaining"), EnemiesAlive);

	if (EnemiesAlive <= 0 && CurrentState == ERoomState::Combat)
	{
		OnRoomCleared();
	}
}

void ARoomManager::OnRoomCleared()
{
	CurrentState = ERoomState::Cleared;
	UE_LOG(LogTemp, Warning, TEXT("=== Room %d CLEARED ==="), CurrentRoomIndex + 1);

	// Last room? Victory!
	if (CurrentRoomIndex >= RoomConfigs.Num() - 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("*** ALL ROOMS CLEARED — VICTORY! ***"));
		return;
	}

	// Reward: drop a random pickup from the loot table near the center so the
	// player can grab it before moving on to the next (harder) room.
	UWorld* World = GetWorld();
	if (World && PossibleDrops.Num() > 0)
	{
		const int32 Pick = FMath::RandRange(0, PossibleDrops.Num() - 1);
		TSubclassOf<APickupBase> DropClass = PossibleDrops[Pick];
		if (DropClass)
		{
			const FVector PickupLoc = ArenaCenter + DropSpawnOffset;
			World->SpawnActor<APickupBase>(DropClass, PickupLoc, FRotator::ZeroRotator);
		}
	}

	// Spawn exit portal
	if (World)
	{
		FVector PortalLoc = ArenaCenter + PortalSpawnOffset;
		ActivePortal = World->SpawnActor<AExitPortal>(
			AExitPortal::StaticClass(), PortalLoc, FRotator::ZeroRotator);
		if (ActivePortal)
		{
			ActivePortal->OnPortalActivated.AddDynamic(this, &ARoomManager::OnPortalActivated);
		}
	}
}

void ARoomManager::OnPortalActivated()
{
	if (CurrentState != ERoomState::Cleared) return;
	CurrentState = ERoomState::Transitioning;

	CleanupRoom();
	AdvanceToNextRoom();
}

void ARoomManager::CleanupRoom()
{
	// Destroy portal
	if (ActivePortal)
	{
		ActivePortal->Destroy();
		ActivePortal = nullptr;
	}

	// Destroy remaining enemy corpses
	for (AActor* Enemy : SpawnedEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}
	SpawnedEnemies.Empty();
}

void ARoomManager::AdvanceToNextRoom()
{
	// Teleport player back to arena center
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		PlayerPawn->SetActorLocation(ArenaCenter + FVector(0.f, 0.f, 100.f));
	}

	// Start next room after brief delay
	FTimerHandle NextRoomTimer;
	GetWorldTimerManager().SetTimer(NextRoomTimer, [this]()
	{
		StartRoom(CurrentRoomIndex + 1);
	}, 1.5f, false);
}
