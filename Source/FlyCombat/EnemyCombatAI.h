// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "Components/ActorComponent.h"
#include "EnemyCombatAI.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLYCOMBAT_API UEnemyCombatAI : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEnemyCombatAI();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	class AEnemy* MyOwner;
	class AAvatar* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		bool bIsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		bool bCanAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		float attackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		bool bHitting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		int32 attackNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		bool bIsUnblockable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyCombatProperties")
		bool bAttacking;

	float attackCoolDown;
	
	void Attack();

	UFUNCTION(BlueprintCallable, Category = "EnemyCombatProperties")
		void ResetCombo();

	TArray<AActor*> ThingsHit;

	USphereComponent* PunchCol;
	USphereComponent* KickCol;

	UFUNCTION(BlueprintNativeEvent, Category = "EnemyCombatProperties")
		void OnAttackEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
};

