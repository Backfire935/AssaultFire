
#include "WeaponBaseClient.h"

#include "Kismet/GameplayStatics.h"

AWeaponBaseClient::AWeaponBaseClient()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	RootComponent = WeaponMesh;


}


void AWeaponBaseClient::BeginPlay()
{
	Super::BeginPlay();
	WeaponMesh->SetOnlyOwnerSee(true);
}


void AWeaponBaseClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBaseClient::DisplayWeaponEffect()
{
	//�������͵�
	if(FireSound)
	UGameplayStatics::PlaySound2D(GetWorld(), FireSound,0.4f);
	//����ǹ����������
	if(MuzzleFlash)
	UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Fire_FX_Slot"), FVector(0,0,0), FRotator::ZeroRotator, FVector(1,1,1), EAttachLocation::KeepRelativeOffset, true , EPSCPoolMethod::AutoRelease, true);
}

