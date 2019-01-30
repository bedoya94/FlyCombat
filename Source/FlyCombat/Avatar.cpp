//This project has been created by Eduardo José Bedoya Coneo. All rights reserved

#include "FlyCombat.h"
#include "Avatar.h"
#include "Engine.h"
#include "FlyCombatGameMode.h"

// Sets default values
AAvatar::AAvatar()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.f);
	GetCharacterMovement()->BrakingDecelerationFlying = 2048.f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	GetCapsuleComponent()->SetCapsuleRadius(40.f);
	GetCapsuleComponent()->SetCapsuleHalfHeight(100.f);

	Combo = 0;
	DashTime = 0.f;
	DashCoolDown = 2.f;
	bIsSpecialAttack = false;

	MyCombatComponent = CreateDefaultSubobject<UAvatarCombatComponent>(TEXT("MyCombatComponent"));
	this->AddOwnedComponent(MyCombatComponent);

	MyCombatComponent->CombatSphere->AttachTo(GetRootComponent());
	MyCombatComponent->RightFist->AttachTo(GetMesh(), "R_HandSocket");
	MyCombatComponent->LeftFist->AttachTo(GetMesh(), "L_HandSocket");
	MyCombatComponent->RightKick->AttachTo(GetMesh(), "R_FootSocket");
	MyCombatComponent->LeftKick->AttachTo(GetMesh(), "L_FootSocket");

	pursueNum = 0;
	bPursueAttack = false;

	bIsGuarding = false;
	bIsParrieng = false;
	baseDamage = 10.f;
	dealDamage = baseDamage;
	maxHP = 300.f;
	HealthPoints = maxHP;
	animSpeed = 0.f;
	bPauseGame = false;
}

void AAvatar::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

// Called when the game starts or when spawned
void AAvatar::BeginPlay()
{
	Super::BeginPlay();
	bPauseGame = false;
}

// Called every frame
void AAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetCharacterMovement()->MovementMode = MOVE_Flying;

	CombatCameraHandler(DeltaTime);

	{
		if (bHitted)
			launchForce = 1000.f;

		if (DashTime > 0.f)
		{
			bUseControllerRotationPitch = false;
			bUseControllerRotationYaw = false;
			bUseControllerRotationRoll = false;

			bIsGuarding = false;
			bIsDashing = true;
			DashTime -= DeltaTime;
		}
		if (DashTime <= 0.f)
		{
			bIsDashing = false;
			DashTime = 0.f;
		}

		if (bCanAttack && canTurn)
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 5400.0f, 0.f);
		else if (!canTurn)
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.f);
	}

	{
		if (Combo >= 4)
			canTurn = false;

		if (Combo < 4)
		{
			bCanPursue = false;
			bPursueing = false;
		}

		if (!MyCombatComponent->inCombat)
		{
			CurrentEnemy = nullptr;
		}

		if (CurrentEnemy)
		{
		
			if (Combo >= 4 && CurrentEnemy->EnemyState != EEnemyState::ST_Dead)
			{
				bCanPursue = true;
			}

			if (bPursueing && CurrentEnemy->EnemyState != EEnemyState::ST_Dead)
			{
				bHitted = false;

				if (MyCombatComponent->TargetEnemies.Num() > 0)
				{
					FVector myLocation = GetActorLocation();
					FVector enemyLocation = CurrentEnemy->GetActorLocation();
					float dist = FVector::Dist(myLocation, enemyLocation);

					if (bInPursue)
						SetActorLocation(FMath::Lerp(myLocation, enemyLocation, 10.f * GetWorld()->GetDeltaSeconds()));
				}
			}

		}

		if (pursueNum > 0)
		{
			if (pursueNum >= 3)
			{
				ComboReset();
			}
		}
	}

	if (InComboRush)
	{
		animSpeed = 1.f;
	}
	else
	{
		animSpeed = 1.f;
	}

	if (Combo >= 4 && MyCombatComponent->bPunching)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	}
}

