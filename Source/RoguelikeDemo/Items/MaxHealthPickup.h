#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "MaxHealthPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API AMaxHealthPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AMaxHealthPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// Permanent bonus to max health (also granted as current health)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float MaxHealthBonus = 25.f;
};
