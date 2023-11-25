// Richard Hill

#pragma once

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERelativeContext : uint8
{
	None = 0 UMETA(Hidden),
	Behind = 1 << 0,
	InFront = 1 << 1,
	FacingSame = 1 << 2,
	FacingOpposite = 1 << 3,
	Hurt = 1 << 4,
	NearDeath = 1 << 5,
	MAX = 1 << 6			UMETA(Hidden)
};

ENUM_CLASS_FLAGS(ERelativeContext)
