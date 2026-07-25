#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "CritPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API ACritPickup : public APickupBase
{
	GENERATED_BODY()

public:
	ACritPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// Added crit chance (0.1 = 10%)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float CritBonus = 0.1f;
};
