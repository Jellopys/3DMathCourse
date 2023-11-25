// Fill out your copyright notice in the Description page of Project Settings.
#include "IntersectionSubsystem.h"

#include "SchoolTesting/Helpers/Constants.h"
#include "SchoolTesting/Helpers/IntersectionUtility.h"
#include "SchoolTesting/Player/MagicPlayerController.h"
#include "SchoolTesting/Player/CustomComponents/HealthComponent.h"

void UIntersectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UIntersectionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UIntersectionSubsystem::RegisterPlayerController(AMagicPlayerController* PlayerController)
{	
	PlayerControllerInstance = PlayerController;

	if (!PlayerControllerInstance)
	{
		UE_LOG(LogTemp,Warning,TEXT("Could not Register Player Controller"));
	}
}

ABaseEnemy* UIntersectionSubsystem::TabTargetSearch(ABaseEnemy* PrevTarget)
{
	if (RegisteredEnemies.Num() <= 0) return nullptr;
	if (!PlayerControllerInstance) return nullptr;

	ABaseEnemy* ClosestEnemy = nullptr;
	float ClosestScreenSpacePosition = -9999;
	
	for(const auto Enemy : RegisteredEnemies)
	{
		if (PrevTarget && Enemy == PrevTarget) continue; // Do not Target the already existing target
		if (Enemy->HealthComponent->GetIsDead()) continue; // Don't target dead enemies
		if (SearchedEnemies.Contains(Enemy)) continue; // Skip enemies in search history
		
		float const DistanceToPlayer = Enemy->GetDistanceTo(PlayerControllerInstance->GetPawn());

		if (DistanceToPlayer >= CONST_TAB_TARGET_RANGE) continue;
			
		if(UIntersectionUtility::IsOnScreen(PlayerControllerInstance,Enemy->GetActorLocation()))
		{
			float const ScreenSpacePosition = UIntersectionUtility::GetActorScreenSpacePosition(PlayerControllerInstance, Enemy->GetActorLocation());
			
			if (ClosestScreenSpacePosition < -8888 || ScreenSpacePosition < ClosestScreenSpacePosition)
			{
				ClosestScreenSpacePosition = ScreenSpacePosition;
				ClosestEnemy = Enemy;
			}
		}
	}
	
	if (ClosestEnemy)
	{
		SearchedEnemies.Add(ClosestEnemy);
		return ClosestEnemy;
	}
	if (SearchedEnemies.Num() > 0) // If you have enemies in your tab history, clear it and retry the search
	{
		SearchedEnemies.Empty();
		return TabTargetSearch(nullptr); // Find fresh search and target best
		// TabTargetSearch(nullptr); // This cancels target and search
	}
	
	return nullptr; // no enemies nearby
}

void UIntersectionSubsystem::RegisterEnemy(ABaseEnemy* Enemy)
{
	if(!RegisteredEnemies.Contains(Enemy))
		RegisteredEnemies.Add(Enemy);
}

void UIntersectionSubsystem::UnregisterEnemy(ABaseEnemy* Enemy)
{
	if(RegisteredEnemies.Contains(Enemy))
		RegisteredEnemies.Remove(Enemy);
}

TStatId UIntersectionSubsystem::GetStatId() const
{
	return GetStatID();
}