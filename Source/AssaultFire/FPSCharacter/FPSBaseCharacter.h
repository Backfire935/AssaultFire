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

	//��ǿ����
	//����ӳ���
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "EnhancedInput | Context",meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_Action;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_MoveBase;

	//�����ƶ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveForward;

	//�����ƶ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveRight;

	//�����ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_LookUpRate;

	//�����ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction>  IA_LookRightRate;

	//��Ծ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Jump;


		//����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FirePressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FireReleased;

	//����shift��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftReleased;

	//����G��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_DropWeapon;


	//��Ϸ�����������
	UPROPERTY(EditAnywhere, Category = "Input")
	float TurnRateGamepad = 5.f;

	//�����˳���������ָ��
	UPROPERTY(meta = (AllowPrivateAccess = " true"))
		AWeaponBaseServer* ServerPrimaryWeapon;


	//��һ�˳���������ָ��
	UPROPERTY(meta = (AllowPrivateAccess = " true"))
		AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = " true"))
	EWeaponType CurrentWeaponType;

	AWeaponBaseClient* GetCurrentClientWeapon();

public:

protected:

private:

	UPROPERTY(EditAnywhere, Category= "WeaponType")
	EWeaponType WeaponType = EWeaponType::AK47;


	UPROPERTY(EditAnywhere, Category = "WeaponType")
		bool bIfSpawnWeapon = false;

	FTimerHandle CollisionTimer;

	UPROPERTY(EditDefaultsOnly)
		float CollisionDelay = 1.f;

	//Ҫ�����Եļ�����
	UPROPERTY()
		TArray<AActor* > IgnoreArray;
private:

	//����ʱ�Դ��趨������
	void StartWithKindOfWeapon(bool _bIfSpawnWeapon,EWeaponType _WeaponType);
	//��������
	void PurchaseWeapon(EWeaponType _WeaponType);
protected:

	virtual void BeginPlay() override;

	///�����
	void MoveForward(const FInputActionValue& InputValue);

	void MoveRight(const FInputActionValue& InputValue);

	void LookRightRate(const FInputActionValue& InputValue);

	void LookUpRate(const FInputActionValue& InputValue);

	void OnJump(const FInputActionValue& InputValue);

	void OnJumpStoping(const FInputActionValue& InputValue);

	//������shift��������
	void LowSpeedWalk(const FInputActionValue& InputValue);

	//�ɿ���shift��������
	void NormalSpeedWalk(const FInputActionValue& InputValue);
	///

	void FirePressed(const FInputActionValue& InputValue);

	void FireReleased(const FInputActionValue& InputValue);


	//��G��������
	void DropWeapon();
	///
	
	void CollisionTimerFinished();


public:
	AFPSBaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//RPC��ɫ����
	UFUNCTION(Server, Reliable)
	void ServerLowSpeedWalk();

	//RPC��ɫ�ָ��ٶ�
	UFUNCTION(Server, Reliable)
	void ServerNormalSpeedWalk();

	//����˿���
	UFUNCTION(Server, Reliable)
		void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

	//�ಥ�������忪�𶯻���̫��
	UFUNCTION(NetMulticast, Reliable)
		void MultiShooting();

	//�ಥ���ɵ���
	UFUNCTION(NetMulticast, Reliable)
		void MultiSpawnBulletDecal(FVector Location, FRotator Rotation);

	//�ͻ�������������
	UFUNCTION(Client, Reliable)
		void ClientEquipFPArmsPrimary();


	//�ͻ��˿���
		UFUNCTION(Client, Reliable)
			void ClientFire();

	//�ͻ��˸���UI
		UFUNCTION(Client, Reliable)
			void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);


	//��ɫװ������,װ������֮ǰ�ǵ���EquipWeapon()�ر���ײ
	void EquipPrimaryWeapon(AWeaponBaseServer* Weapon);


	//����������
	AWeaponBaseServer* GetServerPrimaryWeapon();

	void StartPrimaryFire();

	void StopPrimaryFire();

	//��ǹ���߼��
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

};










