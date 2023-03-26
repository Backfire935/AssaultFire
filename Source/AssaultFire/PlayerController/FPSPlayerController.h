// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ASSAULTFIRE_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//²¥·ÅÆÁÄ»¶¶¶¯
	void PlayerCameraShake( TSubclassOf<UCameraShakeBase> CameraShake);

	UFUNCTION(BlueprintImplementableEvent,Category="Player UI")
	void CreatePlayerUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void DoCrossHairRecoilAnim();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
	void UpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
	void UpdateHealthUI(float NewHealth,float _MaxHealth);

};
