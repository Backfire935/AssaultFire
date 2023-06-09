// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FPSBaseCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "Components/DecalComponent.h"
#include "Engine/SkeletalMeshSocket.h"
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
	//这个手臂模型是给第一人称的自己看的，别人看到就穿帮了
	FPSArmsMesh->SetOnlyOwnerSee(true);

	//这个模型是character自带的，用来给别人看，自己就不看了，不然重叠了
	GetMesh()->SetOwnerNoSee(true);
	//模型不需要碰撞，碰撞后面由胶囊体处理
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);


	
}

void AFPSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSBaseCharacter, bIsFiring, COND_None);
	DOREPLIFETIME_CONDITION(AFPSBaseCharacter, bIsReloading, COND_None);
	DOREPLIFETIME_CONDITION(AFPSBaseCharacter, CurrentWeaponType, COND_None);
	

}


void AFPSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	//绑定应用伤害函数的回调
	OnTakePointDamage.AddDynamic(this, &AFPSBaseCharacter::OnHit);
	//开始的时候带枪
	StartWithKindOfWeapon(bIfSpawnWeapon, WeaponType);

	FPSArmsAnimBP = FPSArmsMesh->GetAnimInstance();
	TPSBodysAnimBP = GetMesh()->GetAnimInstance();

	FPSPlayerController = Cast<AFPSPlayerController>(GetController());
	if(FPSPlayerController)
	{
		//创建并添加玩家UI
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
		// 判断哪个旋转方向是向前的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向前的向量
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}

}

void AFPSBaseCharacter::MoveRight(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if(Controller != nullptr && value != 0.f)
	{
		// 判断哪个旋转方向是向右的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向右的向量
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
		case EWeaponType::AK47:
		{
			 StartPrimaryFire();
			break;
		}
		case EWeaponType::M4A1:
		{
			StartPrimaryFire();
			break;
		}
		case EWeaponType::MP7:
		{
			StartPrimaryFire();
			break;
		}
		case EWeaponType::DesertEagle:
		{
			StartSecondaryFire();
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
	case EWeaponType::M4A1:
		{
			StopPrimaryFire();
			break;
		}
	case EWeaponType::MP7:
	{
		StopPrimaryFire();
		break;
	}
	case EWeaponType::DesertEagle:
	{
		StopSecondaryFire();
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
		//多播NetMulticast
		ServerPrimaryWeapon->MultiShootingEffect();

		//开枪 减一发子弹
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;

		//多播 播放身体射击动画蒙太奇
		MultiShooting();

		//让客户端更新UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

	}

	bIsFiring = true;
	RifleLineTrace(CameraLocation, CameraRotation, bIsMoving);
}

void AFPSBaseCharacter::ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool bIsMoving)
{

	if (ServerSecondaryWeapon)
	{
		if(!ServerSecondaryWeapon->IsAutomatic)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			//在这里指定延迟后的回调函数
			ActionInfo.ExecutionFunction = TEXT("DelaySpreedWeaponShootCallBack");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			//C++中实现delay较为麻烦
			UKismetSystemLibrary::Delay(this, ServerSecondaryWeapon->PistolSpreadRecoverZero, ActionInfo);

		}

		//多播NetMulticast
		ServerSecondaryWeapon->MultiShootingEffect();

		//开枪 减一发子弹
		ServerSecondaryWeapon->ClipCurrentAmmo -= 1;

		//多播 播放身体射击动画蒙太奇
		MultiShooting();

		//让客户端更新UI
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

	}

	bIsFiring = true;
	PistolLineTrace(CameraLocation, CameraRotation, bIsMoving);

}

