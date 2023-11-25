// Richard Hill

#pragma once

#define TEST_BIT(Bitmask, Bit) (((Bitmask) & (1 << static_cast<uint8>(Bit))) > 0)
#define TEST_BITS(Bitmask, Bit) (((Bitmask) & Bit) == Bit)
#define SET_BIT(Bitmask, Bit) (Bitmask |= static_cast<uint8>(Bit))
#define CLEAR_BIT(Bitmask, Bit) (Bitmask &= ~(1 << static_cast<uint8>(Bit)))

#include "CoreMinimal.h"
#include "Constants.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RelativeContext.h"
#include "ContextHelper.generated.h"

UCLASS()
class UContextHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	// UFUNCTION(BlueprintCallable, Category= Context)
	static uint8 GetRelativeContext(const AActor* FromActor, const AActor* TargetActor)
	{
		uint8 ReturnContext = 0;

		const FVector FromLocation = FromActor->GetActorLocation();
		const FVector TargetLocation = TargetActor->GetActorLocation();
		const FVector Direction = TargetLocation - FromLocation;

		const auto LocalSpaceLocation = FromActor->GetActorTransform().InverseTransformPosition(TargetLocation);

		// Check where the target actor is in relation to FromActor
		if (LocalSpaceLocation.X > 0)
			SET_BIT(ReturnContext, ERelativeContext::InFront);
		else if(LocalSpaceLocation.X < 0)
			SET_BIT(ReturnContext, ERelativeContext::Behind);
		
		// if(LocalSpaceLocation.Y > 0)
		// 	SET_BIT(ReturnContext, ERelativeContext::ToTheRight);
		// else if(LocalSpaceLocation.Y < 0)
		// 	SET_BIT(ReturnContext, ERelativeContext::ToTheLeft);
		//
		// if(LocalSpaceLocation.Z > 0)
		// 	SET_BIT(ReturnContext, ERelativeContext::Above);
		// else if(LocalSpaceLocation.Z < 0)
		// 	SET_BIT(ReturnContext, ERelativeContext::Below);
		//
		// // Checks if the target is 400 units away from FromActor
		// const auto Distance = Direction.Length();
		// if (Distance > CONST_RANGE)
		// 	SET_BIT(ReturnContext, ERelativeContext::Far);
		// else
		// 	SET_BIT(ReturnContext, ERelativeContext::Close);
		//
		// // Check if actor is in front by 45 degree angle
		// const float Angle = FindAngle(FromActor->GetActorForwardVector(), Direction.GetSafeNormal());
		// const auto HalfGlobalAngle = CONST_ANGLE * 0.5f;
		// if (Angle < HalfGlobalAngle && Angle > -HalfGlobalAngle)
		// 	SET_BIT(ReturnContext, ERelativeContext::Seen);
		// else
		// 	SET_BIT(ReturnContext, ERelativeContext::Unseen);

		// Check if actors are facing eachother or not
		const auto FacingDot = FVector::DotProduct(FromActor->GetActorForwardVector(), TargetActor->GetActorForwardVector());
		if (FacingDot > CONST_DIRECTION_THRESHOLD)
		{
			SET_BIT(ReturnContext, ERelativeContext::FacingSame);
		}
			
		else if (FacingDot < -CONST_DIRECTION_THRESHOLD)
		{
			SET_BIT(ReturnContext, ERelativeContext::FacingOpposite);
		}

		return ReturnContext;
	}

	UFUNCTION(BlueprintCallable, Category = "Context")
	static float FindAngle(const FVector SelfForward, const FVector TargetForward)
	{
		auto Dot = FVector::DotProduct(SelfForward, TargetForward);
		Dot = FMath::Clamp(Dot, -1.f, 1.f);
		return FMath::RadiansToDegrees(FMath::Acos(Dot));
	}

	// UFUNCTION(BlueprintCallable, Category = "Context")
	static bool CheckContext(const uint8 Test, const uint8 Value)
	{
		return TEST_BITS(Test, Value);
	}
};
