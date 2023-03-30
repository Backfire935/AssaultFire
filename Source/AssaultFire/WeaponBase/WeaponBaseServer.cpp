
#include "WeaponBaseServer.h"

#include "AssaultFire/FPSCharacter/FPSBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBaseServer::AWeaponBaseServer()
{

	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECC_WorldStatic);

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetSphereRadius(72.f);
	 
	WeaponMesh->SetOwnerNoSee(true);
	//��������
	WeaponMesh->SetEnableGravity(true);
	//����ģ������
	WeaponMesh->SetSimulatePhysics(true);
	//��ί�У�����ʰȡ


	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnSphereBeginOverlap);
	SetReplicates(true);
	
}

void AWeaponBaseServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, ClipCurrentAmmo, COND_None);
}


void AWeaponBaseServer::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//�����ܲ���ת���ɹ�
	AFPSBaseCharacter * FPSCharacter  = Cast<AFPSBaseCharacter>(OtherActor);
	if(FPSCharacter)
	{
		if(FPSCharacter->GetServerPrimaryWeapon())
		{
			//������ϵ�ǹ����ײ�Ͱ���ײ����
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//��������Ѿ��������ˣ��Ͳ�ʰȡ��
			return;
		}
		EquipWeapon();

		if(KindOfWeapon == EWeaponType::DesertEagle)
		{
			FPSCharacter->EquipSecondaryWeapon(this);
		}
		else
		{
			//װ������
			FPSCharacter->EquipPrimaryWeapon(this);
		}
	}

}


//ʰȡ�������Լ���ص�����ֵ
void AWeaponBaseServer::EquipWeapon()
{
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	//ֻҪ�����ಥ����Ҳ�����Ŀǰ���Ƶ���� ��ִ�У��൱�ڲ��ڷ����ಥ�������ִ��
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		DisplayWeaponEffect();
	}
}



void AWeaponBaseServer::DisplayWeaponEffect()
{
	//����ǹ����������
	if (MuzzleFlash)
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Fire_FX_Slot"), FVector(0, 0, 0), FRotator::ZeroRotator, FVector(1, 1, 1), EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::AutoRelease, true);

	if(FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation(),0.4f);
	}
}

void AWeaponBaseServer::DropWeapon()
{

		//��֪��һ�˳��ӽ�Ҳִ�ж����������߼�

		//�����˳�ģ�ͷ���
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
		WeaponMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
		//����ģ������ģ�������
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetSimulatePhysics(true);


		SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		//����ģ�Ϳɼ���Ϊ�����˶��ɼ�
		WeaponMesh->SetOwnerNoSee(false);
		WeaponMesh->SetOnlyOwnerSee(false);
		//������Ϊ��
		SetOwner(nullptr);
		//������ָ����Ϊ��
	

}


void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();

	
}

void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