void AFPSBaseCharacter::ServerReloadPrimary_Implementation()
{
	if(ServerPrimaryWeapon)
	{
		//子弹不够或者满的都不换
		if(ServerPrimaryWeapon->ClipCurrentAmmo < ServerPrimaryWeapon->MaxClipAmmo && ServerPrimaryWeapon->GunCurrentAmmo > 0)
		{
			//客户端手臂播放动画，
			ClientReload();
			//服务器多播身体动画，
			MultiReloadAnimation();

			bIsReloading = true;

			//数据更新,UI更改
			if(ClientPrimaryWeapon)
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.CallbackTarget = this;
				//在这里指定延迟后的回调函数
				ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
				ActionInfo.UUID = FMath::Rand();
				ActionInfo.Linkage = 0;
				//C++中实现delay较为麻烦
				UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(), ActionInfo);
			}
			
		}
	
	}

	
}

void AFPSBaseCharacter::ServerReloadSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		//子弹不够或者满的都不换
		if (ServerSecondaryWeapon->ClipCurrentAmmo < ServerSecondaryWeapon->MaxClipAmmo && ServerSecondaryWeapon->GunCurrentAmmo > 0)
		{
			//客户端手臂播放动画，
			ClientReload();
			//服务器多播身体动画，
			MultiReloadAnimation();

			bIsReloading = true;

			//数据更新,UI更改
			if (ClientSecondaryWeapon)
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.CallbackTarget = this;
				//在这里指定延迟后的回调函数
				ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
				ActionInfo.UUID = FMath::Rand();
				ActionInfo.Linkage = 0;
				//C++中实现delay较为麻烦
				UKismetSystemLibrary::Delay(this, ClientSecondaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(), ActionInfo);
			}

		}

	}
}

void AFPSBaseCharacter::ServerStopFire_Implementation()
{
	bIsFiring = false;
}

void AFPSBaseCharacter::MultiShooting_Implementation()
{

	if(TPSBodysAnimBP)
	{
		AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
				if (CurrentServerWeapon && CurrentServerWeapon->ServerTPSBodysShootAnimMontage)
				{
					TPSBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPSBodysShootAnimMontage);
				}
		
	}
}

void AFPSBaseCharacter::MultiReloadAnimation_Implementation()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
	if (TPSBodysAnimBP)
	{
		if (CurrentServerWeapon && CurrentServerWeapon->ServerTPSBodysReloadAnimMontage)
		{
			TPSBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPSBodysReloadAnimMontage);
		}
	}
	
}

void AFPSBaseCharacter::ClientEquipFPArmsPrimary_Implementation()
{
	if(ServerPrimaryWeapon)
	{
		if(ClientPrimaryWeapon)
		{
						//如果身上有武器了自然就不执行
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			//客户端上生成的武器种类应该与服务端的保持一致而不是被限定的
			//调用角色在客户端上生成武器
			if(ServerPrimaryWeapon->WeaponClient)
			{
				//如果已经被限定了就执行默认实现,比如开局指定某把武器
				ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->WeaponClient, GetActorTransform(), SpawnInfo);
			}
			//比如从地上
			else
			{
				//不存在就return,防止编译器崩溃
				UE_LOG(LogTemp, Warning, TEXT("Server Weapon Doesn't Set Client Weapon"));
				return;
			}

			//不同的武器使用不同的socket名字
			//默认AK47插槽
			FName WeaponSocketName = TEXT("WeaponSocket");
			if (CurrentWeaponType == EWeaponType::AK47) WeaponSocketName = TEXT("WeaponSocket");
			//M4A1插槽
			if (CurrentWeaponType == EWeaponType::M4A1) WeaponSocketName = TEXT("M4A1_Socket");
			//MP7插槽
			if (CurrentWeaponType == EWeaponType::MP7) WeaponSocketName = TEXT("WeaponSocket");

			if(ClientPrimaryWeapon)
			{
				//手臂动画混合
				UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPose);
				//附加武器模型到第一人称手臂上

				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Weapon Location %.0f %.0f %.0f"), ClientPrimaryWeapon->GetActorLocation().X, ClientPrimaryWeapon->GetActorLocation().Y, ClientPrimaryWeapon->GetActorLocation().Z));

				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Socket Location %.0f %.0f %.0f"),FPSArmsMesh->GetSocketLocation(WeaponSocketName).X, FPSArmsMesh->GetSocketLocation(WeaponSocketName).Y, FPSArmsMesh->GetSocketLocation(WeaponSocketName).Z));

				ClientPrimaryWeapon->K2_AttachToComponent(FPSArmsMesh, WeaponSocketName, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);

				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Weapon Location %.0f %.0f %.0f"), ClientPrimaryWeapon->GetActorLocation().X, ClientPrimaryWeapon->GetActorLocation().Y, ClientPrimaryWeapon->GetActorLocation().Z));

				ClientPrimaryWeapon->WeaponMesh->SetOnlyOwnerSee(true);

			}

				//	更新子弹UI
			if(FPSPlayerController)
			{
				FPSPlayerController->UpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			}
		}
		
	}
}

