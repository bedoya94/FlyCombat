//This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "Components/ActorComponent.h"
#include "Enemy.h"
#include "AvatarCombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLYCOMBAT_API UAvatarCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAvatarCombatComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	USphereComponent* CombatSphere;

	class AAvatar* Player;

	class AEnemy* CurrentTarget;


	TArray<AActor*>CollectedEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<AActor*>CloserEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<AActor*>TargetEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float>EnemiesDistances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float>EnemiesDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<AEnemy*>Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool inCombat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float rushTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 numEnemies;

	void GetCloserEnemy();
	void GetTargetEnemy();
	void GetCurrentTarget();
	void CombatMoveLogic(AAvatar* _Player, float _DeltaTime);
	void GuardMovementLogic();

	//Attacks Hit Colliders Section Begin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<AActor*> ThingsHit;

	float rushCount;

	float attackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bPunching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAttacking;

	USphereComponent* RightFist;
	USphereComponent* LeftFist;
	USphereComponent* RightKick;
	USphereComponent* LeftKick;

	UFUNCTION(BlueprintCallable, Category = "AttackComponent")
		void Punch();

	UFUNCTION(BlueprintCallable, Category = "AttackComponent")
		void Rest();

	UFUNCTION(BlueprintNativeEvent, Category = "Collision")
		void OnAttackingEnter(AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	//Attacks Hit Colliders Section End

	float InflictDamage();
};
