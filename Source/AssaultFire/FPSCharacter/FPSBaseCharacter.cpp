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
#include "PhysicalMaterials/PhysicalMaterial.h"
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
	DOREPLIFETIME_CONDITION(AFPSBaseCharacter, bIsFiring, COND_None);
	DOREPLIFETIME_CONDITION(AFPSBaseCharacter, bIsReloading, COND_None);

}


void AFPSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	//��Ӧ���˺������Ļص�
	OnTakePointDamage.AddDynamic(this, &AFPSBaseCharacter::OnHit);
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

	bIsFiring = true;
	RifleLineTrace(CameraLocation, CameraRotation, bIsMoving);
}

void AFPSBaseCharacter::ServerReloadPrimary_Implementation()
{
	//�ͻ����ֱ۲��Ŷ������������ಥ���嶯�������ݸ���,UI����
	ClientReload();
}

void AFPSBaseCharacter::ServerStopFire_Implementation()
{
	bIsFiring = false;
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

void AFPSBaseCharacter::ResetRecoil()
{
	 NewVerticalRecoilAmount = 0.f;
	 OldVerticalRecoilAmount = 0.f;
	 VerticalRecoilAmount = 0.f;
	 RecoilXCoordPerShoot = 0.f;
	 NewHorizontalRecoilAmount = 0.f;
		 OldHorizontalRecoilAmount = 0.f;
		 HorizontalRecoilAmount = 0.f;
		 RecoilYCoordPerShoot = 0.f;
}


//����������ȥ����
void AFPSBaseCharacter::InputDropWeapon()
{
	if(ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon->DropWeapon();
	}
}

void AFPSBaseCharacter::InputReload()
{
	if(!bIsReloading)
	{
		if(!bIsFiring)
		{
			switch (CurrentWeaponType)
			{
			case EWeaponType::AK47:
			{
				ServerReloadPrimary();
				break;
			}
			}
		}
		
	}
}


void AFPSBaseCharacter::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if(FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
	}
}


void AFPSBaseCharacter::ClientUpdateHealthUI_Implementation(float NewHealth,float _MaxHealth)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateHealthUI(NewHealth, _MaxHealth);
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


void AFPSBaseCharacter::ClientRecoil_Implementation()
{
	UCurveFloat* VerticalRecoilCurve = nullptr;
	UCurveFloat* HorizontalRecoilCurve = nullptr;
	if(ServerPrimaryWeapon)
	{
		//��ȡ�����ĺ�������
		VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
		HorizontalRecoilCurve = ServerPrimaryWeapon->HorizontalRecoilCurve;
	}
	//��ĺ������0.1
	RecoilXCoordPerShoot += 0.1;
	RecoilYCoordPerShoot += 0.1;

	if(VerticalRecoilCurve)
	{
		//����ĺ������ȡ��Ӧ��floatֵ��Ӧ�õ��µĴ�ֱ��������ֵ��
		NewVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);
	}

	if (HorizontalRecoilCurve)
	{
		//����ĺ������ȡ��Ӧ��floatֵ��Ӧ�õ��µ�ˮƽ��������ֵ��
		NewHorizontalRecoilAmount = HorizontalRecoilCurve->GetFloatValue(RecoilYCoordPerShoot);
	}
	
	//
	VerticalRecoilAmount = NewVerticalRecoilAmount - OldVerticalRecoilAmount;
	HorizontalRecoilAmount = NewHorizontalRecoilAmount - OldHorizontalRecoilAmount;

	if(FPSPlayerController)
	{
		FRotator ControllerRotator = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(ControllerRotator.Pitch + VerticalRecoilAmount, ControllerRotator.Yaw +	HorizontalRecoilAmount,ControllerRotator.Roll));

	}

	OldVerticalRecoilAmount = NewVerticalRecoilAmount;
	OldHorizontalRecoilAmount = NewHorizontalRecoilAmount;

}

void AFPSBaseCharacter::ClientReload_Implementation()
{
	//�ֱ۲��Ŷ�����̫��
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if(CurrentClientWeapon)
	{
		UAnimMontage* ClientArmsReloadingMontage = GetCurrentClientWeapon()->ClientArmsReloadAnimMontage;
		FPSArmsAnimBP->Montage_Play(ClientArmsReloadingMontage);
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

		//�ͻ��˺�����
		ClientRecoil();

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

void AFPSBaseCharacter::AutoFire()
{
	//�ж��ӵ�����ʣ��
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//�����(�����������, ǹ����������Ч��, ���ٵ�ҩ�����߼��( ����,��ǹ����ǹ���ѻ�ǹ)���˺�Ӧ�ã���������)
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}
		//�ͻ��ˣ�ǹ�岥�Ŷ���done���ֱ۲��Ŷ���done�������������done����Ļ����done����������ǹ����������Ч��done����
		//����ʮ��׼��done ����ʮ��׼��done ����ʱ׼����ɢ ��beginplay��ӣ��ɿ�����������ͼʵ��done
		//�����ӵ�UI�����ӵ�UI 
		ClientFire();

		//�ͻ��˺�����
		//ClientRecoil();
	}
	else
	{
		StopPrimaryFire();
	}
}