AWeaponBaseServer* AFPSBaseCharacter::GetServerPrimaryWeapon()
{
	return ServerPrimaryWeapon;
}


AWeaponBaseClient* AFPSBaseCharacter::GetCurrentClientWeapon()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::AK47 :
		{
		return ClientPrimaryWeapon;
		}
	case EWeaponType::M4A1:
	{
		return ClientPrimaryWeapon;
	}
	case EWeaponType::MP7:
	{
		return ClientPrimaryWeapon;
	}
	case EWeaponType::DesertEagle:
	{
		return ClientSecondaryWeapon;
	}

		default:
			return ClientPrimaryWeapon;
	}

}

AWeaponBaseServer* AFPSBaseCharacter::GetCurrentServerWeapon()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::AK47:
	{
		return ServerPrimaryWeapon;
	}
	case EWeaponType::M4A1:
	{
		return ServerPrimaryWeapon;
	}
	case EWeaponType::MP7:
	{
		return ServerPrimaryWeapon;
	}
	case EWeaponType::DesertEagle:
	{
		return ServerSecondaryWeapon;
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


//交给武器类去处理
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
			case EWeaponType::M4A1:
			{
				ServerReloadPrimary();
				break;
			}
			case EWeaponType::MP7:
			{
				ServerReloadPrimary();
				break;
			}
			case EWeaponType::DesertEagle:
			{
				ServerReloadSecondary();
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
	AWeaponBaseServer *CurrentWeapon = GetCurrentServerWeapon();
	if(CurrentWeapon->BulletDecalMaterial)
	{
		UDecalComponent * Decal =  UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CurrentWeapon->BulletDecalMaterial,FVector(8,8,8),Location, Rotation , 10.f);
		if(Decal)
		{
			//这个属性越小，越远能看见贴花
			Decal->SetFadeScreenSize(0.001);
		}
	}
}


void AFPSBaseCharacter::ClientRecoil_Implementation()
{
	UCurveFloat* VerticalRecoilCurve = nullptr;
	UCurveFloat* HorizontalRecoilCurve = nullptr;
	AWeaponBaseServer* CurrentWeapon = GetCurrentServerWeapon();
	if(CurrentWeapon)
	{
		//获取武器的后坐力表
		VerticalRecoilCurve = CurrentWeapon->VerticalRecoilCurve;
		HorizontalRecoilCurve = CurrentWeapon->HorizontalRecoilCurve;
	}
	//表的横坐标加0.1
	RecoilXCoordPerShoot += 0.1;
	RecoilYCoordPerShoot += 0.1;

	if(VerticalRecoilCurve)
	{
		//按表的横坐标获取对应的float值并应用到新的垂直后坐力数值上
		NewVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);
	}

	if (HorizontalRecoilCurve)
	{
		//按表的横坐标获取对应的float值并应用到新的水平后坐力数值上
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
	//手臂播放动画蒙太奇
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if(CurrentClientWeapon)
	{
		UAnimMontage* ClientArmsReloadingMontage = GetCurrentClientWeapon()->ClientArmsReloadAnimMontage;
		FPSArmsAnimBP->Montage_Play(ClientArmsReloadingMontage);
		CurrentClientWeapon->PlayReloadAnimation();
	}
	
}

