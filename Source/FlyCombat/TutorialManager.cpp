// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyCombat.h"
#include "TutorialManager.h"


// Sets default values
ATutorialManager::ATutorialManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bInTutorial = false;
}

// Called when the game starts or when spawned
void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()->GetMapName() != "UEDPIE_0_Test_Level")
	{
		bInTutorial = true;
	}
	else
	{
		bInTutorial = false;
	}
}

// Called every frame
void ATutorialManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

