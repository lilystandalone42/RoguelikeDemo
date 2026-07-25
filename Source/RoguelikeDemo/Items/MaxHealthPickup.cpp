#include "MaxHealthPickup.h"
#include "Character/BaseCharacter.h"

AMaxHealthPickup::AMaxHealthPickup()
{
	// Magenta/pink — reads as "vitality"
	PickupColor = FLinearColor(1.f, 0.3f, 0.7f);
	LabelColor = FColor(255, 90, 190);
}

void AMaxHealthPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->AddMaxHealth(MaxHealthBonus);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up max health (+%.0f)"),
			*Collector->GetName(), MaxHealthBonus);
	}
}

FString AMaxHealthPickup::GetLabelText() const
{
	return FString::Printf(TEXT("MAX HP +%.0f"), MaxHealthBonus);
}