void AFPSBaseCharacter::ClientEquipFPArmsSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		if (ClientSecondaryWeapon)
		{
			//如果身上有武器了自然就不执行
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//客户端上生成的武器种类应该与服务端的保持一致而不是被限定的
			//调用角色在客户端上生成武器
			if (ServerSecondaryWeapon->WeaponClient)
			{
				//如果已经被限定了就执行默认实现,比如开局指定某把武器
				ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondaryWeapon->WeaponClient, GetActorTransform(), SpawnInfo);
			}
			//比如从地上
			else
			{
				//不存在就return,防止编译器崩溃
				UE_LOG(LogTemp, Warning, TEXT("Server Weapon Doesn't Set Client Weapon"));
				return;
			}

			//不同的武器使用不同的socket名字
			FName WeaponSocketName = TEXT("WeaponSocket");

			if (ClientSecondaryWeapon)
			{
				//手臂动画混合
				UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPose);
				//附加武器模型到第一人称手臂上

				ClientSecondaryWeapon->K2_AttachToComponent(FPSArmsMesh, WeaponSocketName, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);

				ClientSecondaryWeapon->WeaponMesh->SetOnlyOwnerSee(true);

			}

			//	更新子弹UI
			if (FPSPlayerController)
			{
				FPSPlayerController->UpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);
			}
		}

	}
}

void AFPSBaseCharacter::EquipPrimaryWeapon(AWeaponBaseServer* WeaponBaseServer)
{
	if(ServerPrimaryWeapon)
	{
		//如果身上有武器了自然就不执行
		return;
	}
	else
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		//将自身模型，添加到插槽下，以跟随目标方式附加，这三个参数分别为位置旋转尺寸，最后一个是是否跟随被附加的物体进行模拟物理，即角色死的时候模拟物理的话枪械模型也跟着模拟物理
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(),TEXT("Weapon_Socket"),EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		ServerPrimaryWeapon->WeaponMesh->SetOwnerNoSee(true);
		CurrentWeaponType = ServerPrimaryWeapon->KindOfWeapon;
		//调用客户端装备主武器
		ClientEquipFPArmsPrimary();
	}
}

void AFPSBaseCharacter::EquipSecondaryWeapon(AWeaponBaseServer* Weapon)
{
	if (ServerSecondaryWeapon)
	{
		//如果身上有武器了自然就不执行
		return;
	}
	else
	{
		ServerSecondaryWeapon = Weapon;
		ServerSecondaryWeapon->SetOwner(this);
		//将自身模型，添加到插槽下，以跟随目标方式附加，这三个参数分别为位置旋转尺寸，最后一个是是否跟随被附加的物体进行模拟物理，即角色死的时候模拟物理的话枪械模型也跟着模拟物理
		ServerSecondaryWeapon->K2_AttachToComponent(GetMesh(), TEXT("Weapon_Socket"), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		ServerSecondaryWeapon->WeaponMesh->SetOwnerNoSee(true);
		CurrentWeaponType = ServerSecondaryWeapon->KindOfWeapon;
		//调用客户端装备副武器
		//ClientEquipFPArmsPrimary();
		ClientEquipFPArmsSecondary();

	}
}


void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* WeaponBaseClient = GetCurrentClientWeapon();
	if (WeaponBaseClient)
	{
		//播放射击动画
		WeaponBaseClient->PlayShootAnimation();
		//手臂播放动画
		UAnimMontage* ClientArmsFireMontage = WeaponBaseClient->ClientArmsFireAnimMontage;
			
		if (ClientArmsFireMontage)
		{
			FPSArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);//设置一倍速播放 
			FPSArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		}

		//客户端后坐力
		ClientRecoil();

		//播放射击声音，播放枪口闪光
		WeaponBaseClient->DisplayWeaponEffect();

		if(FPSPlayerController)
		{
			//播放屏幕抖动
			FPSPlayerController->PlayerCameraShake(WeaponBaseClient->CameraShakeClass);

			//播放准星扩散动画
			FPSPlayerController->DoCrossHairRecoilAnim(); 
		}
	
	}
	
}

