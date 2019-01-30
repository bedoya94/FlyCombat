// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#include "FlyCombat.h"
#include "Enemy.h"
#include "Avatar.h"
#include "EnemyAI.h"
#include "FlyCombatGameMode.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	stuntTest = false;
	speed = 20.f;
	maxHP = 450.f;
	hp = maxHP;
	exp = 0;
	launched = false;

	SightSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SightSphere"));
	SightSphere->SetSphereRadius(300.f);
	SightSphere->AttachTo(RootComponent);

	stuntCoolDown = 2.f;
	stuntTimer = 0.f;
	timesHitted = 0;
	bHitted = false;
	bTurnToAttack = false;

	MyCombatComponent = CreateDefaultSubobject<UEnemyCombatAI>(TEXT("CombatComponent"));
	this->AddOwnedComponent(MyCombatComponent);

	MyCombatComponent->PunchCol->AttachTo(GetMesh(), "RightHandSocket");
	MyCombatComponent->KickCol->AttachTo(GetMesh(), "LeftFootSocket");

	bParried = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AAvatar>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0.f));
	GetCharacterMovement()->MaxFlySpeed = 1000.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 4096.f;
	destructionTimer = 0.f;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetCharacterMovement()->MovementMode = MOVE_Flying;

	if (!this->IsPendingKill())
	{
		if (!Player) { return; }

		if (EnemyState == EEnemyState::ST_Dead)
		{
			destructionTimer += DeltaTime;

			Player->MyCombatComponent->EnemiesDistances.Empty();
			Player->MyCombatComponent->EnemiesDirection.Empty();
			Player->MyCombatComponent->CloserEnemies.Empty();
			Player->MyCombatComponent->TargetEnemies.Empty();
			Player->MyCombatComponent->Enemies.Empty();
			Player->MyCombatComponent->numEnemies--;


			if (destructionTimer > 3.f)
			{
				AFlyCombatGameMode* const MyGM = Cast<AFlyCombatGameMode>(GetWorld()->GetAuthGameMode());
				if (MyGM)
				{
					MyGM->enemiesDestroyed++;
				}

				GLog->Log("Destroyed");
				Destroy();
			}
		}

		if ((Player->CurrentEnemy == this && Player->bCanAttack) || launched || bHitted || stunt || MyCombatComponent->bIsAttacking || Player->bPursueing)
		{
			bCanMove = false;
		}
		else
		{
			bCanMove = true;
		}

		if (EnemyState == EEnemyState::ST_Attacked)
		{
			MyCombatComponent->bCanAttack = false;
			//bTurnToAttack = false;
		}

		SwitchStates();

		if (launched)
		{
			bParried = false;

			if (!bIsLaunched && GetCharacterMovement()->Velocity.IsZero())
			{
				if (Player->bPursueing || Player->bPursueAttack)
				{
					PlayAnimMontage(DamageAnims[5]);
				}
				else
				{
					if (stuntTest)
						stunt = true;
					else
						PlayAnimMontage(DamageAnims[5]);
				}

				stuntTest = false;
				launched = false;
			}
		}

		{
			if (bParried)
			{
				stuntCoolDown = 1.5f;
			}
			else if (Player->Combo >= 4)
			{
				stuntCoolDown = 3.5f;
			}

			if (!stunt)
			{
				stuntTimer = stuntCoolDown;
			}

			if (stunt)
			{
				stuntTimer -= DeltaTime;
				MyCombatComponent->bIsAttacking = false;
				MyCombatComponent->bAttacking = false;
				bParried = false;

				if (stuntTimer <= 0.f)
					stunt = false;
			}
		}

		{
			if (bHitted)
			{
				if (Player->Combo >= 4 && !Player->bPursueAttack && Player->CurrentEnemy == this)
				{
					launched = true;
					bIsLaunched = true;
					stuntTest = true;
				}
			}

			if (bIsLaunched)
			{
				if (Player->bIsFinishingForward)
					bLaunch = true;
				else if (Player->bIsFinishingDown)
					bLaunchDown = true;
				else if (Player->bIsFinishingUp)
					bLaunchUp = true;
			}
			else
			{
				bLaunch = false;
				bLaunchDown = false;
				bLaunchUp = false;
			}
		}

		if (bIsLaunched)
		{
			Launched();
		}

		if (Player->bCounter)
		{
			FVector myLoc = this->GetActorLocation();
			FVector targetLoc = Player->GetActorLocation();

			float dist = FVector::Dist(myLoc, targetLoc);

			if (dist < 180.f)
			{
				bParried = true;
			}
		}
	}

}

void AEnemy::Launched()
{
	FVector DirLaunch = FVector::ZeroVector;
	bool XY = false, Z = false;

	if (bLaunch)
	{
		if (Player->bCanAttack && Player->bCounter)
		{
			DirLaunch = GetActorForwardVector() * -10000.f;
			XY = false;
			Z = true;
		}
		else
		{
			DirLaunch = GetActorForwardVector() * -20000.f;
			XY = false;
			Z = true;
		}
	}
	else if (bLaunchUp)
	{
		DirLaunch = ((GetActorUpVector() * 10000.f) + (GetActorForwardVector() * -17500.f));
		XY = false;
		Z = false;
	}
	else if (bLaunchDown)
	{
		DirLaunch = ((GetActorUpVector() * -10000.f) + (GetActorForwardVector() * -17500.f));
		XY = false;
		Z = false;
	}

	LaunchCharacter((DirLaunch * GetWorld()->GetDeltaSeconds()), XY, Z);
}

void AEnemy::SwitchStates()
{
	if (hp <= 0)
	{
		EnemyState = EEnemyState::ST_Dead;
	}
	else
	{
		if ((Player->CurrentEnemy == this && (Player->Combo > 1 || Player->pursueNum > 0)) || launched)
		{
			EnemyState = EEnemyState::ST_Attacked;
		}
		else if (MyCombatComponent->bCanAttack || MyCombatComponent->bAttacking)
		{
			EnemyState = EEnemyState::ST_Attacking;
		}
		else
		{
			EnemyState = EEnemyState::ST_InCombat;
		}
	}

}

void AEnemy::NextCounterAction()
{
	if (Player->bCanAttack && Player->bCounter)
	{
		launched = true;
		bIsLaunched = true;
		bLaunch = true;
		MyCombatComponent->bCanAttack = false;
		TakingDamage(Player->MyCombatComponent->InflictDamage());
	}
	else if (!Player->bCanAttack && !Player->bCounter)
	{
		bParried = false;
		MyCombatComponent->bIsAttacking = false;
		MyCombatComponent->bAttacking = false;
		bCanMove = true;
	}
}

void AEnemy::TakingDamage(float _damage)
{
	hp -= _damage;

	if (hp <= 0.f)
	{
		hp = 0.f;
	}
}