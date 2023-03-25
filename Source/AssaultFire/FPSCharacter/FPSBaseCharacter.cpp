// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FPSBaseCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

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

void AFPSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION(AFPSBaseCharacter, ServerPrimaryWeapon, COND_OwnerOnly);
}


void AFPSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	//��ʼ��ʱ���ǹ
	StartWithKindOfWeapon(bIfSpawnWeapon, WeaponType);

	FPSArmsAnimBP = FPSArmsMesh->GetAnimInstance();
	TPSBodysAnimBP = GetMesh()->GetAnimInstance();

	FPSPlayerController = Cast<AFPSPlayerController>(GetController());
	if(FPSPlayerController)
	{
		//������������UI
		FPSPlayerController->CreatePlayerUI();
		if(ServerPrimaryWeapon)
		{
			ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
		}
	}
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


void AFPSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AFPSBaseCharacter::FirePressed(const FInputActionValue& InputValue)
{
	switch (CurrentWeaponType)
	{
		case EWeaponType::AK47 :
			{
				StartPrimaryFire();
				break;
			}
		default:
			break;
	}
}

void AFPSBaseCharacter::FireReleased(const FInputActionValue& InputValue)
{
	switch (CurrentWeaponType)
	{
			case EWeaponType::AK47:
		{
			StopPrimaryFire();
			break;
		}
		default:
			break;
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



void AFPSBaseCharacter::CollisionTimerFinished()
{
	if(ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon->SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AFPSBaseCharacter::ServerLowSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}


void AFPSBaseCharacter::ServerNormalSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
}

void AFPSBaseCharacter::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool bIsMoving)
{
	if(ServerPrimaryWeapon)
	{
		//�ಥNetMulticast
		ServerPrimaryWeapon->MultiShootingEffect();

		//��ǹ ��һ���ӵ�
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;

		//�ಥ �����������������̫��
		MultiShooting();

		//�ÿͻ��˸���UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

	}


	RifleLineTrace(CameraLocation, CameraRotation, bIsMoving);
}

void AFPSBaseCharacter::MultiShooting_Implementation()
{
	if(TPSBodysAnimBP)
	{
		if(ServerPrimaryWeapon && ServerPrimaryWeapon->ServerTPSBodysAnimMontage)
		{
			TPSBodysAnimBP->Montage_Play(ServerPrimaryWeapon->ServerTPSBodysAnimMontage);
		}
	}
}

void AFPSBaseCharacter::ClientEquipFPArmsPrimary_Implementation()
{
	if(ServerPrimaryWeapon)
	{
		if(ClientPrimaryWeapon)
		{
						//�����������������Ȼ�Ͳ�ִ��
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//�ͻ��������ɵ���������Ӧ�������˵ı���һ�¶����Ǳ��޶���
			//���ý�ɫ�ڿͻ�������������
			if(ServerPrimaryWeapon->WeaponClient)
			{
				//����Ѿ����޶��˾�ִ��Ĭ��ʵ��,���翪��ָ��ĳ������
				ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->WeaponClient, GetActorTransform(), SpawnInfo);
			}
			//����ӵ���
			else
			{
				//�����ھ�return,��ֹ����������
				UE_LOG(LogTemp, Warning, TEXT("Server Weapon Doesn't Set Client Weapon"));
				return;
			}
			//��������ģ�͵���һ�˳��ֱ���
			ClientPrimaryWeapon->K2_AttachToComponent(FPSArmsMesh, TEXT("Weapon_Socket"), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			ClientPrimaryWeapon->WeaponMesh->SetOnlyOwnerSee(true);
			//�ֱ۶���

				//	�����ӵ�UI
			if(FPSPlayerController)
			{
				FPSPlayerController->UpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			}
		}
		
	}
}




AWeaponBaseClient* AFPSBaseCharacter::GetCurrentClientWeapon()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::AK47 :
		{
		return ClientPrimaryWeapon;
			
		}

		default:
			return nullptr;
	}

}


//����������ȥ����
void AFPSBaseCharacter::DropWeapon()
{
	if(ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon->DropWeapon();
	}
}


void AFPSBaseCharacter::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if(FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
	}
}

void AFPSBaseCharacter::MultiSpawnBulletDecal_Implementation(FVector Location, FRotator Rotation)
{
	if(ServerPrimaryWeapon->BulletDecalMaterial)
	{
		UDecalComponent * Decal =  UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ServerPrimaryWeapon->BulletDecalMaterial,FVector(8,8,8),Location, Rotation , 10.f);
		if(Decal)
		{
			//�������ԽС��ԽԶ�ܿ�������
			Decal->SetFadeScreenSize(0.001);
		}
	}
}

void AFPSBaseCharacter::EquipPrimaryWeapon(AWeaponBaseServer* WeaponBaseServer)
{
	if(ServerPrimaryWeapon)
	{
		//�����������������Ȼ�Ͳ�ִ��
		return;
	}
	else
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		//������ģ�ͣ���ӵ�����£��Ը���Ŀ�귽ʽ���ӣ������������ֱ�Ϊλ����ת�ߴ磬���һ�����Ƿ���汻���ӵ��������ģ����������ɫ����ʱ��ģ������Ļ�ǹеģ��Ҳ����ģ������
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(),TEXT("Weapon_Socket"),EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		ServerPrimaryWeapon->WeaponMesh->SetOwnerNoSee(true);
		//���ÿͻ���װ��������
		ClientEquipFPArmsPrimary();
	}
}


