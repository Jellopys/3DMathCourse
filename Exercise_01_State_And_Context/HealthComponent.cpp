// Richard Hill


#include "HealthComponent.h"

#include <string>

#include "Kismet/GameplayStatics.h"
#include "SchoolTesting/Enemy/BaseEnemy.h"
#include "SchoolTesting/Helpers/ContextHelper.h"
#include "SchoolTesting/Player/PlayerCharacter.h"
#include "SchoolTesting/Widgets/EnemyHealthBarWidget.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	ImmunityCountDown = ImmunityTime;
	
	CurrentHealth = MaxHealth;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HandleImmunityTimer(DeltaTime);
	GainHealthPerSec(DeltaTime);
}

void UHealthComponent::HandleImmunityTimer(float DeltaTime)
{
	if (!bIsImmune) return;
	if (bIsDead) return;
	
	ImmunityCountDown -= DeltaTime;
	if (ImmunityCountDown <= 0)
	{
		bIsImmune = false;
		ImmunityCountDown = ImmunityTime; 
	}
}

void UHealthComponent::GainHealthPerSec(float DeltaTime)
{
	if (CurrentHealth >= MaxHealth) return;
	if (bIsDead) return;

	CurrentHealth += HealthGainPerSec * DeltaTime;
}

void UHealthComponent::TakeDamage(float IncomingHealth, bool IsCritHit = false, AActor* Instigator = nullptr)
{
	if (bIsImmune) return;

	bool bIsBackStab = false;

	if (Instigator)
	{
		uint8 const RelativeContext = UContextHelper::GetRelativeContext(Owner, Instigator);
		
		if(UContextHelper::CheckContext(RelativeContext, static_cast<int8>(ERelativeContext::FacingSame)))
		{
			IncomingHealth *= CONST_BACKSTAB_MULTIPLIER;
			bIsBackStab = true;
		}
	}

	float const FinalDamage = IncomingHealth;

	if (bIsPlayer)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0, MaxHealth);
		bIsImmune = true;
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0, MaxHealth);
	}
	
	OnHit.Broadcast(FinalDamage, IsCritHit, bIsBackStab);
	
	if (CurrentHealth <= 0)
	{
		Death();
	}
}

void UHealthComponent::GainHealth(int IncomingHealth)
{
	FMath::Clamp(CurrentHealth += IncomingHealth, 0, MaxHealth);
}

bool UHealthComponent::GetIsImmune() const
{
	return bIsImmune;
}

void UHealthComponent::Death()
{
	if (bIsDead) return;
	bIsDead = true;
	
	if (!bIsPlayer) // If enemy dies
	{
		OnDeath.Broadcast();
		ABaseEnemy* EnemyRef = Cast<ABaseEnemy>(Owner);
		EnemyRef->TriggerDeath();
		float const ExpToGive = EnemyRef->StatsComponent->CharacterStats.ExpWorth;
		
		APlayerCharacter* const PlayerRef = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (!PlayerRef) return;
		PlayerRef->StatsComponent->CharacterStats.GainExp(ExpToGive);
	}
	else // if Player dies
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::DashImmunity(bool EnableImmunity)
{
	if (EnableImmunity)
	{
		ImmunityCountDown = 9999;
	}
	else
	{
		ImmunityCountDown = 0;
	}
	bIsImmune = EnableImmunity;
}


void UHealthComponent::IncreaseVitality(float Vitality)
{
	float const IncreaseAmount = Vitality * VitalityHealthMultiplier;
	MaxHealth = BaseMaxHealth + IncreaseAmount;
}

bool UHealthComponent::GetIsDead()
{
	return bIsDead;
}

void UHealthComponent::SetIsDead(bool InBool)
{
	bIsDead = InBool;
}
