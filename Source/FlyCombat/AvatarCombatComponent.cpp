//This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#include "FlyCombat.h"
#include "Avatar.h"
#include "AvatarCombatComponent.h"


// Sets default values for this component's properties
UAvatarCombatComponent::UAvatarCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetSphereRadius(2000.f);

	inCombat = false;
	numEnemies = 0;

	RightFist = CreateDefaultSubobject<USphereComponent>(TEXT("RightFist"));
	RightFist->SetSphereRadius(45.f);

	LeftFist = CreateDefaultSubobject<USphereComponent>(TEXT("LeftFist"));
	LeftFist->SetSphereRadius(45.f);

	RightKick = CreateDefaultSubobject<USphereComponent>(TEXT("RightKick"));
	RightKick->SetSphereRadius(45.f);

	LeftKick = CreateDefaultSubobject<USphereComponent>(TEXT("LeftKick"));
	LeftKick->SetSphereRadius(45.f);

	attackDamage = 1.f;
	bPunching = false;
	rushTimer = 3.f;
	rushCount = 0.f;

	RightFist->OnComponentBeginOverlap.AddDynamic(this, &UAvatarCombatComponent::OnAttackingEnter);
	LeftFist->OnComponentBeginOverlap.AddDynamic(this, &UAvatarCombatComponent::OnAttackingEnter);
	RightKick->OnComponentBeginOverlap.AddDynamic(this, &UAvatarCombatComponent::OnAttackingEnter);
	LeftKick->OnComponentBeginOverlap.AddDynamic(this, &UAvatarCombatComponent::OnAttackingEnter);
}

// Called when the game starts
void UAvatarCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	numEnemies = 0;

	Player = Cast<AAvatar>(GetOwner());

}


// Called every frame
void UAvatarCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	CombatSphere->GetOverlappingActors(CollectedEnemies, AEnemy::StaticClass());

	if (!Player) { return; }

	{
		if (CollectedEnemies.Num() > 0)
		{
			for (int32 i = 0; i < CollectedEnemies.Num(); i++)
			{
				AEnemy* const curEnemy = Cast<AEnemy>(CollectedEnemies[i]);
				if (curEnemy)
				{
					if (!Enemies.Contains(curEnemy))
					{
						if (curEnemy->EnemyState != EEnemyState::ST_Dead)
							Enemies.Add(curEnemy);
					}
				}
			}

			numEnemies = Enemies.Num();

			if (numEnemies <= 0)
			{
				numEnemies = 0;
			}
		}

		if (numEnemies > 0)
			inCombat = true;
		else if (numEnemies <= 0)
			inCombat = false;

		if (inCombat)
		{
			EnemiesDistances.SetNum(numEnemies);
			EnemiesDirection.SetNum(numEnemies);
			CloserEnemies.SetNum(numEnemies);
			TargetEnemies.SetNum(numEnemies);

			if (numEnemies - 1 < CollectedEnemies.Num())
			{
				GetCloserEnemy();

				if (!bAttacking && Player->Combo < 4 && Player->pursueNum == 0)
					GetTargetEnemy();
			}

			GetCurrentTarget();

			if (Player->CurrentEnemy)
				CombatMoveLogic(Player, DeltaTime);
		}

		{
			if (Player->bInPursue)
			{
				Player->comboRushHits = 0;
				rushCount = 0.f;
				rushTimer = 3.f;
			}

			if (Player->Combo == 2 && Player->comboRushHits > 9)
			{
				Player->comboRushHits = 0;
				Player->InComboRush = true;
				rushCount = 0.f;
				rushTimer = 3.f;
			}

			if (Player->InComboRush)
			{
				rushTimer -= DeltaTime * (rushCount / Player->pursueNum);
				if (rushTimer < 0.f)
				{
					Player->comboRushHits = 0.f;
					Player->InComboRush = false;
					Player->Combo = 2.f;
					rushTimer = 0.f;
				}
			}

			if (Player->pursueNum > 0)
			{
				if (Player->Combo == 3)
				{
					if (Player->CurrentEnemy)
					{
						if (Player->CurrentEnemy->bHitted)
						{
							UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);
						}
					}
				}
			}

			if (Player->Combo >= 4)
			{
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			}
		}
	}

	GuardMovementLogic();

	if (Player->Combo < 4)
	{
		EnemiesDistances.Empty();
		EnemiesDirection.Empty();
		CloserEnemies.Empty();
		TargetEnemies.Empty();
		Enemies.Empty();
	}

}

