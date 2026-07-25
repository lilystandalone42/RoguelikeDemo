#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "SpeedPickup.generated.h"

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API ASpeedPickup : public APickupBase
{
	GENERATED_BODY()

public:
	ASpeedPickup();

protected:
	virtual void ApplyEffect(ABaseCharacter* Collector) override;
	virtual FString GetLabelText() const override;

	// Permanent bonus to max walk speed (cm/s)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpeedBonus = 80.f;
};
