// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FPSBaseCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values
AFPSBaseCharacter::AFPSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;


	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	
	FPSArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSArmsMesh"));
	FPSArmsMesh->SetupAttachment(PlayerCamera);
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

void AFPSBaseCharacter::MoveForward(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if((Controller != nullptr) && (value!= 0.f))
	{
		// �ж��ĸ���ת��������ǰ��
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// ��ȡ��ǰ������
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}

}

void AFPSBaseCharacter::MoveRight(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if(Controller != nullptr && value != 0.f)
	{
		// �ж��ĸ���ת���������ҵ�
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// ��ȡ���ҵ�����
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}

}

void AFPSBaseCharacter::LookRightRate(const FInputActionValue& InputValue)
{
	AddControllerYawInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSBaseCharacter::LookUpRate(const FInputActionValue& InputValue)
{
	AddControllerPitchInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSBaseCharacter::OnJump(const FInputActionValue& InputValue)
{
	Jump();
}

void AFPSBaseCharacter::OnJumpStoping(const FInputActionValue& InputValue)
{
	StopJumping();
}

void AFPSBaseCharacter::Fire(const FInputActionValue& InputValue)
{
	FString Message = FString::SanitizeFloat(999);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			60,
			FColor::Green,
			Message
		);
	}
}

void AFPSBaseCharacter::LowSpeedWalk(const FInputActionValue& InputValue)
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	ServerLowSpeedWalk();
}

void AFPSBaseCharacter::NormalSpeedWalk(const FInputActionValue& InputValue)
{
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	ServerNormalSpeedWalk();
}

void AFPSBaseCharacter::ServerLowSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

bool AFPSBaseCharacter::ServerLowSpeedWalk_Validate()
{
	return true;
}

void AFPSBaseCharacter::ServerNormalSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
}

bool AFPSBaseCharacter::ServerNormalSpeedWalk_Validate()
{
	return true;
}

void AFPSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//ʹ�ÿ�����������תpitch��yaw
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	
	if(APlayerController * PC = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) )
		{
			Subsystem->AddMappingContext(IMC_Action, 0);
			Subsystem->AddMappingContext(IMC_MoveBase, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if(EnhancedInputComponent)
	{

		//�ƶ�
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::MoveRight);
		//��ת
		EnhancedInputComponent->BindAction(IA_LookRightRate, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LookRightRate);
		EnhancedInputComponent->BindAction(IA_LookUpRate, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LookUpRate);
		//��Ծ
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AFPSBaseCharacter::OnJump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AFPSBaseCharacter::OnJumpStoping);
		//���
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::Fire);
		
		//shift����
		EnhancedInputComponent->BindAction(IA_Shift, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LowSpeedWalk);
		EnhancedInputComponent->BindAction(IA_Shift, ETriggerEvent::Completed, this, &AFPSBaseCharacter::NormalSpeedWalk);

	}

	// �����ͨ������"ETriggerEvent"ö��ֵ���󶨵��˴������ⴥ�����¼�
	//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);
}





