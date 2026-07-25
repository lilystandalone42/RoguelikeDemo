#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "HealthPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API AHealthPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// How much health this pickup restores
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float HealAmount = 30.f;
};
