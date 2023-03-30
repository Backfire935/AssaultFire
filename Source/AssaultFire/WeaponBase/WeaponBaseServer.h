// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBaseClient.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseServer.generated.h"


UENUM()
enum class EWeaponType :uint8
{
	AK47 UMETA(DisplayName = "AK47"),
	M4A1 UMETA(DisplayName = "M4A1"),
	MP7 UMETA(DisplayName = "MP7"),
	DesertEagle UMETA(DisplayName = "DesertEagle"),

};

UCLASS()
class ASSAULTFIRE_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()



public:
	//武器种类
	UPROPERTY(EditAnywhere)
		EWeaponType KindOfWeapon;

	//武器模型
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent *WeaponMesh;

	//武器圆形碰撞盒子,捡枪用的
	UPROPERTY(EditAnywhere)
	USphereComponent *SphereCollision;

	//客户端上生成的武器种类
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient> WeaponClient;


protected:

	virtual void BeginPlay() override;

public:	

	AWeaponBaseServer();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()
		void EquipWeapon();

	UFUNCTION(NetMulticast,Unreliable)
	void MultiShootingEffect();



	void DisplayWeaponEffect();

	void DropWeapon();

	UPROPERTY(EditAnywhere)
		UParticleSystem*  MuzzleFlash;//枪口火焰


	UPROPERTY(EditAnywhere)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
		int32 GunCurrentAmmo = 90;//备弹

	UPROPERTY(EditAnywhere,Replicated) //如果服务端改变了，客户端也跟着变
		int32 ClipCurrentAmmo = 10;//弹夹里还剩多少子弹

	UPROPERTY(EditAnywhere)
		int32 MaxClipAmmo = 30; //弹夹容量

	UPROPERTY(EditAnywhere,Category= "Anim")
		UAnimMontage* ServerTPSBodysShootAnimMontage;//第三人称射击动画蒙太奇

	UPROPERTY(EditAnywhere, Category = "Anim")
		UAnimMontage* ServerTPSBodysReloadAnimMontage;//第三人称换弹动画蒙太奇

	UPROPERTY(EditAnywhere)
		float BulletDistance = 1000.f;//攻击距离

	UPROPERTY(EditAnywhere)
		UMaterialInterface* BulletDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float BaseDamage = 20.f;

	//后面参照玩家血量进行属性同步，后期可以升级攻击倍率
	UPROPERTY(EditAnywhere, Category = "Damage")
		float HeadDamageRate = 2.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float ArmDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float LegDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float BodyDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		bool IsAutomatic = true;//是否为全自动武器

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		float AutoFireRate = 0.2;//自动射击频率

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		UCurveFloat* VerticalRecoilCurve;//枪械的垂直后坐力表

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		UCurveFloat* HorizontalRecoilCurve;//枪械的水平后坐力表

	UPROPERTY(EditAnywhere, Category = "Auto Weapon && HalfAuto Weapon")
		float MovingFireRandomRange = 300.f;//枪械的跑打随机后坐力偏移

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMin = 0;//手枪单发散射最小值

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMax = 0;//手枪单发散射最小值

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadRecoverZero = 0.5f;//手枪散射恢复平稳时间

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMaxIncrease = 0.5f;//手枪向右向上随机扩散增量

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMinIncrease = 0.5f;//手枪向左向下随机扩散增量
};


