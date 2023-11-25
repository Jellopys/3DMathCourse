// Richard Hill

#pragma once

#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// class UGrimGameInstance;
// class UPlayerStatsObject;
// class AGrimGameMode;
class UHealthWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTakeDamageDelegate, float, Damage, bool, bIsCrit, bool, bIsBackStab);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(meta=(BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float CurrentHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float BaseMaxHealth = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float MaxHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float ImmunityTime = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float HealthGainPerSec = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float VitalityHealthMultiplier = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	bool bIsPlayer = false;
	
	// void TakeDamageInternal(float IncomingHealth);
	UFUNCTION(BlueprintCallable)
	void TakeDamage(float IncomingHealth, bool IsCritHit, AActor* Instigator);

	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnTakeDamageDelegate OnHit;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnDeath OnDeath;
	
	UFUNCTION(BlueprintCallable)
	void GainHealth(int IncomingHealth);

	UFUNCTION()
	bool GetIsImmune() const;

	UFUNCTION(BlueprintCallable)
	void Death();

	UFUNCTION()
	void DashImmunity(bool EnableImmunity);

	UFUNCTION()
	void IncreaseVitality(float Vitality);
	UFUNCTION()
	bool GetIsDead();
	UFUNCTION(BlueprintCallable)
	void SetIsDead(bool InBool);
	

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	AActor* Owner;

	UPROPERTY()
	float ImmunityCountDown;
	UPROPERTY(BlueprintReadOnly)
	bool bIsImmune;
	UPROPERTY(BlueprintReadOnly)
	bool bIsDead;

	UFUNCTION()
	void HandleImmunityTimer(float DeltaTime);

	void GainHealthPerSec(float DeltaTime);
};