void AFPSBaseCharacter::RifleAutoFire()
{
	//判断子弹数量剩余
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//服务端(播放射击声音, 枪口闪光粒子效果, 减少弹药，射线检测( 三种,步枪，手枪，狙击枪)，伤害应用，弹孔生成)
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}
		//客户端（枪体播放动画done，手臂播放动画done，播放射击声音done，屏幕抖动done，后坐力，枪口闪光粒子效果done，）
		//创建十字准星done 播放十字准星done 开火时准星扩散 在beginplay添加，由控制器类在蓝图实现done
		//创建子弹UI更新子弹UI 
		ClientFire();

		//客户端后坐力
		//ClientRecoil();
	}
	else
	{
		StopPrimaryFire();
		ServerReloadPrimary();
	}
}

void AFPSBaseCharacter::PistolAutoFire()
{
	//判断子弹数量剩余
	if (ServerSecondaryWeapon->ClipCurrentAmmo > 0)
	{
		//服务端(播放射击声音, 枪口闪光粒子效果, 减少弹药，射线检测( 三种,步枪，手枪，狙击枪)，伤害应用，弹孔生成)
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}
		//客户端（枪体播放动画done，手臂播放动画done，播放射击声音done，屏幕抖动done，后坐力，枪口闪光粒子效果done，）
		//创建十字准星done 播放十字准星done 开火时准星扩散 在beginplay添加，由控制器类在蓝图实现done
		//创建子弹UI更新子弹UI 
		ClientFire();

		//客户端后坐力
		//ClientRecoil();
	}
	else
	{
		StopSecondaryFire();
		ServerReloadSecondary();
	}
}

void AFPSBaseCharacter::StartPrimaryFire()
{
	//判断子弹数量剩余
	if(ServerPrimaryWeapon && ServerPrimaryWeapon->ClipCurrentAmmo > 0 && !bIsReloading)
	{
		
		//服务端(播放射击声音, 枪口闪光粒子效果, 减少弹药，射线检测( 三种,步枪，手枪，狙击枪)，伤害应用，弹孔生成)
		if(UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}
		//客户端（枪体播放动画done，手臂播放动画done，播放射击声音done，屏幕抖动done，后坐力，枪口闪光粒子效果done，）
		//创建十字准星done 播放十字准星done 开火时准星扩散 在beginplay添加，由控制器类在蓝图实现done
		//创建子弹UI更新子弹UI 
		ClientFire();
		//半自动全自动处理
		//开启计时器 每隔固定时间重新射击
		if(ServerPrimaryWeapon->IsAutomatic)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSBaseCharacter::RifleAutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}
	
		
	}
	else
	{
		if(!bIsReloading)
		{
			ServerReloadPrimary();
		}
	}
	
}

void AFPSBaseCharacter::StopPrimaryFire()
{
	//更改isFiring变量
	ServerStopFire();
	//关闭计时器，每隔固定时间重新计时
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	//重置后坐力相关变量
	ResetRecoil();
}

