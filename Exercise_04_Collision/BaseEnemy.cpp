// Richard Hill


#include "BaseEnemy.h"

#include <string>

#include "AIState.h"
#include "GrimAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SchoolTesting/Interfaces/AbilityInterface.h"
#include "SchoolTesting/Player/CustomComponents/Magic/BaseSpell.h"
#include "SchoolTesting/Player/CustomComponents/HealthComponent.h"
#include "SchoolTesting/Player/CustomComponents/Magic/ManaComponent.h"
#include "SchoolTesting/Player/CustomComponents/Stats/StatsComponent.h"
#include "SchoolTesting/Subsystems/IntersectionSubsystem.h"


class UIntersectionSubsystem;

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	ManaComponent = CreateDefaultSubobject<UManaComponent>(TEXT("Mana Component"));
	StatsComponent = CreateDefaultSubobject<UStatsComponent>("Stats Component");
}


float ABaseEnemy::GetAttackTime()
{
	return AttackTime;
}

void ABaseEnemy::SetDeathMaterialFade(float DeltaTime)
{	
	if (!bPlayDeathMaterialFade) return;
	
	float const CurrentAlpha = DynamicMaterial->K2_GetScalarParameterValue(TEXT("AnimAlpha"));

	if (bPlayReviveMaterialFade)
	{
		float const ReviveFadeAlpha = FMath::FInterpConstantTo(CurrentAlpha, 0, DeltaTime, DeathMaterialFadeInterpSpeed);
		DynamicMaterial->SetScalarParameterValue(TEXT("AnimAlpha"), ReviveFadeAlpha);
		
		if (ReviveFadeAlpha <= 0)
		{
			bPlayReviveMaterialFade = false;
			bPlayDeathMaterialFade = false;
		}
	}
	else
	{
		float const FadeAlpha = FMath::FInterpConstantTo(CurrentAlpha, 1, DeltaTime, DeathMaterialFadeInterpSpeed);
		DynamicMaterial->SetScalarParameterValue(TEXT("AnimAlpha"), FadeAlpha);
		if (FadeAlpha >= 1)
		{
			SetActorHiddenInGame(true);
			SetActorTickEnabled(false);
			FTimerHandle Timer;
			GetWorldTimerManager().SetTimer(Timer, this, &ABaseEnemy::SpawnEnemy, RespawnTime);
		}
	}
}

void ABaseEnemy::PerformAttack()
{
	if (!CurrentSpell) return;
	
	if (!ManaComponent->CheckMana(CurrentSpell->ManaCost)) return;
		
	CurrentSpell->Activate();
}

void ABaseEnemy::OnHit_Internal(float Damage, bool IsCrit, bool bIsBackStab, AActor* DamageInstigator)
{
	if (HealthComponent->GetIsDead()) return;
	if (!bHasAggro)
	{
		SetAggro_Implementation(true);
	}

	if (DamageInstigator->GetClass()->ImplementsInterface(UAbilityInterface::StaticClass()))
	{
		// Push back if the damageinstigator has interface and is a pushback actor (bool)
		if (IAbilityInterface::Execute_GetIsPushBackActor(DamageInstigator))
		{
			float const PushbackValue = IAbilityInterface::Execute_GetPushBackValue(DamageInstigator);
			FVector const PushbackDirection = DamageInstigator->GetActorForwardVector();
			Pushback(PushbackValue, PushbackDirection);
			
		}
	}
	
	// Play Random Hit Animation Montage
	if (!HitMontage) return;
	int const Sections = HitMontage->GetNumSections();
	int const Random = FMath::RandRange(0, Sections - 1);
	FName const StartSectionName = HitMontage->GetSectionName(Random);
	PlayAnimationMontage(HitMontage, 1, StartSectionName);
}

void ABaseEnemy::TriggerDeath()
{
	AIController->UnPossess();
	SetAggro_Implementation(false);
	OnDeAggro.Broadcast();
	UBlackboardComponent* BBComponent = AIController->GetBlackboardComponent();
	// BBComponent->SetValueAsBool(FName(TEXT("HasAggro")), bHasAggro);
	SetActorEnableCollision(false);
	PlayAnimationMontage(DeathMontage);
}

void ABaseEnemy::StartDeathMaterialFade()
{
	bPlayDeathMaterialFade = true;
}

