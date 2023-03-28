// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClient.generated.h"

UCLASS()
class ASSAULTFIRE_API AWeaponBaseClient : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USkeletalMeshComponent *WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
		UAnimMontage* ClientArmsFireAnimMontage;//������̫��

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* ClientArmsReloadAnimMontage;//������̫��

	UPROPERTY(EditAnywhere)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;//ǹ�ڻ����Ч

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere)
		int FPArmsBlendPose = 0;
	
private:

protected:


private:

protected:
	
	virtual void BeginPlay() override;
		
public:	
	AWeaponBaseClient();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent,Category= "FPSAnimation")
		void PlayShootAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSAnimation")
	void PlayReloadAnimation();

	void DisplayWeaponEffect();

	
};