void AFPSBaseCharacter::StartPrimaryFire()
{
	//�ж��ӵ�����ʣ��
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		
		//�����(�����������, ǹ����������Ч��, ���ٵ�ҩ�����߼��( ����,��ǹ����ǹ���ѻ�ǹ)���˺�Ӧ�ã���������)
		if(UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}
		//�ͻ��ˣ�ǹ�岥�Ŷ���done���ֱ۲��Ŷ���done�������������done����Ļ����done����������ǹ����������Ч��done����
		//����ʮ��׼��done ����ʮ��׼��done ����ʱ׼����ɢ ��beginplay��ӣ��ɿ�����������ͼʵ��done
		//�����ӵ�UI�����ӵ�UI 
		ClientFire();
		//���Զ�ȫ�Զ�����
		//������ʱ�� ÿ���̶�ʱ���������
		if(ServerPrimaryWeapon->IsAutomatic)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSBaseCharacter::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}
	
		
	}
	
}

void AFPSBaseCharacter::StopPrimaryFire()
{
	//����isFiring����
	ServerStopFire();
	//�رռ�ʱ����ÿ���̶�ʱ�����¼�ʱ
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	//���ú�������ر���
	ResetRecoil();
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
			//xyzȫ�������ƫ��
			FVector Vector = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
			//�ܴ�ƫ�Ʒ�Χ
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);

			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
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
			if (ServerPrimaryWeapon)
			{
			//����ң�Ӧ���˺�
			//����������˺��ӳ�,��������monster����,Ҳ���Բ����������˺��ӳɣ�������Ӧ���˺����������˺��Ŵ�
			DamagePlayer(HitResult.PhysMaterial.Get() ,HitResult.GetActor(), ServerPrimaryWeapon->BaseDamage, CameraLocation, HitResult, GetController(), this);
			}
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

void AFPSBaseCharacter::DamagePlayer(UPhysicalMaterial* PhysicalMaterial ,AActor* DamagedActor, float BaseDamage, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = BaseDamage;
	//�����ĸ�λ���ܵ���ͬ�˺�
	if(ServerPrimaryWeapon)
	{
		switch (PhysicalMaterial->SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1:
		{
			//head
			FinalDamage = BaseDamage* ServerPrimaryWeapon->HeadDamageRate;
			break;
		}
		case EPhysicalSurface::SurfaceType2:
		{
			//Body
			FinalDamage = BaseDamage* ServerPrimaryWeapon->BodyDamageRate;
			break;
		}
		case EPhysicalSurface::SurfaceType3:
		{
			//Arm
			FinalDamage = BaseDamage* ServerPrimaryWeapon->ArmDamageRate;
			break;
		}
		case EPhysicalSurface::SurfaceType4:
		{
			//Leg
			FinalDamage = BaseDamage* ServerPrimaryWeapon->LegDamageRate;
			break;
		}
		default:
			FinalDamage = BaseDamage;
			break;

		}
	}
	

	UGameplayStatics::ApplyPointDamage(DamagedActor, FinalDamage, HitFromDirection, HitInfo, EventInstigator, DamageCauser, UDamageType::StaticClass());

	//OnTakePointDamage
}

void AFPSBaseCharacter::OnHit(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
	UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType,
	AActor* DamageCauser)
{
	CurrentHealth -= Damage;
	//1. �ͻ���RPC 2.���ÿͻ���PlayerController��һ������(��ͼʵ��) 3. ʵ��PlayerUIѪ�����ٵĽӿ�
	if(CurrentHealth>0)
	{
		//�ͻ��˵��ú�������healthUI
		ClientUpdateHealthUI(CurrentHealth,MaxHealth);
	}
	else
	{
		//�����߼�

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
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::InputDropWeapon);

		//��R��������
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::InputReload);

	// �����ͨ������"ETriggerEvent"ö��ֵ���󶨵��˴������ⴥ�����¼�
	//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}

}