void AFPSBaseCharacter::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector =  UKismetMathLibrary::GetForwardVector(CameraRotation);
	FHitResult HitResult;

	//自己和队友 先把自己加进去
	IgnoreArray.Add(this);

	if (ServerPrimaryWeapon)
	{
		//是否移动导致不同的location检测
		if (bIsMoving)
		{
			//xyz全部加随机偏移
			FVector Vector = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
			//跑打偏移范围
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

	//返回是否命中
	bool  HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None , HitResult,true,FLinearColor::Blue , FLinearColor::Red , 10.F );

	if(HitSuccess)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit Actor Name : %s"), *HitResult.GetActor()->GetName()));
		if(AFPSBaseCharacter * HittedCharacter = Cast<AFPSBaseCharacter>(HitResult.GetActor()))
		{
			if (ServerPrimaryWeapon)
			{
			//打到玩家，应用伤害
			//这里可以做伤害加成,可以做个monster基类,也可以不在这里做伤害加成，而是在应用伤害的类里做伤害放大
			DamagePlayer(HitResult.PhysMaterial.Get() ,HitResult.GetActor(), ServerPrimaryWeapon->BaseDamage, CameraLocation, HitResult, GetController(), this);
			}
		}
		else
		{
			//命中物体的法线向量
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			//打到杂物 生成广播贴花
			MultiSpawnBulletDecal(HitResult.Location, XRotator );
		}
	}
}

void AFPSBaseCharacter::StartSecondaryFire()
{
	//判断子弹数量剩余
	if (ServerSecondaryWeapon && ServerSecondaryWeapon->ClipCurrentAmmo > 0 && !bIsReloading)
	{
	
			//服务端(播放射击声音, 枪口闪光粒子效果, 减少弹药，射线检测( 三种,步枪，手枪，狙击枪)，伤害应用，弹孔生成)
			if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
			{
				ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
			}
			else
			{
				ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
			}

		//客户端（枪体播放动画done，手臂播放动画done，播放射击声音done，屏幕抖动done，后坐力，枪口闪光粒子效果done，）
		//创建十字准星done 播放十字准星done 开火时准星扩散 在beginplay添加，由控制器类在蓝图实现done
		//创建子弹UI更新子弹UI 
		ClientFire();
		//半自动全自动处理
		//开启计时器 每隔固定时间重新射击
		if (ServerSecondaryWeapon->IsAutomatic)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSBaseCharacter::PistolAutoFire, ServerSecondaryWeapon->AutoFireRate, true);
		}


	}
	else
	{
		if (!bIsReloading)
		{
			ServerReloadSecondary();
		}
	}
}

void AFPSBaseCharacter::StopSecondaryFire()
{
	//更改isFiring变量
	ServerStopFire();
	//关闭计时器，每隔固定时间重新计时
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	//重置后坐力相关变量
	ResetRecoil();
}

