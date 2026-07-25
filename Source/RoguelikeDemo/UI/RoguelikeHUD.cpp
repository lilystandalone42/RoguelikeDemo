#include "RoguelikeHUD.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ARoguelikeHUD::DrawStatLine(const FString& Text, const FLinearColor& Color, float X, float& Y)
{
	DrawText(Text, Color, X, Y, GEngine ? GEngine->GetMediumFont() : nullptr, 1.15f);
	Y += 24.f;
}

void ARoguelikeHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	ABaseCharacter* Player = Cast<ABaseCharacter>(PlayerOwner->GetPawn());
	if (!Player)
	{
		return;
	}

	const float Margin = 40.f;
	const float X = Margin;
	float Y = Margin;

	// --- Health bar ---
	const float BarW = 320.f;
	const float BarH = 26.f;
	const float Pct = FMath::Clamp(Player->GetHealthPercent(), 0.f, 1.f);

	// Border + empty background
	DrawRect(FLinearColor(0.03f, 0.03f, 0.03f, 0.85f), X - 3.f, Y - 3.f, BarW + 6.f, BarH + 6.f);
	DrawRect(FLinearColor(0.25f, 0.05f, 0.05f, 0.9f), X, Y, BarW, BarH);

	// Fill — green / yellow / red by remaining health
	const FLinearColor FillColor =
		Pct > 0.5f ? FLinearColor(0.1f, 0.8f, 0.2f) :
		(Pct > 0.25f ? FLinearColor(0.9f, 0.8f, 0.1f) : FLinearColor(0.9f, 0.15f, 0.1f));
	DrawRect(FillColor, X, Y, BarW * Pct, BarH);

	// HP number
	const FString HPText = FString::Printf(TEXT("HP  %.0f / %.0f"),
		Player->GetCurrentHealth(), Player->GetMaxHealth());
	DrawText(HPText, FLinearColor::White, X + 10.f, Y + 4.f,
		GEngine ? GEngine->GetMediumFont() : nullptr, 1.1f);

	// --- Stat panel ---
	Y += BarH + 18.f;

	float Speed = 0.f;
	if (UCharacterMovementComponent* Move = Player->GetCharacterMovement())
	{
		Speed = Move->MaxWalkSpeed;
	}

	DrawStatLine(FString::Printf(TEXT("ATK          %.0f"), Player->GetAttackPower()),
		FLinearColor(1.f, 0.5f, 0.1f), X, Y);
	DrawStatLine(FString::Printf(TEXT("CRIT         %.0f%%"), Player->GetCritChance() * 100.f),
		FLinearColor(1.f, 0.85f, 0.1f), X, Y);
	DrawStatLine(FString::Printf(TEXT("LIFESTEAL    %.0f%%"), Player->GetLifesteal() * 100.f),
		FLinearColor(0.9f, 0.25f, 0.35f), X, Y);
	DrawStatLine(FString::Printf(TEXT("SPEED        %.0f"), Speed),
		FLinearColor(0.2f, 0.7f, 1.f), X, Y);
}
