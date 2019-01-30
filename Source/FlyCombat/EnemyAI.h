// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "AIController.h"
#include "Enemy.h"
#include "EnemyAI.generated.h"

/**
*
*/
UCLASS()
class FLYCOMBAT_API AEnemyAI : public AAIController
{
	GENERATED_BODY()

public:

	AEnemyAI();

	void virtual BeginPlay() override;

	void virtual Tick(float DeltaSeconds) override;

	class AAvatar* Target;
	AEnemy* ControlledEnemy;

	//Vectors to move the enemy towards a target
	FVector targetEnemy;
	FVector targetDestination;

	UPROPERTY(EditAnywhere)
		FVector targetDesPos;

	float distanceThreshold;
	float stoppingDist;
	float evadeTime;
	bool hasDestination;

	UPROPERTY(EditAnywhere)
		float evadeT;

	//Function for the movement logic
	void FindDestination();
	void LookAtTarget();
	void NavigationAI();
	void WaitingToAttack();
	FVector CreateDesPos();
};