void AFPSBaseCharacter::PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	FHitResult HitResult;

	//自己和队友 先把自己加进去
	IgnoreArray.Add(this);

	if (ServerSecondaryWeapon)
	{

		//是否移动导致不同的location检测
		if (bIsMoving)
		{
			//连发手枪
			if (ServerSecondaryWeapon->IsAutomatic)
				{
				//xyz全部加随机偏移
				FVector Vector = CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;
				//跑打偏移范围
				float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
				float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
				float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
				EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
				}
			//半自动手枪
			else
				{
					//开启时加个摄像机旋转偏移,根据连续按鼠标的快慢决定，连续越快，偏移越大
					FRotator Rotator;
					Rotator.Roll = CameraRotation.Roll;
					Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(ServerSecondaryWeapon->PistolSpreadMin, ServerSecondaryWeapon->PistolSpreadMax);
					Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(ServerSecondaryWeapon->PistolSpreadMin, ServerSecondaryWeapon->PistolSpreadMax);
					CameraForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);

					FVector  Vector =  CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;

					//跑打偏移范围
					float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
					float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
					float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);

				//连续射击偏移叠加跑打偏移
					EndLocation = FVector(RandomX + Vector.X, RandomY + Vector.Y, RandomZ + Vector.Z);
				}

		}
		else
		{
			//连发手枪
			if (ServerSecondaryWeapon->IsAutomatic)
			{
				CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
				EndLocation = CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;
			}
			//半自动手枪
			else
			{
				//连续射击偏移
				FRotator Rotator;
				Rotator.Roll = CameraRotation.Roll;
				Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(ServerSecondaryWeapon->PistolSpreadMin, ServerSecondaryWeapon->PistolSpreadMax);
				Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(ServerSecondaryWeapon->PistolSpreadMin, ServerSecondaryWeapon->PistolSpreadMax);

				CameraForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);

				EndLocation = CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;
			}
		}
	}

	//返回是否命中
	bool  HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None, HitResult, true, FLinearColor::Blue, FLinearColor::Red, 10.F);

	//连续射击的偏移量增加
	ServerSecondaryWeapon->PistolSpreadMax += ServerSecondaryWeapon->PistolSpreadMaxIncrease;
	ServerSecondaryWeapon->PistolSpreadMin -= ServerSecondaryWeapon->PistolSpreadMinIncrease;

	if (HitSuccess)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit Actor Name : %s"), *HitResult.GetActor()->GetName()));
		if (AFPSBaseCharacter* HittedCharacter = Cast<AFPSBaseCharacter>(HitResult.GetActor()))
		{
			if (ServerSecondaryWeapon)
			{
				//打到玩家，应用伤害
				//这里可以做伤害加成,可以做个monster基类,也可以不在这里做伤害加成，而是在应用伤害的类里做伤害放大
				DamagePlayer(HitResult.PhysMaterial.Get(), HitResult.GetActor(), ServerSecondaryWeapon->BaseDamage, CameraLocation, HitResult, GetController(), this);
			}
		}
		else
		{
			//命中物体的法线向量
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			//打到杂物 生成广播贴花
			MultiSpawnBulletDecal(HitResult.Location, XRotator);
		}
	}
}

void AFPSBaseCharacter::DelaySpreedWeaponShootCallBack()
{
	ServerSecondaryWeapon->PistolSpreadMin = 0;
	ServerSecondaryWeapon->PistolSpreadMax = 0;
}

void AFPSBaseCharacter::DelayPlayArmReloadCallBack()
{
	AWeaponBaseServer* CurrentWeapon = GetCurrentServerWeapon();
	if(CurrentWeapon)
	{
		int32 _GunCurrentAmmo = CurrentWeapon->GunCurrentAmmo;//身上的子弹
		int32 _ClipCurrentAmmo = CurrentWeapon->ClipCurrentAmmo;//弹夹还剩多少
		int32 _MaxClipAmmo = CurrentWeapon->MaxClipAmmo;//弹夹容量

		//是否装填全部子弹
		if (_MaxClipAmmo - _ClipCurrentAmmo > _GunCurrentAmmo)//要装填的子弹>还剩下的子弹 说明身上的子弹不够填满弹夹了 此时全部子弹扔进去但还填不满弹夹
		{
			_ClipCurrentAmmo += _GunCurrentAmmo;
			_GunCurrentAmmo -= _GunCurrentAmmo;
		}
		else//要装填的子弹<=还剩下的子弹 说明身上的子弹足够填满弹夹 此时把弹夹填满
		{
			//身上的子弹减掉消耗的部分
			_GunCurrentAmmo -= _MaxClipAmmo - _ClipCurrentAmmo;
			//弹夹填充要补充的子弹
			_ClipCurrentAmmo = _MaxClipAmmo;
		}

		//变量设置回去
		CurrentWeapon->GunCurrentAmmo = _GunCurrentAmmo;
		CurrentWeapon->ClipCurrentAmmo = _ClipCurrentAmmo;
		CurrentWeapon->MaxClipAmmo = _MaxClipAmmo;

		ClientUpdateAmmoUI(_ClipCurrentAmmo, _GunCurrentAmmo);
		bIsReloading = false;
	}


}

