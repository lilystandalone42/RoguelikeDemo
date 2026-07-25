#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RoguelikeDemoPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class ARoguelikeDemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARoguelikeDemoPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
};
