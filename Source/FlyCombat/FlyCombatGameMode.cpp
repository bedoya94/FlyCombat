//This project has been created by Eduardo José Bedoya Coneo.All rights reserved

#include "FlyCombat.h"
#include "FlyCombatGameMode.h"
#include "EnemyAI.h"

AFlyCombatGameMode::AFlyCombatGameMode()
{
	closeEnemiesCount = 0;
	closeEnemiesThreshold = 3;
	maxTicketTimer = 1.f;
	tTimer = 0.f;
	timeScale = 1.f;
}

void AFlyCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	avatar = Cast<AAvatar>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0.f));

	// (GetWorld()->GetMapName() == "UEDPIE_0_Test_Level")
	{
		AEnemy* const SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyToSpawn, FVector::ZeroVector, FRotator::ZeroRotator);

		for (int32 i = 0; i < 4; i++)
		{
			FVector SpawnPos = FVector::ZeroVector;
			SpawnPos.X = FMath::FRandRange(-500.f, 1500.f);
			SpawnPos.Y = FMath::FRandRange(-4000.f, 4000.f);
			SpawnPos.Z = 1000.f;

			AEnemy* const SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyToSpawn, SpawnPos, FRotator::ZeroRotator);
		}
	}
}

void AFlyCombatGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//GLog->Log("Level Name = " + GetWorld()->GetMapName());

	if (avatar)
	{
		GetAllEnemies();
		TicketTimers();
		LogGameInfo();
	}
}

void AFlyCombatGameMode::GetAllEnemies()
{
	if (avatar)
	{
		if (avatar->MyCombatComponent->inCombat)
		{
			AllActors = avatar->MyCombatComponent->CloserEnemies;
			AllEnemies.SetNum(AllActors.Num());

			for (int32 i = 0; i < AllActors.Num(); i++)
			{
				AEnemy* const curEnemy = Cast<AEnemy>(AllActors[i]);

				if (curEnemy)
				{
					AllEnemies[i] = curEnemy;
				}
			}

			for (int32 i = 0; i < FarEnemies.Num(); i++)
			{
				if (Cast<AEnemy>(FarEnemies[i]))
					FarEnemies[i]->bTurnToAttack = false;
			}
		}
		else
		{
			AllEnemies.Empty();
			CloseEnemies.Empty();
			FarEnemies.Empty();
		}
	}
	
}

void AFlyCombatGameMode::TicketTimers()
{
	if (avatar)
	{
		//tTimer += GetWorld()->GetDeltaSeconds();

		//if (tTimer > maxTicketTimer)
		//{
		FarEnemies.Empty();
		CloseEnemies.Empty();
		TicketSystemAI();
		//tTimer = 0.f;
		//}
	}
}

void AFlyCombatGameMode::TicketSystemAI()
{
	if (avatar)
	{
		if (avatar->MyCombatComponent->Enemies.Num() > 0)
		{
			int32 threshold = 0.f;

			if (AllEnemies.Num() <= closeEnemiesThreshold)
			{
				threshold = AllEnemies.Num();
			}
			else
			{
				threshold = closeEnemiesThreshold;
			}

			if (AllEnemies.Num() > 0)
			{
				for (int32 i = 0; i < AllEnemies.Num(); i++)
				{
					if (i < threshold)
					{
						if (!CloseEnemies.Contains(AllEnemies[i]))
							CloseEnemies.Add(AllEnemies[i]);
					}
					else
					{
						if (!FarEnemies.Contains(AllEnemies[i]))
							FarEnemies.Add(AllEnemies[i]);
					}
				}

				FindWhoAttacks();
			}
		}
	}
}

void AFlyCombatGameMode::FindWhoAttacks()
{
	if (avatar)
	{
		if (avatar->MyCombatComponent->Enemies.Num() > 0)
		{
			int32 threshold = 0;

			if (AllEnemies.Num() > 0)
			{
				if (AllEnemies.Num() <= closeEnemiesThreshold)
				{
					threshold = AllEnemies.Num();
				}
				else
				{
					threshold = closeEnemiesThreshold;
				}

				int32 ran = FMath::RandRange(0, threshold - 1);

				for (int32 i = 0; i < threshold; i++)
				{
					if (CloseEnemies[i]->EnemyState != EEnemyState::ST_Dead)
						CloseEnemies[i]->bTurnToAttack = true;
					else
						CloseEnemies[i]->bTurnToAttack = false;
				}
			}
		}
	}
}

void AFlyCombatGameMode::LogGameInfo()
{
	if (GEngine)
	{
		FString avatarHpText;
		FString enemyHpText;

		avatarHpText = FString::Printf(TEXT("Player HP = %f"), avatar->HealthPoints);

		if (avatar->CurrentEnemy)
			enemyHpText = FString::Printf(TEXT("Current Enemy HP = %f"), avatar->CurrentEnemy->hp);
		else
			enemyHpText = FString::Printf(TEXT("Current Enemy HP = %f"));

		GEngine->AddOnScreenDebugMessage(0, 100.f, FColor::Red, avatarHpText);
		GEngine->AddOnScreenDebugMessage(1, 100.f, FColor::Red, enemyHpText);
		GEngine->AddOnScreenDebugMessage(2, 100.f, FColor::Red, FString::Printf(TEXT("Current Time = %f"), UGameplayStatics::GetRealTimeSeconds(GetWorld())));

		GEngine->AddOnScreenDebugMessage(3, 100.f, FColor::Red, FString::Printf(TEXT("Enemies Destroyed = %d"), enemiesDestroyed));

		if (enemiesDestroyed >= 5)
			GEngine->AddOnScreenDebugMessage(4, 100.f, FColor::Red, TEXT("GameState = Win"));
		else if (avatar->HealthPoints <= 0.f)
			GEngine->AddOnScreenDebugMessage(4, 100.f, FColor::Red, TEXT("GameState = Lose"));
		else
			GEngine->AddOnScreenDebugMessage(4, 100.f, FColor::Red, TEXT("GameState = NONE"));
	}
}

/*void AFlyCombatGameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
if (!CurrentWidget)
{
CurrentWidget->RemoveFromViewport();
CurrentWidget = nullptr;
}

if (!NewWidgetClass)
{
CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);

if (!CurrentWidget)
{
CurrentWidget->AddToViewport();
}
}
}*/