// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"

void AFPSPlayerController::PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake)
{
	// 'APlayerController::ClientPlayCameraShake': Please use ClientStartCameraShake  otherwise your project will no longer compile.
	ClientStartCameraShake(CameraShake, 1.f, ECameraShakePlaySpace::CameraLocal,  FRotator::ZeroRotator);
}


