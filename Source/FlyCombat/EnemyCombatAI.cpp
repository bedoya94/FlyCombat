//This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#include "FlyCombat.h"
#include "EnemyCombatAI.h"
#include "Enemy.h"
#include "Avatar.h"


// Sets default values for this component's properties
UEnemyCombatAI::UEnemyCombatAI()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	attackCoolDown = 5.f;
	attackTimer = 0;
	bIsAttacking = false;

	PunchCol = CreateDefaultSubobject<USphereComponent>(TEXT("Punch Collider"));
	PunchCol->SetSphereRadius(25.f);

	KickCol = CreateDefaultSubobject<USphereComponent>(TEXT("Kick Collider"));
	KickCol->SetSphereRadius(25.f);

	PunchCol->OnComponentBeginOverlap.AddDynamic(this, &UEnemyCombatAI::OnAttackEnter);
	KickCol->OnComponentBeginOverlap.AddDynamic(this, &UEnemyCombatAI::OnAttackEnter);

	bAttacking = false;
	bIsUnblockable = false;
}


// Called when the game starts
void UEnemyCombatAI::BeginPlay()
{
	Super::BeginPlay();

	// ...

	MyOwner = Cast<AEnemy>(GetOwner());

}


// Called every frame
void UEnemyCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (MyOwner)
	{
		if (MyOwner->Player)
		{
			Target = MyOwner->Player;
		}

		if (MyOwner->EnemyState == EEnemyState::ST_Dead || MyOwner->bParried)
		{
			ResetCombo();
		}
	}
	else
	{
		return;
	}

	if (Target)
	{
		if (!Target->bCounter)
		{
			if (attackTimer > 0.f)
			{
				attackTimer -= DeltaTime;
				bCanAttack = false;
				int32 ran = FMath::RandRange(0, 2);
				attackNum = ran;
			}

			if (attackTimer <= 0.f)
			{
				attackTimer = 0.f;

				if (MyOwner->bTurnToAttack)
					bCanAttack = true;
				else
					bCanAttack = false;

				if (attackNum == 2)
					bIsUnblockable = true;
			}

			if (bCanAttack && MyOwner->EnemyState != EEnemyState::ST_Attacked && !MyOwner->bHitted && !MyOwner->launched)
				Attack();
		}

		if (bAttacking && Target->CheckHit())
			bHitting = Target->bHitted;
		else
			bHitting = false;
	}
}

void UEnemyCombatAI::Attack()
{
	float dist = FVector::Dist(MyOwner->GetActorLocation(), Target->GetActorLocation());

	if (dist < 180.f && bCanAttack)
	{
		bAttacking = true;
		MyOwner->LaunchCharacter(MyOwner->GetActorForwardVector() * 600.f, false, true);
		MyOwner->PlayAnimMontage(MyOwner->BasicAttacks[attackNum]);
		attackTimer = attackCoolDown;
		ThingsHit.Empty();
		bCanAttack = false;
	}
}

void UEnemyCombatAI::ResetCombo()
{
	ThingsHit.Empty();
	bIsAttacking = false;
	bAttacking = false;
	bIsUnblockable = false;
	attackTimer = attackCoolDown;
}

void UEnemyCombatAI::OnAttackEnter_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AAvatar* const Target = Cast<AAvatar>(OtherActor);

	if (!Target || OtherComp != OtherActor->GetRootComponent())
	{
		return;
	}
	else
	{
		if (bIsAttacking && OtherActor != MyOwner && !ThingsHit.Contains(OtherActor))
		{
			ThingsHit.Add(OtherActor);

			if (Target->bIsParrieng && !bIsUnblockable)
			{
				MyOwner->bParried = true;
				Target->bCounter = true;
			}
			else
			{
				Target->bHitted = true;
				if (Target->CheckHit())
				{
					Target->TakingDamage(25.f);
				}
			}
		}
	}
}