#include "AttackPickup.h"
#include "Character/BaseCharacter.h"

AAttackPickup::AAttackPickup()
{
	// Orange-red — reads as "power / damage"
	PickupColor = FLinearColor(1.f, 0.4f, 0.05f);
	LabelColor = FColor::Orange;
}

void AAttackPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->AddAttackPower(AttackBonus);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up attack boost (+%.0f, now %.0f)"),
			*Collector->GetName(), AttackBonus, Collector->GetAttackPower());
	}
}

FString AAttackPickup::GetLabelText() const
{
	return FString::Printf(TEXT("ATK +%.0f"), AttackBonus);
}
