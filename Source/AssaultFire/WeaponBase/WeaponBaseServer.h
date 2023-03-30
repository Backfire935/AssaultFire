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
	//��������
	UPROPERTY(EditAnywhere)
		EWeaponType KindOfWeapon;

	//����ģ��
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent *WeaponMesh;

	//����Բ����ײ����,��ǹ�õ�
	UPROPERTY(EditAnywhere)
	USphereComponent *SphereCollision;

	//�ͻ��������ɵ���������
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
		UParticleSystem*  MuzzleFlash;//ǹ�ڻ���


	UPROPERTY(EditAnywhere)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
		int32 GunCurrentAmmo = 90;//����

	UPROPERTY(EditAnywhere,Replicated) //�������˸ı��ˣ��ͻ���Ҳ���ű�
		int32 ClipCurrentAmmo = 10;//�����ﻹʣ�����ӵ�

	UPROPERTY(EditAnywhere)
		int32 MaxClipAmmo = 30; //��������

	UPROPERTY(EditAnywhere,Category= "Anim")
		UAnimMontage* ServerTPSBodysShootAnimMontage;//�����˳����������̫��

	UPROPERTY(EditAnywhere, Category = "Anim")
		UAnimMontage* ServerTPSBodysReloadAnimMontage;//�����˳ƻ���������̫��

	UPROPERTY(EditAnywhere)
		float BulletDistance = 1000.f;//��������

	UPROPERTY(EditAnywhere)
		UMaterialInterface* BulletDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float BaseDamage = 20.f;

	//����������Ѫ����������ͬ�������ڿ���������������
	UPROPERTY(EditAnywhere, Category = "Damage")
		float HeadDamageRate = 2.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float ArmDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float LegDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float BodyDamageRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		bool IsAutomatic = true;//�Ƿ�Ϊȫ�Զ�����

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		float AutoFireRate = 0.2;//�Զ����Ƶ��

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		UCurveFloat* VerticalRecoilCurve;//ǹе�Ĵ�ֱ��������

	UPROPERTY(EditAnywhere, Category = "Auto Weapon")
		UCurveFloat* HorizontalRecoilCurve;//ǹе��ˮƽ��������

	UPROPERTY(EditAnywhere, Category = "Auto Weapon && HalfAuto Weapon")
		float MovingFireRandomRange = 300.f;//ǹе���ܴ����������ƫ��

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMin = 0;//��ǹ����ɢ����Сֵ

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMax = 0;//��ǹ����ɢ����Сֵ

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadRecoverZero = 0.5f;//��ǹɢ��ָ�ƽ��ʱ��

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMaxIncrease = 0.5f;//��ǹ�������������ɢ����

	UPROPERTY(EditAnywhere, Category = "HalfAuto Weapon")
		float PistolSpreadMinIncrease = 0.5f;//��ǹ�������������ɢ����
};


