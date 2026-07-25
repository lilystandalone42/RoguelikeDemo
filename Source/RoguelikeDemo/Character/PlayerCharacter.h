#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UCombatComponent;

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const { return TopDownCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	virtual void BeginPlay() override;
	virtual void OnDied() override;

	// Input actions
	void Move(const FInputActionValue& Value);
	void Dash();
	void OnDashEnd();

	// Rotate character to face mouse cursor
	void RotateTowardsCursor();

	void Attack();

	// Camera
	// Temporary mesh for testing (replace with skeletal mesh later)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* TempMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* DirectionIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	// Dash parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashCooldown = 0.8f;

private:
	bool bIsDashing = false;
	float DashCooldownRemaining = 0.f;
	FVector DashDirection;
	float DashTimeRemaining = 0.f;

	FTimerHandle DashTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComp;
};