// void ABaseEnemy::AggroTarget()
// {
// 	SetAggro_Implementation(true);
// 	// UBlackboardComponent* BBComponent = AIController->GetBlackboardComponent();
// 	// BBComponent->SetValueAsBool(FName(TEXT("HasAggro")), bHasAggro);
// }

void ABaseEnemy::SetAggro_Implementation(bool NewAggro)
{
	Execute_SetAggro(this, NewAggro);
	bHasAggro = NewAggro;
	UBlackboardComponent* BBComponent = AIController->GetBlackboardComponent();
	BBComponent->SetValueAsBool(FName(TEXT("HasAggro")), bHasAggro);
	NewAggro ? OnAggro.Broadcast() : OnDeAggro.Broadcast(); // Broadcast OnAggro event if aggro
}

bool ABaseEnemy::GetAggroStatus_Implementation()
{
	return bHasAggro;
}

void ABaseEnemy::DisableEnemy()
{
	bPlayDeathMaterialFade = true;
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	SetActorEnableCollision(false);
}

void ABaseEnemy::Pushback(float PushbackValue, FVector Direction)
{
	// Bareboned pushback implementation
	// TODO: Extend and make the pushback prettier
	AddActorWorldOffset(Direction *= PushbackValue);
}

void ABaseEnemy::RotateTowardPlayer()
{
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHit.AddDynamic(this, &ABaseEnemy::OnHit_Internal);
	SpawnLocation = GetActorLocation();

	// Set Dynamic Material for Death Material Fade
	UMaterialInterface* MaterialInstance = GetMesh()->GetMaterial(0);
	DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialInstance, this);
	GetMesh()->SetMaterial(0, DynamicMaterial);

	AIController = Cast<AGrimAIController>(GetController());
	if (AIController)
	{
		AIController->SetBehaviorTree(BehaviorTree);
		AIController->GetBlackboard()->SetValueAsVector("SpawnPoint", SpawnLocation);
	}
	
	if (Spell != nullptr)
	{
		CurrentSpell = GetWorld()->SpawnActor<ABaseSpell>(Spell);
		CurrentSpell->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		CurrentSpell->SetSpellOwner(this);
	}

	UIntersectionSubsystem* SubSystem = GetWorld()->GetSubsystem<UIntersectionSubsystem>();
	SubSystem->RegisterEnemy(this);
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetDeathMaterialFade(DeltaTime);
}

float ABaseEnemy::PlayAnimationMontage(UAnimMontage* Montage, float InPlayRate, FName StartSectionName, float StartMontageAt)
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	UAnimInstance* AnimInstance = (SkeletalMesh)? SkeletalMesh->GetAnimInstance() : nullptr; 
	if ( Montage && AnimInstance )
	{
		float Duration = AnimInstance->Montage_Play(Montage, InPlayRate, EMontagePlayReturnType::MontageLength, StartMontageAt);
		if (Duration > 0.f)
		{
			// Start at a given Section.
			if( StartSectionName != NAME_None )
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
				int const MontageSectionID = Montage->GetSectionIndex(StartSectionName);
				Duration = Montage->GetSectionLength(MontageSectionID);
			}
			return Duration;
		}
	}
	return 0;
}

void ABaseEnemy::Destroyed()
{
	Super::Destroyed();

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	if (AttachedActors.Num() > 0)
	{
		for (AActor* AttachedActor : AttachedActors)
		{
			AttachedActor->Destroy();
		}
	}
}

void ABaseEnemy::SpawnEnemy()
{
	SetAggro_Implementation(false);
	SetActorLocation(SpawnLocation);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	SetActorEnableCollision(true);
	
	HealthComponent->GainHealth(HealthComponent->MaxHealth);
	HealthComponent->SetIsDead(false);
	
	bPlayReviveMaterialFade = true;
	bPlayDeathMaterialFade = true;
	
	

	AIController->Possess(this);
	AIController->RunBehaviorTree(BehaviorTree);
	AIController->SetFocus(nullptr);

	UBlackboardComponent* BBComponent = AIController->GetBlackboardComponent();
	BBComponent->SetValueAsEnum(FName(TEXT("EnemyState")), (uint8)EAIState::Patrol);
}
