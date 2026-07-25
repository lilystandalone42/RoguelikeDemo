#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/EnemyAIController.h"
#include "Combat/CombatComponent.h"

#if WITH_EDITOR
#include "Materials/MaterialExpressionVectorParameter.h"
#endif

static UMaterial* GetPrototypeColorMaterial()
{
	static TWeakObjectPtr<UMaterial> CachedMat;
	if (CachedMat.IsValid())
		return CachedMat.Get();

#if WITH_EDITORONLY_DATA
	UMaterial* Mat = NewObject<UMaterial>(
		GetTransientPackage(), FName("M_PrototypeColor"), RF_Transient);
	Mat->AddToRoot();

	UMaterialExpressionVectorParameter* ColorParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	ColorParam->ParameterName = "Color";
	ColorParam->DefaultValue = FLinearColor(1.f, 1.f, 1.f, 1.f);

	Mat->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(ColorParam);
	Mat->GetEditorOnlyData()->BaseColor.Expression = ColorParam;
	Mat->PostEditChange();

	CachedMat = Mat;
	return Mat;
#else
	return UMaterial::GetDefaultMaterial(MD_Surface);
#endif
}

AEnemyBase::AEnemyBase()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// AI setup
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Movement config for AI navigation
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	// Hide default skeletal mesh
	GetMesh()->SetVisibility(false);

	// Red cube as placeholder enemy mesh
	TempMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempMesh"));
	TempMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (MeshAsset.Succeeded())
	{
		TempMesh->SetStaticMesh(MeshAsset.Object);
		TempMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.f));
		TempMesh->SetRelativeLocation(FVector(0.f, 0.f, -46.f));
		TempMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Combat component for enemy attacks
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	CombatComp->MeleeRange = 120.f;
	CombatComp->MeleeRadius = 70.f;
	CombatComp->MeleeAngle = 90.f;
	CombatComp->AttackDuration = 0.5f;
	CombatComp->AttackCooldown = 1.0f;
	CombatComp->KnockbackForce = 400.f;
	CombatComp->MaxComboSteps = 1;

	// Default enemy stats
	MaxHealth = 60.f;
	AttackPower = 10.f;
	Defense = 0.f;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (TempMesh)
	{
		UMaterial* ColorMat = GetPrototypeColorMaterial();
		if (ColorMat)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(ColorMat, this);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), EnemyColor);
			TempMesh->SetMaterial(0, DynamicMaterial);
		}
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		FlashDamage();
		UE_LOG(LogTemp, Warning, TEXT("%s took %.0f damage, HP: %.0f/%.0f"),
			*GetName(), ActualDamage, CurrentHealth, MaxHealth);
	}

	return ActualDamage;
}

void AEnemyBase::FlashDamage()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"),
			FLinearColor(1.f, 1.f, 1.f));
	}

	GetWorldTimerManager().ClearTimer(FlashTimerHandle);
	GetWorldTimerManager().SetTimer(FlashTimerHandle, this,
		&AEnemyBase::ResetFlash, 0.1f, false);
}

void AEnemyBase::ResetFlash()
{
	if (DynamicMaterial)
	{
		if (IsAlive())
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), EnemyColor);
		}
		else
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"),
				FLinearColor(0.2f, 0.2f, 0.2f));
		}
	}
}

void AEnemyBase::OnDied()
{
	Super::OnDied();

	UE_LOG(LogTemp, Warning, TEXT("%s died!"), *GetName());

	// Grey out and disable collision
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"),
			FLinearColor(0.2f, 0.2f, 0.2f));
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Destroy after delay
	SetLifeSpan(2.f);
}
