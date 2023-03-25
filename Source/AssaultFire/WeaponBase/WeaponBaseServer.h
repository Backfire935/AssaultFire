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

	UPROPERTY(EditAnywhere)
		UAnimMontage* ServerTPSBodysAnimMontage;//�����˳����������̫��

	UPROPERTY(EditAnywhere)
		float BulletDistance = 1000.f;//��������

	UPROPERTY(EditAnywhere)
		UMaterialInterface* BulletDecalMaterial;



};