void UAvatarCombatComponent::GetCloserEnemy()
{
	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		if (Enemies[i])
			EnemiesDistances[i] = FVector::Dist(GetOwner()->GetActorLocation(), Enemies[i]->GetActorLocation());
	}


	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		for (int32 j = i + 1; j < Enemies.Num(); j++)
		{
			if (EnemiesDistances[i] > EnemiesDistances[j])
			{
				float aux;
				aux = EnemiesDistances[j];
				EnemiesDistances[j] = EnemiesDistances[i];
				EnemiesDistances[i] = aux;
			}
		}
	}

	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		for (int32 j = 0; j < Enemies.Num(); j++)
		{
			if (FVector::Dist(Player->GetActorLocation(), Enemies[j]->GetActorLocation()) == EnemiesDistances[i])
				CloserEnemies[i] = Enemies[j];
		}
	}
}

void UAvatarCombatComponent::GetTargetEnemy()
{
	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		if (Enemies[i])
		{
			FVector Direction = (Enemies[i]->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
			float dir = FVector::DotProduct(Direction, Player->CurrentControllerDirection);
			EnemiesDirection[i] = dir;
		}
	}

	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		for (int32 j = i + 1; j < Enemies.Num(); j++)
		{
			if (EnemiesDirection[i] < EnemiesDirection[j])
			{
				float aux;
				aux = EnemiesDirection[j];
				EnemiesDirection[j] = EnemiesDirection[i];
				EnemiesDirection[i] = aux;
			}
		}
	}

	for (int32 i = 0; i < Enemies.Num(); i++)
	{
		for (int32 j = 0; j < Enemies.Num(); j++)
		{
			FVector Direction = (Enemies[j]->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
			float dir = FVector::DotProduct(Direction, Player->CurrentControllerDirection);

			if (dir == EnemiesDirection[i])
			{
				TargetEnemies[i] = Enemies[j];
			}
		}
	}
}

void UAvatarCombatComponent::GetCurrentTarget()
{

	if (!bAttacking  && Player->Combo < 4 && Player->pursueNum == 0)
	{
		if (Enemies.Num() > 0 && !Player->GetDirectionAxis())
		{
			AEnemy* const enemy = Cast<AEnemy>(CloserEnemies[0]);

			if (enemy)
			{
				if (!enemy->stunt && EnemiesDistances[0] < 500.f && enemy->EnemyState != EEnemyState::ST_Dead && (!enemy->launched && !Player->bPursueing))
					CurrentTarget = enemy;
				else
					CurrentTarget = nullptr;
			}
		}
		else if (Enemies.Num() > 0 && Player->GetDirectionAxis())
		{
			AEnemy* const enemy = Cast<AEnemy>(TargetEnemies[0]);

			if (enemy)
			{
				FVector myLocation = Player->GetActorLocation();
				FVector enemyLocation = enemy->GetActorLocation();
				float dist = FVector::Dist(myLocation, enemyLocation);

				if (!enemy->stunt && EnemiesDirection[0] > 0.f && dist < 2500.f && enemy->EnemyState != EEnemyState::ST_Dead && (!enemy->launched && !Player->bPursueing))
					CurrentTarget = enemy;
				else
					CurrentTarget = nullptr;
			}
		}
		Player->CurrentEnemy = CurrentTarget;
	}
}