// Called to bind functionality to input
void AAvatar::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("Zooming", IE_Pressed, this, &AAvatar::ZoomIn);
	InputComponent->BindAction("Zooming", IE_Released, this, &AAvatar::ZoomOut);
	InputComponent->BindAction("SpeedBoost", IE_Pressed, this, &AAvatar::Faster);
	InputComponent->BindAction("SpeedBoost", IE_Released, this, &AAvatar::NotFaster);

	InputComponent->BindAction("Evade", IE_Pressed, this, &AAvatar::Evading);
	InputComponent->BindAction("Evade", IE_Released, this, &AAvatar::NotEvading);
	InputComponent->BindAction("Faster", IE_Repeat, this, &AAvatar::Dodge);
	InputComponent->BindAxis("Guarding", this, &AAvatar::Guarding);
	InputComponent->BindAction("Guard", IE_Pressed, this, &AAvatar::Parry);
	InputComponent->BindAction("Guard", IE_Released, this, &AAvatar::NotGuarding);

	InputComponent->BindAction("BasicAttack", IE_Pressed, this, &AAvatar::Attacking);
	InputComponent->BindAction("FinisherUp", IE_Pressed, this, &AAvatar::FinisherUp);
	InputComponent->BindAction("FinisherDown", IE_Pressed, this, &AAvatar::FinisherDown);

	InputComponent->BindAxis("MoveForward", this, &AAvatar::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AAvatar::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &AAvatar::MoveUp);
	InputComponent->BindAxis("TurnRate", this, &AAvatar::TurnRate);
	InputComponent->BindAxis("LookUpRate", this, &AAvatar::LookUpRate);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AAvatar::MoveForward(float Axis)
{
	if (!bIsGuarding)
	{
		if (Controller != NULL && Axis != 0.0f && !bIsDashing && !bHitted  && !bCounter)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Axis);

			CurrentControllerDirection = GetController()->GetActorForwardVector() * Axis;
			CurrentActorDirection = GetActorForwardVector() * Axis;

		}
	}
	else
	{
		if (Controller != NULL && Axis != 0.0f && !bIsDashing && !bHitted  && !bCounter)
		{
			AddMovementInput(GetActorForwardVector(), Axis);

			CurrentControllerDirection = GetController()->GetActorForwardVector() * Axis;
			CurrentActorDirection = GetActorForwardVector() * Axis;
		}

	}
	
	CurForwardAxis = Axis;
}

void AAvatar::MoveRight(float Axis)
{
	if (!bIsGuarding)
	{
		if (Controller != NULL && Axis != 0.0f && !bIsDashing && !CheckHit() && !bCounter)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Axis);

			CurrentControllerDirection = GetController()->GetActorRightVector() * Axis;
			CurrentActorDirection = GetActorRightVector() * Axis;
		}
	}
	else
	{
		if (Controller != NULL && Axis != 0.0f && !bIsDashing && !bHitted  && !bCounter)
		{
			AddMovementInput(GetActorRightVector(), Axis);

			CurrentControllerDirection = GetController()->GetActorRightVector() * Axis;
			CurrentActorDirection = GetActorRightVector() * Axis;
		}
	}


	CurRightAxis = Axis;
}

void AAvatar::MoveUp(float Axis)
{
	if (Controller != NULL && Axis != 0.0f && !bIsDashing && !CheckHit() && !bCounter)
	{
		if (!bCanAttack)
			AddMovementInput(GetActorUpVector(), Axis);

		CurrentActorDirection = GetActorUpVector() * Axis;
	}

	CurUpAxis = Axis;
}

bool AAvatar::GetDirectionAxis()
{
	bool isMoving = false;

	if (CurForwardAxis != 0.f || CurRightAxis != 0.f)
	{
		isMoving = true;
	}
	else if (CurForwardAxis == 0.f && CurRightAxis == 0.f)
	{
		isMoving = false;
	}

	return isMoving;
}

void AAvatar::TurnRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AAvatar::LookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AAvatar::ZoomIn()
{
	bZooming = true;
}

void AAvatar::ZoomOut()
{
	bZooming = false;
}

