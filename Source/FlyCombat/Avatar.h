// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#pragma once

#include "GameFramework/Character.h"
#include "Enemy.h"
#include "AvatarCombatComponent.h"
#include "Avatar.generated.h"

UCLASS()
class FLYCOMBAT_API AAvatar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAvatar();

	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	//Basic variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCamera")
		float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCamera")
		float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCamera")
		USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCamera")
		UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float CurUpAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float CurForwardAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float CurRightAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float launchForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bCanAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		int32 Combo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bChainAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool canTurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		TArray<UAnimMontage*> BasicAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		TArray<UAnimMontage*> PursuitAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		TArray<UAnimMontage*> FinisherAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
		TArray<UAnimMontage*> DefensiveAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bIsFinishingUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bIsFinishingDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bIsFinishingForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		bool bPursueAttack;

	float DashTime;
	float DashCoolDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
		bool bIsGuarding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
		bool bIsParrieng;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool bCanPursue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool bPursueing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool bInPursue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool launchToEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool InComboRush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		int32 pursueNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	class AEnemy* CurrentEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	class UAvatarCombatComponent* MyCombatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool bHitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		bool bCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
		int32 comboRushHits;

	//Damaging Setcion Begin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking")
		float dealDamage;

	float baseDamage;
	//Damaging Setcion End

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Properties")
		float HealthPoints;

	float maxHP;

	FVector CurrentControllerDirection, CurrentActorDirection;
	float zoomFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
		bool bIsDashing;

	bool bZooming, bIsFaster, bIsSpecialAttack;
	float animSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bPauseGame;

	UFUNCTION(BlueprintCallable, Category = "PauseGame")
		bool GamePaused();
	//Movement
	void MoveForward(float Axis);
	void MoveRight(float Axis);
	void MoveUp(float Axis);
	void Faster();
	void NotFaster();
	void Evading();
	void NotEvading();

	//Camera
	void TurnRate(float Rate);
	void LookUpRate(float Rate);
	void ZoomIn();
	void ZoomOut();

	//Combat
	void Attacking();
	void FinisherUp();
	void FinisherDown();
	void TakingDamage(float _damage);

	bool GetDirectionAxis();
	bool hitted;

	UFUNCTION(BlueprintCallable, Category = "Attacking")
		void ComboReset();

	UFUNCTION(BlueprintCallable, Category = "Attacking")
		void ComboChain();

	UFUNCTION(BlueprintCallable, Category = "Damaged")
		bool CheckHit();

	UFUNCTION(BlueprintCallable, Category = "Attacking")
		void PursueEnemyTarget();

	UFUNCTION(BlueprintCallable, Category = "Movement")
		void Launcher();

	//Defense
	void Dodge();
	void Dashing();
	void Guarding(float axis);
	void Parry();
	void NotGuarding();

	void CombatCameraHandler(float DeltaTime);
};