#include "SpeedPickup.h"
#include "Character/BaseCharacter.h"

ASpeedPickup::ASpeedPickup()
{
	// Cyan — reads as "speed"
	PickupColor = FLinearColor(0.15f, 0.7f, 1.f);
	LabelColor = FColor::Cyan;
}

void ASpeedPickup::ApplyEffect(ABaseCharacter* Collector)
{
	if (Collector)
	{
		Collector->AddMoveSpeed(SpeedBonus);
		UE_LOG(LogTemp, Warning, TEXT("%s picked up move speed (+%.0f)"),
			*Collector->GetName(), SpeedBonus);
	}
}

FString ASpeedPickup::GetLabelText() const
{
	return FString::Printf(TEXT("SPD +%.0f"), SpeedBonus);
}
