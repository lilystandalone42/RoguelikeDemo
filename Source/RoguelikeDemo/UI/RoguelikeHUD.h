#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RoguelikeHUD.generated.h"

// Simple canvas-drawn HUD: health bar + live stat panel (attack, crit,
// lifesteal, speed). All stats reflect buffs the player has picked up.
UCLASS()
class ROGUELIKEDEMO_API ARoguelikeHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	// Draws one label line at (X, Y) and advances Y downward.
	void DrawStatLine(const FString& Text, const FLinearColor& Color, float X, float& Y);
};
