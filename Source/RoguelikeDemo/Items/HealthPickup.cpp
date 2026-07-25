#include "HealthPickup.h"
#include "Character/BaseCharacter.h"

AHealthPickup::AHealthPickup()
{
	// Bright green — reads as "heal"
	PickupColor = FLinearColor(0.1f, 0.9f, 0.2f);
	LabelColor = FColor::Green;
}

void AHealthPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->Heal(HealAmount);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up health (+%.0f)"),
			*Collector->GetName(), HealAmount);
	}
}

FString AHealthPickup::GetLabelText() const
{
	return FString::Printf(TEXT("HP +%.0f"), HealAmount);
}
