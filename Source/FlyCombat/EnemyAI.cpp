// This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#include "FlyCombat.h"
#include "Avatar.h"
#include "EnemyAI.h"
#include "FlyCombatGameMode.h"

AEnemyAI::AEnemyAI()
{
	evadeTime = 1.f;
	evadeT = 0.f;

	hasDestination = false;
}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();

	ControlledEnemy = Cast<AEnemy>(GetPawn());

	if (ControlledEnemy)
	{
		if (ControlledEnemy->Player)
		{
			Target = Cast<AAvatar>(ControlledEnemy->Player);
		}

		targetDesPos = ControlledEnemy->GetActorLocation();
	}
}

void AEnemyAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ControlledEnemy->EnemyState != EEnemyState::ST_Dead)
	{
		FindDestination();
		LookAtTarget();
		NavigationAI();

		if (ControlledEnemy->bCanMove)
		{
			MoveToLocation(targetDesPos, stoppingDist, true, false);
		}
		else
		{
			StopMovement();
		}

		{
			if (!ControlledEnemy->stunt && !ControlledEnemy->launched && ControlledEnemy->EnemyState != EEnemyState::ST_Attacked && !ControlledEnemy->MyCombatComponent->bIsAttacking)
			{
				FVector NewPos = ControlledEnemy->GetActorLocation();
				NewPos.Z = FMath::Lerp(NewPos.Z, ControlledEnemy->Player->GetActorLocation().Z, 8.f * DeltaSeconds);
				ControlledEnemy->SetActorLocation(NewPos);
			}
		}
	}
}

void AEnemyAI::FindDestination()
{
	float distanceFromTarget = FVector::Dist(ControlledEnemy->GetActorLocation(), targetEnemy);
	AFlyCombatGameMode* const MyGM = Cast<AFlyCombatGameMode>(GetWorld()->GetAuthGameMode());

	if (MyGM)
	{
		if (ControlledEnemy->MyCombatComponent->bCanAttack)
		{
			targetDesPos = ControlledEnemy->Player->GetActorLocation();

			stoppingDist = 20.f;
		}
		else
		{
			if (ControlledEnemy->bTurnToAttack/*MyGM->FarEnemies.Contains(ControlledEnemy)*/)
			{
				stoppingDist = 500.f;
			}
			else if (!ControlledEnemy->bTurnToAttack/*MyGM->CloseEnemies.Contains(ControlledEnemy)*/)
			{
				stoppingDist = 1000.f;
			}
			WaitingToAttack();
		}
	}
}

void AEnemyAI::LookAtTarget()
{
	if (ControlledEnemy)
	{
		if (!ControlledEnemy->stunt)
		{
			FVector toPlayer = ControlledEnemy->Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
			toPlayer.Normalize();

			FRotator toPlayerRot = toPlayer.Rotation();

			if(!ControlledEnemy->launched)
				toPlayerRot.Pitch = 0.f;

			ControlledEnemy->GetRootComponent()->SetWorldRotation(toPlayerRot);
		}
	}
}

void AEnemyAI::NavigationAI()
{

}

void AEnemyAI::WaitingToAttack()
{
	float distanceToPlayer = FVector::Dist(ControlledEnemy->GetActorLocation(), ControlledEnemy->Player->GetActorLocation());

	if (distanceToPlayer < 1200.f)
	{
		if (distanceToPlayer > 100.f)
		{
			if (!hasDestination)
			{
				targetDesPos = CreateDesPos();
				hasDestination = true;
			}

			evadeT += GetWorld()->GetDeltaSeconds();

			if (evadeT > evadeTime)
			{
				hasDestination = false;
				evadeT = 0.f;
			}
		}
		else
		{
			if (ControlledEnemy->EnemyState != EEnemyState::ST_Attacking)
				targetDesPos = (ControlledEnemy->GetActorLocation() + (ControlledEnemy->GetActorForwardVector() * -1.f)) * 120.f * GetWorld()->GetDeltaSeconds();
		}
	}
	else
	{
		targetDesPos = ControlledEnemy->Player->GetActorLocation();
	}
}

FVector AEnemyAI::CreateDesPos()
{
	FVector retVal = FVector::ZeroVector;

	AFlyCombatGameMode* const MyGM = Cast<AFlyCombatGameMode>(GetWorld()->GetAuthGameMode());

	if (MyGM)
	{
		if (!MyGM->CloseEnemies.Contains(ControlledEnemy))
		{
			targetDesPos = ControlledEnemy->GetActorLocation();
			retVal = targetDesPos;
		}
		else
		{
			FVector pos = FVector::ZeroVector;

			TArray<AActor*> Partners;
			ControlledEnemy->GetCapsuleComponent()->GetOverlappingActors(Partners, AEnemy::StaticClass());

			if (Partners.Num() > 0)
			{
				for (int32 i = 0; i < Partners.Num(); i++)
				{
					if (Cast<AEnemy>(Partners[i]))
					{
						FVector dir = Partners[i]->GetActorLocation() - ControlledEnemy->GetActorLocation();
						pos = dir.GetSafeNormal();
						break;
					}
				}
			}
			else
			{
				GLog->Log("No collider found");
			}
			retVal = pos;
		}
	}

	retVal = ControlledEnemy->GetTransform().TransformPosition(retVal);
	retVal *= 120.f;

	return retVal;
}