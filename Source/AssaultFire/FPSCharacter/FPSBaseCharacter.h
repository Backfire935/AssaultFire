// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include"InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "AssaultFire/PlayerController/FPSPlayerController.h"

#include "AssaultFire/WeaponBase/WeaponBaseServer.h"

#include "Camera/CameraComponent.h"
#include "FPSBaseCharacter.generated.h"

UCLASS()
class ASSAULTFIRE_API AFPSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/*UPROPERTY(VisibleAnywhere, Category = camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;*/

	UPROPERTY(VisibleAnywhere, Category = camera , meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* PlayerCamera;

	UPROPERTY(VisibleAnywhere, Category = Character, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent * FPSArmsMesh;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AFPSPlayerController* FPSPlayerController;

	UPROPERTY(Category=Character, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* FPSArmsAnimBP;

	UPROPERTY(Category = Character, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* TPSBodysAnimBP;

	//增强输入
	//两个映射表
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "EnhancedInput | Context",meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_Action;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_MoveBase;

	//上下移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveForward;

	//左右移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveRight;

	//上下视角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_LookUpRate;

	//左右视角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction>  IA_LookRightRate;

	//跳跃
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Jump;


		//开火
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FirePressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FireReleased;

	//按下shift静步减速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftReleased;

	//按下G丢弃武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_DropWeapon;

	//按下R武器换弹
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Reload;


	//游戏内鼠标灵敏度
	UPROPERTY(EditAnywhere, Category = "Input")
	float TurnRateGamepad = 5.f;

	//第三人称主武器的指针
	UPROPERTY(meta = (AllowPrivateAccess = " true"))
		AWeaponBaseServer* ServerPrimaryWeapon;


	//第一人称主武器的指针
	UPROPERTY(meta = (AllowPrivateAccess = " true"))
		AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = " true"))
	EWeaponType CurrentWeaponType;


public:

	//垂直后坐力
	float NewVerticalRecoilAmount = 0.f;
	float OldVerticalRecoilAmount = 0.f;
	float VerticalRecoilAmount = 0.f;
	//每次射击的X坐标
	float RecoilXCoordPerShoot = 0.f;
	//水平后座力
	float NewHorizontalRecoilAmount = 0.f;
	float OldHorizontalRecoilAmount = 0.f;
	float HorizontalRecoilAmount = 0.f;
	//每次射击的y坐标
	float RecoilYCoordPerShoot = 0.f;

	void ResetRecoil();

protected:

private:

	UPROPERTY(EditAnywhere, Category= "WeaponType")
	EWeaponType WeaponType = EWeaponType::AK47;


	UPROPERTY(EditAnywhere, Category = "WeaponType")
		bool bIfSpawnWeapon = false;

	FTimerHandle CollisionTimer;

	UPROPERTY(EditDefaultsOnly)
		float CollisionDelay = 1.f;

	//要被忽略的检测对象
	UPROPERTY()
		TArray<AActor* > IgnoreArray;

	UPROPERTY(EditAnywhere, Category = "Health")
		float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health")
		float MaxHealth = 100.f;


private:

	//出生时自带设定的武器
	void StartWithKindOfWeapon(bool _bIfSpawnWeapon,EWeaponType _WeaponType);
	//购买武器
	void PurchaseWeapon(EWeaponType _WeaponType);
protected:

	virtual void BeginPlay() override;

	///输入绑定
	void MoveForward(const FInputActionValue& InputValue);

	void MoveRight(const FInputActionValue& InputValue);

	void LookRightRate(const FInputActionValue& InputValue);

	void LookUpRate(const FInputActionValue& InputValue);

	void OnJump(const FInputActionValue& InputValue);

	void OnJumpStoping(const FInputActionValue& InputValue);

	//按下左shift静步慢走
	void LowSpeedWalk(const FInputActionValue& InputValue);

	//松开左shift正常行走
	void NormalSpeedWalk(const FInputActionValue& InputValue);
	///

	void FirePressed(const FInputActionValue& InputValue);

	void FireReleased(const FInputActionValue& InputValue);


	//按G丢弃武器
	void InputDropWeapon();

	//按R武器换弹
	void InputReload();
	///
	
	void CollisionTimerFinished();


public:
	AFPSBaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//RPC角色减速
	UFUNCTION(Server, Reliable)
	void ServerLowSpeedWalk();

	//RPC角色恢复速度
	UFUNCTION(Server, Reliable)
	void ServerNormalSpeedWalk();

	//服务端开火
	UFUNCTION(Server, Reliable)
		void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

	//服务端换弹
	UFUNCTION(Server, Reliable)
		void ServerReloadPrimary();

	//服务端停止射击
	UFUNCTION(Server, Reliable)
		void ServerStopFire();

	//多播播放身体开火动画蒙太奇
	UFUNCTION(NetMulticast, Reliable)
		void MultiShooting();

	//多播换弹动画
	UFUNCTION(NetMulticast, Reliable)
		void MultiReloadAnimation();

	//多播生成弹孔
	UFUNCTION(NetMulticast, Reliable)
		void MultiSpawnBulletDecal(FVector Location, FRotator Rotation);

	//客户端生成主武器
	UFUNCTION(Client, Reliable)
		void ClientEquipFPArmsPrimary();


	//客户端开火
		UFUNCTION(Client, Reliable)
			void ClientFire();

	//客户端更新AmmoUI
		UFUNCTION(Client, Reliable)
			void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//客户端更新healthUI
		UFUNCTION(Client, Reliable)
			void ClientUpdateHealthUI(float NewHealth,float _MaxHealth);

		//客户端后坐力实现
		UFUNCTION(Client, Reliable)
			void ClientRecoil();

		//客户端后坐力实现
		UFUNCTION(Client, Reliable)
			void ClientReload();

	//角色装备武器,装备武器之前记得用EquipWeapon()关闭碰撞
	void EquipPrimaryWeapon(AWeaponBaseServer* Weapon);


	//返回主武器
	AWeaponBaseServer* GetServerPrimaryWeapon();
	AWeaponBaseClient* GetCurrentClientWeapon();
	AWeaponBaseServer* GetCurrentServerWeapon();

	FTimerHandle AutoFireTimerHandle;


	void AutoFire();
	//步枪的射击相关
	void StartPrimaryFire();
	void StopPrimaryFire();
	//步枪射线检测
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

	//是否正在开火
	UPROPERTY(Replicated)
		bool bIsFiring = false;
	//是否在换弹
	UPROPERTY(Replicated)
		bool bIsReloading = false;

	UFUNCTION()
		void DelayPlayArmReloadCallBack();

	//狙击枪射击相关

	//手枪射击相关

	//应用伤害到玩家
	void DamagePlayer(UPhysicalMaterial * PhysicalMaterial, AActor* DamagedActor, float BaseDamage, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser);

	//OnTakePointDamage应用伤害函数的回调
	UFUNCTION()
		void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

};

















