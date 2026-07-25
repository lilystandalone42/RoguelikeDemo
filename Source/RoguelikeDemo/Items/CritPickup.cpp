#include "CritPickup.h"
#include "Character/BaseCharacter.h"

ACritPickup::ACritPickup()
{
	// Yellow — reads as "critical"
	PickupColor = FLinearColor(1.f, 0.85f, 0.1f);
	LabelColor = FColor::Yellow;
}

void ACritPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->AddCritChance(CritBonus);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up crit chance (+%.0f%%)"),
			*Collector->GetName(), CritBonus * 100.f);
	}
}

FString ACritPickup::GetLabelText() const
{
	return FString::Printf(TEXT("CRIT +%.0f%%"), CritBonus * 100.f);
}
