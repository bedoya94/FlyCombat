//This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "GameFramework/GameMode.h"
#include "Enemy.h"
#include "Avatar.h"
#include "TutorialManager.h"
#include "FlyCombatGameMode.generated.h"
/**
*
*/
UCLASS()
class FLYCOMBAT_API AFlyCombatGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFlyCombatGameMode();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	TArray<AActor*> AllActors;
	UPROPERTY(EditAnywhere)
		TArray<AEnemy*> AllEnemies;

	UPROPERTY(EditAnywhere)
		TArray<AEnemy*> CloseEnemies;

	UPROPERTY(EditAnywhere)
		TArray<AEnemy*> FarEnemies;

	AEnemy* enemy;
	AAvatar* avatar;

	UPROPERTY(EditAnywhere)
		int32 enemiesDestroyed;

	UPROPERTY(EditAnywhere)
		int32 closeEnemiesThreshold; // how many enemies do we want close to the character

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float timeScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AEnemy> EnemyToSpawn;

	int32 closeEnemiesCount;//how many enemies we have close to the character
	float maxTicketTimer; // how many seconds till we repeat the process
	float tTimer;

	void GetAllEnemies();
	void TicketTimers();
	void TicketSystemAI();
	void FindWhoAttacks();

	void LogGameInfo();
};
