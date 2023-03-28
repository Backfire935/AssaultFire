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
		UAnimMontage* ClientArmsFireAnimMontage;//¿ª»ðÃÉÌ«Ææ

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* ClientArmsReloadAnimMontage;//»»µ¯ÃÉÌ«Ææ

	UPROPERTY(EditAnywhere)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;//Ç¹¿Ú»ð¹âÌØÐ§

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
