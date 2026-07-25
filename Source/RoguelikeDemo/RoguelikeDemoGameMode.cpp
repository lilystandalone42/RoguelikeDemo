#include "RoguelikeDemoGameMode.h"
#include "RoguelikeDemoPlayerController.h"
#include "Character/PlayerCharacter.h"
#include "UI/RoguelikeHUD.h"
#include "UObject/ConstructorHelpers.h"

ARoguelikeDemoGameMode::ARoguelikeDemoGameMode()
{
	PlayerControllerClass = ARoguelikeDemoPlayerController::StaticClass();
	HUDClass = ARoguelikeHUD::StaticClass();
	// DefaultPawnClass is set in Blueprint (BP_PlayerCharacter)
	// so we don't hardcode it here
}
