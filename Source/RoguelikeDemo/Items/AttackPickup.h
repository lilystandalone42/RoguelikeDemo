#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AttackPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API AAttackPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AAttackPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// How much attack power this pickup grants (permanent for the run)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float AttackBonus = 10.f;
};
