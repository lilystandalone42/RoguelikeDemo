#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomManager.generated.h"

class AEnemyBase;
class AExitPortal;
class APickupBase;

UENUM(BlueprintType)
enum class ERoomState : uint8
{
	WaitingToStart,
	Combat,
	Cleared,
	Transitioning
};

USTRUCT(BlueprintType)
struct FRoomWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MeleeEnemyCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RangedEnemyCount = 0;
};

UCLASS()
class ROGUELIKEDEMO_API ARoomManager : public AActor
{
	GENERATED_BODY()

public:
	ARoomManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Root so the actor has a Transform and can be moved in the editor
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

#if WITH_EDITORONLY_DATA
	// Editor-only icon so it's visible/selectable in the viewport
	UPROPERTY()
	class UBillboardComponent* EditorIcon;
#endif

private:
	// Room configurations — each entry = one room
	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<FRoomWaveConfig> RoomConfigs;

	// Arena settings
	UPROPERTY(EditAnywhere, Category = "Arena")
	float SpawnRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Arena")
	FVector PortalSpawnOffset = FVector(600.f, 0.f, 50.f);

	// Reward pool — one is picked at random and dropped when a room is cleared.
	// Add more pickup subclasses here to expand the loot table.
	UPROPERTY(EditAnywhere, Category = "Rewards")
	TArray<TSubclassOf<APickupBase>> PossibleDrops;

	UPROPERTY(EditAnywhere, Category = "Rewards")
	FVector DropSpawnOffset = FVector(-400.f, 0.f, 80.f);

	// State
	ERoomState CurrentState = ERoomState::WaitingToStart;
	int32 CurrentRoomIndex = 0;
	int32 EnemiesAlive = 0;
	FVector ArenaCenter;

	UPROPERTY()
	AExitPortal* ActivePortal = nullptr;

	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;

	// Core flow
	void StartRoom(int32 RoomIndex);
	void SpawnEnemiesForRoom(const FRoomWaveConfig& Config);
	FVector GetRandomSpawnPoint() const;

	UFUNCTION()
	void OnEnemyDied();

	void OnRoomCleared();

	UFUNCTION()
	void OnPortalActivated();

	void CleanupRoom();
	void AdvanceToNextRoom();
};
