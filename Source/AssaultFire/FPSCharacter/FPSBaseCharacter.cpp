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

void AFPSBaseCharacter::ServerReloadPrimary_Implementation()
{
	//客户端手臂播放动画，服务器多播身体动画，数据更新,UI更改
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
			//附加武器模型到第一人称手臂上
			ClientPrimaryWeapon->K2_AttachToComponent(FPSArmsMesh, TEXT("Weapon_Socket"), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			ClientPrimaryWeapon->WeaponMesh->SetOnlyOwnerSee(true);
			//手臂动画

				//	更新子弹UI
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
			//这个属性越小，越远能看见贴花
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
		//获取武器的后坐力表
		VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
		HorizontalRecoilCurve = ServerPrimaryWeapon->HorizontalRecoilCurve;
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
		//调用客户端装备主武器
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

void AFPSBaseCharacter::AutoFire()
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
	}
}

void AFPSBaseCharacter::StartPrimaryFire()
{
	//判断子弹数量剩余
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
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
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSBaseCharacter::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
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
	//1. 客户端RPC 2.调用客户端PlayerController的一个方法(蓝图实现) 3. 实现PlayerUI血量减少的接口
	if(CurrentHealth>0)
	{
		//客户端调用函数更新healthUI
		ClientUpdateHealthUI(CurrentHealth,MaxHealth);
	}
	else
	{
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
	SpawnInfo.SpawnCollisionHandlingOverride =ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (_WeaponType)
	{
	case EWeaponType::AK47 :
		{
			//如果身上有武器了自然就不执行
			if(ServerPrimaryWeapon)
			{
				return;
			}
			else
			{
				//动态拿到AK47类
				//路径结尾加个_C表示是类 不然拿不到
					UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Weapon/AK47/BP_ServerAK47.BP_ServerAK47_C'") );

				AWeaponBaseServer * ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
	
				//主动关闭碰撞
				ServerWeapon->EquipWeapon();
				//生成主武器
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
