#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "RangedEnemy.generated.h"

class AEnemyProjectile;

UCLASS(Blueprintable)
class ROGUELIKEDEMO_API ARangedEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ARangedEnemy();

	void FireProjectile();
};
