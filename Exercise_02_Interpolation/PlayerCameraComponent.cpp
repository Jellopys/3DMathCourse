// Richard Hill


#include "PlayerCameraComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SchoolTesting/Player/PlayerCharacter.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCameraComponent::EnableCameraRotation(bool Enabled)
{
	bRotatingCamera = Enabled;
	
	if (Enabled && PlayerCharacter->GetIsWidgetOpen())
	{
		PlayerCharacter->ShowMouseCursor(false);

		
		// FInputModeGameOnly InputMode;
		// InputMode.SetConsumeCaptureMouseDown(false);
		// PlayerCharacter->PlayerController->SetInputMode(InputMode);
	}
	else if (!Enabled && PlayerCharacter->GetIsWidgetOpen())
	{
		PlayerCharacter->ShowMouseCursor(true);
	}
}

USpringArmComponent* UPlayerCameraComponent::GetSpringArm() const
{
	return SpringArm;
}

UCameraComponent* UPlayerCameraComponent::GetCamera() const
{
	return Camera;
}

void UPlayerCameraComponent::ScrollCamera(float Axis)
{
	if (bHoldingShift)
	{
		SetCameraFoV(Axis);
	}
	else
	{
		SetCameraDistance(Axis);
	}
}

void UPlayerCameraComponent::SetCameraDistance(float Axis)
{
	if (!bCanControlCamera) return;
	NewArmLength = FMath::Clamp(NewArmLength -= Axis * DistanceScrollSpeed, MinArmLength, MaxArmLength);
}

void UPlayerCameraComponent::SetCameraFoV(float Axis)
{
	if (!bCanControlCamera) return;
	NewFoV = FMath::Clamp(NewFoV -= Axis * FoVScrollSpeed, MinFoV, MaxFoV);
}

void UPlayerCameraComponent::EnableFoVShift(bool Enable)
{
	bHoldingShift = Enable;
}

void UPlayerCameraComponent::SetCanMoveCamera(bool CanMoveCamera)
{
	bCanControlCamera = CanMoveCamera;

	if (CanMoveCamera)
	{
		// Return camera to previous position
		bReturningCameraToPrevPos = true;
	}
}

void UPlayerCameraComponent::ForceCameraToLocation(FRotator Rotation, float ArmLength, float FoV)
{
	// Cache current Camera Settings
	CachedArmLength = NewArmLength;
	CachedFoV  = NewFoV;
	CachedTargetRotation = SpringArm->GetRelativeRotation();

	// Set New camera settings
	NewArmLength = ArmLength;
	NewFoV = FoV;
	TargetRotation = Rotation;
}

void UPlayerCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	SpringArm = NewObject<USpringArmComponent>(this);
	SpringArm->SetupAttachment(GetOwner()->GetRootComponent());
	SpringArm->RegisterComponent();

	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 15;
	SpringArm->TargetArmLength = 3000;
	SpringArm->bInheritYaw = false;
	SpringArm->bDoCollisionTest = false;
	SpringArm->ProbeChannel = ECC_Camera;
	
	Camera = NewObject<UCameraComponent>(this);
	Camera->SetupAttachment(SpringArm);
	Camera->RegisterComponent();

	Camera->FieldOfView = NewFoV;
	
	const FRotator StartRotation = FRotator(StartPitch, StartYaw, 0);
	SpringArm->SetRelativeRotation(StartRotation);
}

void UPlayerCameraComponent::UpdateCameraDistance(float DeltaTime)
{
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, NewArmLength, DeltaTime, CameraDistanceInterpSpeed);
}

void UPlayerCameraComponent::UpdateCameraFoV(float DeltaTime)
{
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, NewFoV, DeltaTime, FoVInterpSpeed);
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	RotateCamera(DeltaTime);
	UpdateCameraDistance(DeltaTime);
	UpdateCameraFoV(DeltaTime);
	ForceMoveCameraOnTick(DeltaTime);
}

void UPlayerCameraComponent::ForceMoveCameraOnTick(float DeltaTime)
{
	if (bReturningCameraToPrevPos) 
	{
		if (bRotatingCamera)
		{
			bReturningCameraToPrevPos = false;
			// End the returnal early if the player decides to control the camera manually
		}
		
		const FRotator Rotation = FMath::RInterpTo(SpringArm->GetRelativeRotation(), TargetRotation, DeltaTime, ForceMoveInterpStep);
		SpringArm->SetRelativeRotation(Rotation);
		
		if (SpringArm->GetRelativeRotation().Equals(TargetRotation, 10))
		{
			bReturningCameraToPrevPos = false;
		}
		return;
	}
	
	if (!bCanControlCamera)
	{
		const FRotator Rotation = FMath::RInterpTo(SpringArm->GetRelativeRotation(), TargetRotation, DeltaTime, ForceMoveInterpStep);
		SpringArm->SetRelativeRotation(Rotation);
	}
}


void UPlayerCameraComponent::RotateCamera(float DeltaTime)
{
	if (bCanControlCamera)
	{
		if (bReturningCameraToPrevPos) return;
		if (!bRotatingCamera) return;
		
		FVector2D MousePositionDelta = FVector2D(0, 0);
		GetWorld()->GetFirstPlayerController()->GetInputMouseDelta(MousePositionDelta.X, MousePositionDelta.Y);
		float Pitch = MousePositionDelta.Y * CameraRotationSpeed;

		if (SpringArm->GetRelativeRotation().Pitch < MaxCameraPitchRotation && Pitch < 0)
		{
			Pitch = 0;
		}
		else if (SpringArm->GetRelativeRotation().Pitch > MinCameraPitchRotation && Pitch > 0)
		{
			Pitch = 0;
		}

		TargetRotation = FRotator(Pitch, MousePositionDelta.X * CameraRotationSpeed, 0);
		
		const FRotator Rotation = FMath::RInterpConstantTo(FRotator::ZeroRotator, TargetRotation, DeltaTime, 0);
	
		SpringArm->AddRelativeRotation(Rotation);
	}
}

