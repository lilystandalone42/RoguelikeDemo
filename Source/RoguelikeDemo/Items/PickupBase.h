#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USphereComponent;
class ABaseCharacter;

// Base class for all floating pickups (health, buffs, etc.).
// Subclasses override ApplyEffect to define what grabbing it does.
UCLASS(Abstract)
class ROGUELIKEDEMO_API APickupBase : public AActor
{
	GENERATED_BODY()

public:
	APickupBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// What happens when the player grabs this pickup — override in subclasses.
	virtual void ApplyEffect(ABaseCharacter* Collector) {}

	// Text shown floating above the pickup (e.g. "HP +30")
	virtual FString GetLabelText() const { return TEXT("Pickup"); }

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	// Visual tint applied to the mesh
	UPROPERTY(EditAnywhere, Category = "Pickup")
	FLinearColor PickupColor = FLinearColor(1.f, 1.f, 1.f);

	// Color of the floating debug label
	UPROPERTY(EditAnywhere, Category = "Pickup")
	FColor LabelColor = FColor::White;

private:
	FVector BaseLocation;
	bool bCollected = false;
};