void AAvatar::Faster()
{

	if (!CheckHit() && !bCounter)
	{
		if (!bIsGuarding)
			bIsFaster = true;

		if (!bCanAttack)
		{
			GetCharacterMovement()->MaxFlySpeed = 1500.f;
		}

		if (bIsGuarding)
		{
			if (Combo < 4 && !CheckHit() && !bCounter)
			{
				if (DashTime == 0.f)
				{
					bUseControllerRotationPitch = false;
					bUseControllerRotationYaw = true;
					bUseControllerRotationRoll = false;
					bIsParrieng = false;
					bIsDashing = true;
					Dashing();
					DashTime = 1.f;
				}
			}
		}
		if (bCanPursue && pursueNum < 2)
		{
			bPursueing = true;
		}
	}
}

void AAvatar::NotFaster()
{
	bIsFaster = false;
	if (!bCanAttack)
		GetCharacterMovement()->MaxFlySpeed = 600.f;
}

void AAvatar::Dodge()
{
	if (Combo < 4 && !CheckHit() && !bCounter)
	{
		if (DashTime == 0.f)
		{
			bIsParrieng = false;
			bIsDashing = true;
			Dashing();
			DashTime = 1.f;
		}
	}
}

void AAvatar::Attacking()
{
	if (!CheckHit() && !bIsGuarding && !bIsParrieng)
	{
		bCanAttack = true;
		if (bCanAttack && !bIsDashing && !bPursueing && !bCounter)
		{
			bChainAttack = true;

			GetCharacterMovement()->MaxFlySpeed = 0.f;
			if (Combo == 0)
			{
				launchForce = 600.f;

				if (!InComboRush)
					PlayAnimMontage(BasicAttacks[0], animSpeed);
				else
					PlayAnimMontage(BasicAttacks[3], animSpeed, "CR_Begin");
					
				Combo = 1;
			}

			if (Combo == 3)
			{
				bIsFinishingUp = false;
				bIsFinishingDown = false;
				bIsFinishingForward = true;
			}
		}

		if (bInPursue)
		{
			bPursueAttack = true;
		}

		if (!bPursueing && !InComboRush && pursueNum > 0 && (Combo < 3 || bPursueAttack))
		{
			comboRushHits++;
		}

		if (InComboRush)
		{
			MyCombatComponent->rushTimer++;
			MyCombatComponent->rushCount++;
		}

		if (MyCombatComponent->inCombat && CurrentEnemy)
		{
			if (!CurrentEnemy->stunt && !CurrentEnemy->launched)
				launchToEnemy = true;
		}
	}
}

void AAvatar::ComboChain()
{
	if (bChainAttack && !bIsDashing && !CheckHit())
	{
		canTurn = true;

		if (!InComboRush)
		{
			switch (Combo)
			{
			case 1:
				PlayAnimMontage(BasicAttacks[1], animSpeed);
				Combo = 2;
				break;

			case 2:
				if (pursueNum <= 0)
					PlayAnimMontage(BasicAttacks[2], animSpeed);
				else
					PlayAnimMontage(BasicAttacks[4], animSpeed);

				Combo = 3;
				break;

			case 3:
				Combo = 4;
				launchForce = 2000.f;
				if (bIsFinishingForward)
					PlayAnimMontage(FinisherAttacks[0]);
				else if (bIsFinishingDown)
					PlayAnimMontage(FinisherAttacks[1]);
				else if (bIsFinishingUp)
					PlayAnimMontage(FinisherAttacks[2]);

				break;
			}
		}
		else
		{
			switch (Combo)
			{
			case 1:
				PlayAnimMontage(BasicAttacks[3], animSpeed, "CR_Loop");
				Combo = 2;
				break;

			case 2:
				PlayAnimMontage(BasicAttacks[3], animSpeed, "CR_Loop");
				Combo = 1;
				break;
			}
		}

	}
}

void AAvatar::FinisherUp()
{
	if (Combo == 3 && !CheckHit())
	{
		bChainAttack = true;
		bIsFinishingUp = true;
		bIsFinishingDown = false;
		bIsFinishingForward = false;
	}
}

void AAvatar::FinisherDown()
{
	if (Combo == 3 && !CheckHit())
	{
		bChainAttack = true;
		bIsFinishingUp = false;
		bIsFinishingDown = true;
		bIsFinishingForward = false;
	}
}

