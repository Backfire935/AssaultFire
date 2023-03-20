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
	//����ֱ�ģ���Ǹ���һ�˳Ƶ��Լ����ģ����˿����ʹ�����
	FPSArmsMesh->SetOnlyOwnerSee(true);

	//���ģ����character�Դ��ģ����������˿����Լ��Ͳ����ˣ���Ȼ�ص���
	GetMesh()->SetOwnerNoSee(true);
	//ģ�Ͳ���Ҫ��ײ����ײ�����ɽ����崦��
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