void UAvatarCombatComponent::CombatMoveLogic(AAvatar* _Player, float _DeltaTime)
{
	if (!Player->bIsDashing && !Player->bIsGuarding)
	{
		FVector toEnemy = _Player->CurrentEnemy->GetActorLocation() - _Player->GetActorLocation();
		FRotator toEnemyRot = toEnemy.Rotation();
		if(!_Player->bInPursue)
			toEnemyRot.Pitch = 0.f;
		_Player->GetRootComponent()->SetWorldRotation(toEnemyRot);
	}

	FVector myLocation = _Player->GetActorLocation();
	FVector enemyLocation = _Player->CurrentEnemy->GetActorLocation();
	float dist = FVector::Dist(myLocation, enemyLocation);

	if (_Player->bCanAttack && dist < 80.f)
	{
		FVector NewAttackLocation = _Player->GetActorLocation();
		NewAttackLocation.Z = _Player->CurrentEnemy->GetActorLocation().Z - 5.f;

		if(_Player->bPursueAttack)
			NewAttackLocation.X = _Player->CurrentEnemy->GetActorLocation().X - 70.f;

		_Player->SetActorLocation(NewAttackLocation);
	}

	if (!_Player->CurrentEnemy->launched)
	{
		if (_Player->launchToEnemy)
		{
			_Player->SetActorLocation(FMath::Lerp(myLocation, enemyLocation, 10.f * _DeltaTime));

			if (dist < 80.f)
			{
				_Player->launchToEnemy = false;
				_Player->bCanPursue = false;
				_Player->bPursueing = false;
			}
		}
	}

	if (_Player->bInPursue && dist < 80.f)
	{
		_Player->bPursueing = false;
		_Player->bInPursue = false;

		if (_Player->Combo >= 4)
		{
			if (_Player->bPursueAttack)
			{
				_Player->PlayAnimMontage(_Player->PursuitAttacks[0]);
			}
			else
			{
				_Player->PlayAnimMontage(_Player->DefensiveAnims[5]);
			}
		}
	}
}

void UAvatarCombatComponent::OnAttackingEnter_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AEnemy* const Target = Cast<AEnemy>(OtherActor);

	if (OtherComponent != OtherActor->GetRootComponent())
	{
		return;
	}

	if (!Target)
	{
		return;
	}

	if (bPunching && OtherActor != Player && !ThingsHit.Contains(OtherActor))
	{
		ThingsHit.Add(OtherActor);

		if (!Target->MyCombatComponent->bIsAttacking && !Target->stunt)
		{
			Target->timesHitted++;
			Target->bHitted = true;
			Target->TakingDamage(InflictDamage());
		}
	}
}

void UAvatarCombatComponent::Punch()
{
	ThingsHit.Empty();
	bPunching = true;
}

void UAvatarCombatComponent::Rest()
{
	ThingsHit.Empty();
	bPunching = false;
	bAttacking = false;
	if (Player)
	{
		if (Player->CurrentEnemy)
			Player->CurrentEnemy->bHitted = false;
	}

}

float UAvatarCombatComponent::InflictDamage()
{
	float damage = 0.f;
	float curDamage = 0.f;
	float dealtDamage = 0.f;

	if (Player)
	{
		damage = Player->dealDamage;

		{
			if (Player->bCounter)
			{
				dealtDamage = 50.f;
			}
			else if (Player->bCanAttack && (Player->Combo <= 3 || Player->bPursueAttack))
			{
				curDamage = FMath::FRandRange(damage - (damage * 0.25f), damage + (damage * 0.25f));
				dealtDamage = curDamage;
			}
			else if (Player->InComboRush)
			{
				damage = damage / 2.f;
				curDamage = FMath::FRandRange(damage - (damage * 0.25f), damage + (damage * 0.25f));
				dealtDamage = curDamage;
			}
			else if (Player->Combo >= 4)
			{
				curDamage = (damage + damage * 0.25) * (Player->pursueNum + 1);
				dealtDamage = curDamage;
			}
		}
	}

	return dealtDamage;
	//return 1.f;
}

void UAvatarCombatComponent::GuardMovementLogic()
{
	if (Player->bIsGuarding && Player->GetDirectionAxis() == true)
	{
	Player->bUseControllerRotationPitch = false;
	Player->bUseControllerRotationYaw = true;
	Player->bUseControllerRotationRoll = false;
	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else if (Player->bIsGuarding && Player->GetDirectionAxis() == false)
	{
	Player->bUseControllerRotationPitch = false;
	Player->bUseControllerRotationYaw = false;
	Player->bUseControllerRotationRoll = false;

	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		Player->bUseControllerRotationPitch = false;
		Player->bUseControllerRotationYaw = false;
		Player->bUseControllerRotationRoll = false;

		Player->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}