#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitPortal.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalActivated);

UCLASS()
class ROGUELIKEDEMO_API AExitPortal : public AActor
{
	GENERATED_BODY()

public:
	AExitPortal();

	UPROPERTY(BlueprintAssignable)
	FOnPortalActivated OnPortalActivated;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

private:
	FVector BaseLocation;
	bool bActivated = false;
};
