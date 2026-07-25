#include "LifestealPickup.h"
#include "Character/BaseCharacter.h"

ALifestealPickup::ALifestealPickup()
{
	// Crimson — reads as "blood / lifesteal"
	PickupColor = FLinearColor(0.7f, 0.03f, 0.12f);
	LabelColor = FColor(200, 20, 40);
}

void ALifestealPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->AddLifesteal(LifestealBonus);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up lifesteal (+%.0f%%)"),
			*Collector->GetName(), LifestealBonus * 100.f);
	}
}

FString ALifestealPickup::GetLabelText() const
{
	return FString::Printf(TEXT("LIFESTEAL +%.0f%%"), LifestealBonus * 100.f);
}