void AAvatar::ComboReset()
{
	//GLog->Log("Combo Reset!!!");
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	bCanPursue = false;
	bPursueing = false;
	bPursueAttack = false;
	bIsFinishingUp = false;
	bIsFinishingDown = false;
	bIsFinishingForward = false;
	bIsSpecialAttack = false;

	if (CurrentEnemy)
		CurrentEnemy = nullptr;

	if (InComboRush)
	{
		PlayAnimMontage(BasicAttacks[3], 1.f, "CR_End");
	}
		

	comboRushHits = 0;
	MyCombatComponent->rushTimer = 0.f;
	InComboRush = false;
	MyCombatComponent->ThingsHit.Empty();
	MyCombatComponent->bPunching = false;
	MyCombatComponent->bAttacking = false;
	bHitted = false;
	bCounter = false;
	Combo = 0;
	pursueNum = 0;
	canTurn = true;
	bCanAttack = false;
	bChainAttack = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.f);
	GetCharacterMovement()->MaxFlySpeed = 600.f;
}

void AAvatar::Evading()
{
	if (!CheckHit() && !bCounter)
	{
		if (DashTime == 0.f)
		{
			bIsParrieng = false;
			bIsDashing = true;
			Dashing();
			DashTime = 1.f;
		}
	}
}

void AAvatar::NotEvading()
{
	bIsDashing = false;
}

void AAvatar::Dashing()
{
	launchForce = 2500.f;
	Launcher();
	PlayAnimMontage(DefensiveAnims[6]);

	if (CurUpAxis == 0.f)
	{
		if (CurForwardAxis > 0.f)
			PlayAnimMontage(DefensiveAnims[0]);
		else if (CurForwardAxis < 0.f)
			PlayAnimMontage(DefensiveAnims[6]);
		else if (CurRightAxis > 0.f)
			PlayAnimMontage(DefensiveAnims[7]);
		else if (CurRightAxis < 0.f)
			PlayAnimMontage(DefensiveAnims[8]);
	}
	else
	{
		if (CurUpAxis < 0.f)
			PlayAnimMontage(DefensiveAnims[1]);
		else if (CurUpAxis > 0.f)
			PlayAnimMontage(DefensiveAnims[2]);
	}
}

void AAvatar::Launcher()
{
	if (bIsDashing)
	{
		LaunchCharacter(CurrentActorDirection * launchForce, false, true);
	}
	else if (bCanAttack)
	{
		LaunchCharacter(GetActorForwardVector() * launchForce, false, true);
	}
	else if (CurUpAxis == 0.f && CurForwardAxis == 0.f && CurRightAxis == 0.f)
	{
		LaunchCharacter(GetActorForwardVector() * -1.f * launchForce, false, true);
	}
	else if (CurUpAxis == 0.f && (CurForwardAxis != 0.f || CurRightAxis != 0.f))
	{
		LaunchCharacter(GetActorForwardVector() * launchForce, false, true);
	}
	else if (bHitted && (!bCounter || !bIsParrieng))
	{
		LaunchCharacter(GetActorForwardVector() * -1.f * launchForce, false, true);
	}
}

void AAvatar::Guarding(float axis)
{
	if (axis > 0.f)
	{
		if (!bIsDashing && !CheckHit() && !bCounter && !bPursueing && !bInPursue)
		{
			bIsGuarding = true;
			bUseControllerRotationYaw = true;
			CurrentEnemy = nullptr;
		}
	}
}

void AAvatar::Parry()
{
	if (!bIsDashing && !CheckHit() && !bCounter)
	{
		bIsParrieng = true;
		bUseControllerRotationYaw = true;
	}
}

