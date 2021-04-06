// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "HAL/IConsoleManager.h"
#include "Sound/SoundAttenuation.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "COOPShooter/COOPShooter.h"
#include "TimerManager.h"

static int32 DebugWeaponsDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponsDrawing,
	TEXT("Draw Debug Lines for Weapon"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MozzleFlashSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.f;
	HeadshotBonusDamage = 2.5f;
}

void ASWeapon::CheckReloadEnd()
{
	if (GetWorld()->TimeSeconds >= EndReloadTime) {
		MagInPlace = true;
		IsReloading = false;
	}
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateofFire;

	AmmoInBag = TotalBulletsMax;
	AmmoInMagazine = MagMax;
}

void ASWeapon::Reload()
{
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	StartReloadTime = GetWorld()->TimeSeconds;
	EndReloadTime = StartReloadTime + ReloadLength;
	if (AmmoInBag > 0 && (AmmoInBag - MagMax) >= 0 && AmmoInMagazine < MagMax) {
		AmmoInBag = AmmoInBag - (MagMax - AmmoInMagazine);
		AmmoInMagazine = MagMax;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, MuzzleLocation);
		MagInPlace = false;
		IsReloading = true;
	}
	else if (AmmoInBag > 0 && (AmmoInBag - MagMax) < 0 && AmmoInMagazine < MagMax) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, MuzzleLocation);
		AmmoInMagazine = MagMax - AmmoInBag;
		AmmoInBag = 0;
		MagInPlace = false;
		IsReloading = true;
	}
	else if (AmmoInBag <= 0) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Out of Ammo"));
	}
	else {
		
	}
}

void ASWeapon::Fire()
{
	//trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();

	if (MyOwner && AmmoInMagazine > 0 && MagInPlace == true) {
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Particle "target" paramater
		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISON_WEAPON, QueryParams)) {
			//blocking hit process damage

			AActor* HitActor = Hit.GetActor();

			float ActualDamage = BaseDamage;

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			
			if (SurfaceType == SURFACE_FLESHVULNERABLE) {
				ActualDamage *= HeadshotBonusDamage;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			
			UParticleSystem* SelectedEffect = nullptr;

			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
		}

		if (DebugWeaponsDrawing > 0) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 2.5f);
		}
		
		if (Hit.ImpactPoint != FVector(0, 0, 0)) {
			PlayFireEffect(Hit.ImpactPoint, EyeLocation);
		}
		else {
			PlayFireEffect(TracerEndPoint, EyeLocation);
		}
		
		LastFireTime = GetWorld()->TimeSeconds;

		AmmoInMagazine--;
	}

	if (AmmoInMagazine <= 0) {
		Reload();
	}
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffect(FVector TracerEndPoint, FVector EyePoint)
{
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp) {
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation, 1.0f, 1.0f, 0.0f, SoundAttenuationSettings);
}