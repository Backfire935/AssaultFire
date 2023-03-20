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
		UAnimMontage* ClientArmsFireAnimMontage;

	UPROPERTY(EditAnywhere)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCameraShakeBase> CameraShakeClass;
	
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


	void DisplayWeaponEffect();

	
};
