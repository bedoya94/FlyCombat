// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "GameFramework/Character.h"
#include "EnemyCombatAI.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyState :uint8
{
	ST_Patrol,
	ST_InCombat,
	ST_Attacking,
	ST_Attacked,
	ST_Pursue,
	ST_Dead,
	ST_None
};

UCLASS()
class FLYCOMBAT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		float speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		float hp;

	float maxHP;

	float destructionTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		int32 exp;

	USphereComponent* SightSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool stunt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool launched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bIsLaunched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bHitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bTurnToAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		int32 timesHitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		UEnemyCombatAI* MyCombatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		TArray<UAnimMontage*> BasicAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		TArray<UAnimMontage*> DamageAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bLaunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bLaunchDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bLaunchUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		bool bParried;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyProperties")
		EEnemyState EnemyState;

	UFUNCTION(BlueprintCallable, Category = "Defense")
		void NextCounterAction();

	float stuntTimer, stuntCoolDown;
	bool stuntTest;
	class AAvatar* Player;

	void SwitchStates();
	void Launched();

	void TakingDamage(float _damage);
};