void AFPSBaseCharacter::DamagePlayer(UPhysicalMaterial* PhysicalMaterial ,AActor* DamagedActor, float BaseDamage, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = BaseDamage;
	//身体四个位置受到不同伤害
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
	if (CurrentHealth <= 0) CurrentHealth = 0;
	ClientUpdateHealthUI(CurrentHealth, MaxHealth);
	//1. 客户端RPC 2.调用客户端PlayerController的一个方法(蓝图实现) 3. 实现PlayerUI血量减少的接口
	if(CurrentHealth==0)
	{
		//客户端调用函数更新healthUI
		//死亡逻辑
	
	}

		

	
	
}

void AFPSBaseCharacter::StartWithKindOfWeapon(bool _bIfSpawnWeapon,EWeaponType _WeaponType)
{
	if (bIfSpawnWeapon == false) return;
	//只会在服务器上调用
	if(HasAuthority())
	{
		//仅服务器买武器
		PurchaseWeapon(_WeaponType);
	}
}

void AFPSBaseCharacter::PurchaseWeapon(EWeaponType _WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (_WeaponType)
	{
		case EWeaponType::AK47:
		{
			//动态拿到AK47类
			//路径结尾加个_C表示是类 不然拿不到
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/AK47/BP_ServerAK47.BP_ServerAK47_C'"));
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
			CurrentWeaponType = EWeaponType::AK47;
			//主动关闭碰撞
			ServerWeapon->EquipWeapon();
			//生成主武器
			EquipPrimaryWeapon(ServerWeapon);
		break;
		}
	case EWeaponType::M4A1:
		{
			//动态拿到M4A1类
			//路径结尾加个_C表示是类 不然拿不到
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/M4A1/BP_ServerM4A1.BP_ServerM4A1_C'"));
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
			//主动关闭碰撞
			CurrentWeaponType = EWeaponType::M4A1;
			ServerWeapon->EquipWeapon();
			//生成主武器
			EquipPrimaryWeapon(ServerWeapon);
		}
			case EWeaponType::MP7:
			{
						//动态拿到MP7类
						UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/MP7/BP_ServerMP7.BP_ServerMP7_C'"));
						AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
						//主动关闭碰撞
						CurrentWeaponType = EWeaponType::MP7;
						ServerWeapon->EquipWeapon();
						//生成主武器
						EquipPrimaryWeapon(ServerWeapon);
			}

			case EWeaponType::DesertEagle:
			{
						//动态拿到DeseretEagle类
						UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/DesertEagle/BP_ServerDesertEagle.BP_ServerDesertEagle_C'"));
						AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
						//主动关闭碰撞
						CurrentWeaponType = EWeaponType::DesertEagle;
						ServerWeapon->EquipWeapon();
						//生成主武器
						EquipSecondaryWeapon(ServerWeapon);
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

	//使用控制器控制旋转pitch和yaw
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	
	if(APlayerController * PC = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) )
		{

			//先清除映射避免重复添加
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_Action, 0);
			Subsystem->AddMappingContext(IMC_MoveBase, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if(EnhancedInputComponent)
	{

		//移动
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::MoveRight);
		//旋转
		EnhancedInputComponent->BindAction(IA_LookRightRate, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LookRightRate);
		EnhancedInputComponent->BindAction(IA_LookUpRate, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LookUpRate);
		//跳跃
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AFPSBaseCharacter::OnJump);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AFPSBaseCharacter::OnJumpStoping);

		//射击
		EnhancedInputComponent->BindAction(IA_FirePressed, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::FirePressed);

		EnhancedInputComponent->BindAction(IA_FireReleased, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::FireReleased);

		//shift静步	
		EnhancedInputComponent->BindAction(IA_ShiftPressed, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::LowSpeedWalk);

		EnhancedInputComponent->BindAction(IA_ShiftReleased, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::NormalSpeedWalk);

		//按G丢弃武器
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::InputDropWeapon);

		//按R武器换弹
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::InputReload);

	// 你可以通过更改"ETriggerEvent"枚举值，绑定到此处的任意触发器事件
	//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}

}