void AAvatar::NotGuarding()
{
	bIsGuarding = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AAvatar::PursueEnemyTarget()
{
	pursueNum++;
	bIsFinishingUp = false;
	bIsFinishingDown = false;
	bIsFinishingForward = false;
	bIsSpecialAttack = false;
	bCanPursue = true;
	bCanAttack = true;
	bChainAttack = false;
	canTurn = false;

	if (MyCombatComponent->TargetEnemies.Num() > 0 && pursueNum < 3)
	{
		PlayAnimMontage(DefensiveAnims[4]);
	}
}

bool AAvatar::CheckHit()
{
	hitted = false;

	if (bHitted && (!bIsGuarding && !bIsParrieng && !bCounter && !bPursueing))
	{
		hitted = true;
	}
	else if (bHitted && (bIsGuarding || bIsParrieng || bCounter || bPursueing))
	{
		hitted = false;
	}

	return hitted;
}

void AAvatar::TakingDamage(float _damage)
{
	HealthPoints -= _damage;

	if (HealthPoints <= 0.f)
	{
		HealthPoints = 0.f;
	}
}

void AAvatar::CombatCameraHandler(float _DeltaTime)
{
	if (bZooming || (Combo == 3 && pursueNum > 0) || bCounter)
	{
		zoomFactor += _DeltaTime / 0.5f;
	}
	else
	{
		zoomFactor -= _DeltaTime / 0.25f;

	}

	zoomFactor = FMath::Clamp(zoomFactor, 0.0f, 1.0f);
	Camera->FieldOfView = FMath::Lerp(90.0f, 60.0f, zoomFactor);
	SpringArm->TargetArmLength = FMath::Lerp(850.f, 400.f, zoomFactor);

	if (MyCombatComponent->inCombat)
	{
		if (bPursueing || bPursueAttack || (Combo >= 4 && pursueNum >= 2))
		{

			if (CurrentEnemy)
			{
				FVector NewRot = CurrentEnemy->GetActorLocation() - GetActorLocation();
				NewRot.Normalize();
				FRotator toNewRot = NewRot.Rotation();

				SpringArm->bUsePawnControlRotation = true;
				GetController()->SetControlRotation(toNewRot);

				SpringArm->SocketOffset.X = 100.f;
				SpringArm->SocketOffset.Y = 50.f;
				SpringArm->SocketOffset.Z = 20.f;
				SpringArm->TargetArmLength = 300.f;
				SpringArm->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 50.f), FRotator(-60.f, 0.f, 0.f));
			}
		}
		else if (InComboRush || pursueNum > 0)
		{
			AFlyCombatGameMode* const MyGM = Cast<AFlyCombatGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

			if (CurrentEnemy)
			{
				if (MyGM)
				{
					{
						SpringArm->bUsePawnControlRotation = true;
						SpringArm->SocketOffset.ZeroVector;
						SpringArm->TargetArmLength = 600.f;
					}
				}
			}

		}
		else
		{
			SpringArm->bUsePawnControlRotation = true;
			SpringArm->SocketOffset.ZeroVector;
			SpringArm->TargetArmLength = 850.f;
		}

		if (!CurrentEnemy)
		{
			bCanPursue = false;
			bPursueing = false;
			bPursueAttack = false;
			bIsSpecialAttack = false;
			InComboRush = false;
			MyCombatComponent->ThingsHit.Empty();
			MyCombatComponent->bPunching = false;
			bHitted = false;
			bCounter = false;
			pursueNum = 0;
			canTurn = true;
		}
	}
	else
	{
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->SocketOffset.X = 150.f;
		SpringArm->SocketOffset.Y = 50.f;
		SpringArm->SocketOffset.Z = 20.f;
		SpringArm->TargetArmLength = 300.f;
		SpringArm->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 50.f), FRotator(0.f, 0.f, 0.f));

		bCanPursue = false;
		bPursueing = false;
		bIsSpecialAttack = false;
		InComboRush = false;
		MyCombatComponent->ThingsHit.Empty();
		MyCombatComponent->bPunching = false;
		bHitted = false;
		bCounter = false;
		pursueNum = 0;
		canTurn = true;

		CurrentEnemy = nullptr;

	}
}

bool AAvatar::GamePaused()
{
	bool bPauseState = false;
	bPauseGame = !bPauseGame;
	bPauseState = bPauseGame;

	UGameplayStatics::SetGamePaused(GetWorld(), bPauseGame);

	return bPauseState;
}