#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "LifestealPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API ALifestealPickup : public APickupBase
{
	GENERATED_BODY()

public:
	ALifestealPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// Fraction of damage returned as health (0.1 = 10%)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float LifestealBonus = 0.1f;
};
