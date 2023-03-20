// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBaseCharacter.h"

// Sets default values
AFPSBaseCharacter::AFPSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;


	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	
	FPSArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSArmsMesh"));
	FPSArmsMesh->SetupAttachment(RootComponent);
	//这个手臂模型是给第一人称的自己看的，别人看到就穿帮了
	FPSArmsMesh->SetOnlyOwnerSee(true);

	//这个模型是character自带的，用来给别人看，自己就不看了，不然重叠了
	GetMesh()->SetOwnerNoSee(true);
	//模型不需要碰撞，碰撞后面由胶囊体处理
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);


	
}

void AFPSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void AFPSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

