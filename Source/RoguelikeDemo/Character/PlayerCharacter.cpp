#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Combat/CombatComponent.h"

APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera/controller direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Character movement config - match original TopDown template
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->GravityScale = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->NavAgentProps.bCanJump = false;
	JumpMaxCount = 0;

	// Hide default skeletal mesh (no model assigned yet)
	GetMesh()->SetVisibility(false);

	TempMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempMesh"));
	TempMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
	if (MeshAsset.Succeeded())
	{
		TempMesh->SetStaticMesh(MeshAsset.Object);
		TempMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.f));
		TempMesh->SetRelativeLocation(FVector(0.f, 0.f, -46.f));
		TempMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Direction indicator - small cube on the front of the character
	DirectionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectionIndicator"));
	DirectionIndicator->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeAsset.Succeeded())
	{
		DirectionIndicator->SetStaticMesh(CubeAsset.Object);
		DirectionIndicator->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));
		DirectionIndicator->SetRelativeLocation(FVector(40.f, 0.f, -46.f));
		DirectionIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Camera
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;

	// Combat
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::OnDied()
{
	Super::OnDied();

	UE_LOG(LogTemp, Warning, TEXT("=== PLAYER DIED — restarting level in 3s ==="));

	// Freeze input and movement so the corpse doesn't keep sliding around
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	GetCharacterMovement()->DisableMovement();

	// On-screen notice (survives the Tick early-out that hides the health bar)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
			TEXT("YOU DIED — Restarting..."));
	}

	// Reload the current level for a fresh run
	FTimerHandle RestartTimer;
	GetWorldTimerManager().SetTimer(RestartTimer, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
	}, 3.f, false);
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInput->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::Dash);
		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack);
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DashCooldownRemaining > 0.f)
	{
		DashCooldownRemaining -= DeltaSeconds;
	}

	if (bIsDashing)
	{
		DashTimeRemaining -= DeltaSeconds;
		if (DashTimeRemaining <= 0.f)
		{
			OnDashEnd();
		}
		else
		{
			const float DashSpeed = DashDistance / DashDuration;
			FVector DashVelocity = DashDirection * DashSpeed;
			GetCharacterMovement()->Velocity = FVector(DashVelocity.X, DashVelocity.Y, 0.f);
		}
	}

	if (!bIsDashing)
	{
		RotateTowardsCursor();
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDashing) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Get camera-relative directions
	const FRotator CameraRotation = CameraBoom->GetComponentRotation();
	const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector MoveDirection = (ForwardDir * MovementVector.Y + RightDir * MovementVector.X).GetSafeNormal();
	AddMovementInput(MoveDirection, MoveSpeedMultiplier);
}

void APlayerCharacter::Dash()
{
	if (bIsDashing || DashCooldownRemaining > 0.f) return;

	// Dash in current movement direction, or forward if standing still
	FVector Velocity = GetCharacterMovement()->Velocity;
	if (Velocity.SizeSquared() > 10.f)
	{
		DashDirection = FVector(Velocity.X, Velocity.Y, 0.f).GetSafeNormal();
	}
	else
	{
		FVector Fwd = GetActorForwardVector();
		DashDirection = FVector(Fwd.X, Fwd.Y, 0.f).GetSafeNormal();
	}

	bIsDashing = true;
	bIsInvincible = true;
	DashTimeRemaining = DashDuration;
}

void APlayerCharacter::OnDashEnd()
{
	bIsDashing = false;
	bIsInvincible = false;
	DashCooldownRemaining = DashCooldown;
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
}

void APlayerCharacter::Attack()
{
	if (bIsDashing) return;
	if (CombatComp)
	{
		CombatComp->MeleeAttack();
	}
}

void APlayerCharacter::RotateTowardsCursor()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		FVector Direction = HitResult.ImpactPoint - GetActorLocation();
		Direction.Z = 0.f;

		if (Direction.SizeSquared() > 25.f) // Avoid jitter when cursor is on character
		{
			FRotator TargetRotation = Direction.Rotation();
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 15.f);
			SetActorRotation(FRotator(0.f, NewRotation.Yaw, 0.f));
		}
	}
}
