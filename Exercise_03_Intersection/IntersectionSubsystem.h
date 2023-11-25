// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SchoolTesting/Enemy/BaseEnemy.h"
#include "IntersectionSubsystem.generated.h"

class AMagicPlayerController;
UCLASS()
class UIntersectionSubsystem
	: public UTickableWorldSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<ABaseEnemy*> RegisteredEnemies;
	UPROPERTY()
	TArray<ABaseEnemy*> SearchedEnemies;

	UPROPERTY()
	AMagicPlayerController* PlayerControllerInstance;

	// UPROPERTY()
	// ANoiseTerrain* NoiseTerrain;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;

	void RegisterPlayerController(AMagicPlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	ABaseEnemy* TabTargetSearch(ABaseEnemy* PrevTarget);

	void RegisterEnemy(ABaseEnemy* Enemy);
	void UnregisterEnemy(ABaseEnemy* Enemy);

	virtual TStatId GetStatId() const override;
};