// UCRPGCombatLibrary.h
// PURPOSE: Static utility functions for the d20 dice system.
//          Centralised here so every ability and AI class uses the same
//          roll logic — no duplicated random number calls scattered around.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UCRPGCombatLibrary.generated.h"

class ABaseCharacter;

UCLASS()
class ANAMELESSWORLD_API UCRPGCombatLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Roll a single d20. Returns a value between 1 and 20 inclusive.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 RollD20();

    // Roll 2d20 and return the HIGHER result.
    // Used when a character has State.Advantage — favourable conditions.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 RollWithAdvantage();

    // Roll 2d20 and return the LOWER result.
    // Used when a character has State.Enraged — they're off-balance and reckless.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 RollWithDisadvantage();

    // Calculate the D&D 5e modifier from a raw stat score.
    // Formula: floor((Score - 10) / 2)
    // Score 10 = +0 | Score 14 = +2 | Score 8 = -1 | Score 16 = +3
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 GetModifier(float StatScore);

    // Calculate the Difficulty Class for an ability.
    // Formula: 8 + Modifier
    // This is the number the target must meet or beat to resist the effect.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 CalculateDC(int32 Modifier);

    // Calculate Armor Class from a DEX score.
    // Formula: 10 + DEX modifier
    // This is the number an attacker must meet or beat to land a hit.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dice")
    static int32 CalculateAC(float DexScore);

    // Damage this attacker deals with a normal/heavy attack: DT_Characters base +
    // STR modifier, floored at 1. SINGLE SOURCE OF TRUTH — the ability applies this
    // and the forecast displays it, so the preview can't drift from what lands.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    static float CalculateAttackDamage(const ABaseCharacter* Attacker, bool bHeavy);

    // Percent chance (0-100) that Attacker's d20 + STR beats Target's AC.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    static int32 CalculateHitChance(const ABaseCharacter* Attacker, const ABaseCharacter* Target);
};
