// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include"InputActionValue.h"
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
		TObjectPtr<UInputAction> IA_Fire;

	//按下shift静步减速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Shift;


	//游戏内鼠标灵敏度
	UPROPERTY(EditAnywhere, Category = "Input")
	float TurnRateGamepad = 5.f;

public:

protected:

private:


protected:

	
	virtual void BeginPlay() override;

	///输入绑定
	void MoveForward(const FInputActionValue& InputValue);

	void MoveRight(const FInputActionValue& InputValue);

	void LookRightRate(const FInputActionValue& InputValue);

	void LookUpRate(const FInputActionValue& InputValue);

	void OnJump(const FInputActionValue& InputValue);

	void OnJumpStoping(const FInputActionValue& InputValue);

	void Fire(const FInputActionValue& InputValue);

	//按下左shift静步慢走
	void LowSpeedWalk(const FInputActionValue& InputValue);

	//松开左shift正常行走
	void NormalSpeedWalk(const FInputActionValue& InputValue);
	///


public:

	AFPSBaseCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//RPC角色减速
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalk();
	void ServerLowSpeedWalk_Implementation();
	bool ServerLowSpeedWalk_Validate();

	//RPC角色恢复速度
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalk();
	void ServerNormalSpeedWalk_Implementation();
	bool ServerNormalSpeedWalk_Validate();

};