AWeaponBaseServer* AFPSBaseCharacter::GetServerPrimaryWeapon()
{
	return ServerPrimaryWeapon;
}

void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* WeaponBaseClient = GetCurrentClientWeapon();
	if (WeaponBaseClient)
	{
		//�����������
		WeaponBaseClient->PlayShootAnimation();
		//�ֱ۲��Ŷ���
		UAnimMontage* ClientArmsFireMontage = WeaponBaseClient->ClientArmsFireAnimMontage;
			
		if (ClientArmsFireMontage)
		{
			FPSArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);//����һ���ٲ��� 
			FPSArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		}

		//�����������������ǹ������
		WeaponBaseClient->DisplayWeaponEffect();

		if(FPSPlayerController)
		{
			//������Ļ����
			FPSPlayerController->PlayerCameraShake(WeaponBaseClient->CameraShakeClass);

			//����׼����ɢ����
			FPSPlayerController->DoCrossHairRecoilAnim(); 
		}
	
	}
	
}

void AFPSBaseCharacter::StartPrimaryFire()
{
	//�ж��ӵ�����ʣ��
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//�����(�����������, ǹ����������Ч��, ���ٵ�ҩ�����߼��( ����,��ǹ����ǹ���ѻ�ǹ)���˺�Ӧ�ã���������)
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		//�ͻ��ˣ�ǹ�岥�Ŷ���done���ֱ۲��Ŷ���done�������������done����Ļ����done����������ǹ����������Ч��done����
		//����ʮ��׼��done ����ʮ��׼��done ����ʱ׼����ɢ ��beginplay��ӣ��ɿ�����������ͼʵ��done
		//�����ӵ�UI�����ӵ�UI 
		ClientFire();

	
		//���Զ�ȫ�Զ�����
	}
	
}

void AFPSBaseCharacter::StopPrimaryFire()
{

}

void AFPSBaseCharacter::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector =  UKismetMathLibrary::GetForwardVector(CameraRotation);
	FHitResult HitResult;

	//�Լ��Ͷ��� �Ȱ��Լ��ӽ�ȥ
	IgnoreArray.Add(this);

	if (ServerPrimaryWeapon)
	{
		//�Ƿ��ƶ����²�ͬ��location���
		if (bIsMoving)
		{

		}
		else
		{
			EndLocation = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
		}
	}

	//�����Ƿ�����
	bool  HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None , HitResult,true,FLinearColor::Blue , FLinearColor::Red , 10.F );

	if(HitSuccess)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit Actor Name : %s"), *HitResult.GetActor()->GetName()));
		if(AFPSBaseCharacter * HittedCharacter = Cast<AFPSBaseCharacter>(HitResult.GetActor()))
		{
			//����ң�Ӧ���˺�
				
		}
		else
		{
			//��������ķ�������
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			//������ ���ɹ㲥����
			MultiSpawnBulletDecal(HitResult.Location, XRotator );
		}
	}
}

void AFPSBaseCharacter::StartWithKindOfWeapon(bool _bIfSpawnWeapon,EWeaponType _WeaponType)
{
	if (bIfSpawnWeapon == false) return;
	//ֻ���ڷ������ϵ���
	if(HasAuthority())
	{
		//��������������
		PurchaseWeapon(_WeaponType);
	}
}

void AFPSBaseCharacter::PurchaseWeapon(EWeaponType _WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride =ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (_WeaponType)
	{
	case EWeaponType::AK47 :
		{
			//�����������������Ȼ�Ͳ�ִ��
			if(ServerPrimaryWeapon)
			{
				return;
			}
			else
			{
				//��̬�õ�AK47��
				//·����β�Ӹ�_C��ʾ���� ��Ȼ�ò���
					UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/AK47/BP_ServerAK47.BP_ServerAK47_C'") );

				AWeaponBaseServer * ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
	
				//�����ر���ײ
				ServerWeapon->EquipWeapon();
				//����������
				EquipPrimaryWeapon(ServerWeapon);
		
			}
			break;
		}
		default:
			{
			break;
			}
	}
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

			//�����ӳ������ظ����
			Subsystem->ClearAllMappings();
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
		EnhancedInputComponent->BindAction(IA_FirePressed, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::FirePressed);

		EnhancedInputComponent->BindAction(IA_FireReleased, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::FireReleased);

		//shift����	
		EnhancedInputComponent->BindAction(IA_ShiftPressed, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LowSpeedWalk);

		EnhancedInputComponent->BindAction(IA_ShiftReleased, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::NormalSpeedWalk);

		//��G��������
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::DropWeapon);

	// �����ͨ������"ETriggerEvent"ö��ֵ���󶨵��˴������ⴥ�����¼�
	//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}

}
