#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "EnemyBase.generated.h"

class UCombatComponent;

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API AEnemyBase : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnDied() override;

	// Temporary mesh for visualization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* TempMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor EnemyColor = FLinearColor(0.8f, 0.1f, 0.1f);

private:
	// Flash red on hit
	void FlashDamage();
	void ResetFlash();

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	FTimerHandle FlashTimerHandle;
};
