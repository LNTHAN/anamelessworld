// CRPGCharacterRow.h
// PURPOSE: One row of the Characters DataTable — the spreadsheet schema for a
//          character's *balance numbers* (stats, movement, AI tuning, trait flags).
//          Asset references (abilities, effect classes) stay in UCRPGCharacterData;
//          this struct is the numbers you tune in a CSV without touching code.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"   // FTableRowBase lives here
#include "CRPGCharacterRow.generated.h"

USTRUCT(BlueprintType)
struct ANAMELESSWORLD_API FCRPGCharacterRow : public FTableRowBase
{
    GENERATED_BODY()

    // Display name for HUD/cards ("Nameless", "The Narrator", "The Protagonist").
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    // ── Vital pools (current Health/Mana start equal to these Max values) ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
    float MaxMana = 50.f;

    // ── The six D&D ability scores (whole numbers; applied as floats) ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Strength = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Dexterity = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Constitution = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Intelligence = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Wisdom = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    int32 Charisma = 10;

    // ── Tactical tuning ──
    // Per-turn movement budget in cm (path distance). Was a per-Blueprint property.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tactical")
    float MoveRange = 500.f;

    // Base damage for this unit's normal attack, BEFORE the STR modifier is
    // added at cast time. The old flat GE_DamageInstant value was 25 for everyone.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tactical")
    float AttackDamage = 10.f;

    // Base damage for this unit's heavy strike, same rule. Old flat value: 50.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tactical")
    float HeavyDamage = 20.f;

    // 0–100. Enemy AI: chance per turn to use Heavy Strike over Basic. Player: ignored.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tactical")
    int32 HeavyStrikeChance = 0;

    // The boss's spine (Block J): if true, status GameplayEffects refuse to apply and
    // Intimidate's displacement skips this unit. A capability flag, not an archetype —
    // any unit can carry it.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tactical")
    bool bStatusImmune = false;
};