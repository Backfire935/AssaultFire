
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
	//开启重力
	WeaponMesh->SetEnableGravity(true);
	//开启模拟物理
	WeaponMesh->SetSimulatePhysics(true);
	//绑定委托，触碰拾取


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
	//看看能不能转化成功
	AFPSBaseCharacter * FPSCharacter  = Cast<AFPSBaseCharacter>(OtherActor);
	if(FPSCharacter)
	{
		if(FPSCharacter->GetServerPrimaryWeapon())
		{
			//如果地上的枪有碰撞就把碰撞关了
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//玩家身上已经有武器了，就不拾取了
			return;
		}
		EquipWeapon();

		if(KindOfWeapon == EWeaponType::DesertEagle)
		{
			FPSCharacter->EquipSecondaryWeapon(this);
		}
		else
		{
			//装备武器
			FPSCharacter->EquipPrimaryWeapon(this);
		}
	}

}


//拾取后设置自己相关的属性值
void AWeaponBaseServer::EquipWeapon()
{
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	//只要发出多播的玩家不等于目前控制的玩家 就执行，相当于不在发出多播的玩家上执行
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		DisplayWeaponEffect();
	}
}



void AWeaponBaseServer::DisplayWeaponEffect()
{
	//生成枪口粒子闪光
	if (MuzzleFlash)
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Fire_FX_Slot"), FVector(0, 0, 0), FRotator::ZeroRotator, FVector(1, 1, 1), EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::AutoRelease, true);

	if(FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation(),0.4f);
	}
}

void AWeaponBaseServer::DropWeapon()
{

		//告知第一人称视角也执行丢弃武器的逻辑

		//第三人称模型分离
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
		WeaponMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
		//开启模型物理模拟和重力
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetSimulatePhysics(true);


		SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		//设置模型可见性为所有人都可见
		WeaponMesh->SetOwnerNoSee(false);
		WeaponMesh->SetOnlyOwnerSee(false);
		//所属设为空
		SetOwner(nullptr);
		//主武器指针设为空
	

}


void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();

	
}

void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

