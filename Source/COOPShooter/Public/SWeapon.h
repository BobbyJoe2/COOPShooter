// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class COOPSHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	void PlayFireEffect(FVector TracerEndPoint, FVector EyePoint);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HeadshotBonusDamage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundAttenuation* SoundAttenuationSettings;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	//Number of bullets shot per minute
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateofFire = 240.0f;

	//derived from rate of fire
	float TimeBetweenShots;

	float NumberBulletsInMag;

	float BulletsInBag;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float MagMax = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float TotalBulletsMax = 60;

	bool MagInPlace = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadLength = 1;

	float StartReloadTime;
	float EndReloadTime;

public:	

	virtual void Fire();

	void StartFire();

	void StopFire();

	void Reload();

	void CheckReloadEnd();
};
