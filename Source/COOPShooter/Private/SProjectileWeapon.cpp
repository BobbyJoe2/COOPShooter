// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

void ASProjectileWeapon::Fire()
{
	//trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();

	if (MyOwner && ProjectileClass) {
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);

		APawn* ProjectileOwner = Cast<APawn>(GetOwner());
		if (ProjectileOwner) {
			APlayerController* PC = Cast<APlayerController>(ProjectileOwner->GetController());
			if (PC) {
				PC->ClientPlayCameraShake(FireCamShake);
			}
		}

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation, 1.0f, 1.0f, 0.0f);
	}

}
