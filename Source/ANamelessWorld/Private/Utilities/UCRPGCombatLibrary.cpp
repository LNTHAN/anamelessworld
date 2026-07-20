// UCRPGCombatLibrary.cpp
// PURPOSE: Implements the d20 dice roll functions.
//          All functions are static — no instance needed, call directly from anywhere.

#include "Utilities/UCRPGCombatLibrary.h"
#include "Characters/ABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"


int32 UCRPGCombatLibrary::RollD20()
{
    // FMath::RandRange is inclusive on both ends — RandRange(1, 20) returns 1 to 20.
    return FMath::RandRange(1, 20);
}

int32 UCRPGCombatLibrary::RollWithAdvantage()
{
    // Roll twice, take the higher result.
    // Advantage shifts the probability curve in the roller's favour —
    // the chance of rolling 15+ goes from 30% to 51% with Advantage.
    const int32 Roll1 = RollD20();
    const int32 Roll2 = RollD20();
    return FMath::Max(Roll1, Roll2);
}

int32 UCRPGCombatLibrary::RollWithDisadvantage()
{
    // Roll twice, take the lower result.
    // Disadvantage mirrors Advantage — same code, different pick function.
    // An Enraged enemy is off-balance: likely to swing wild and miss.
    const int32 Roll1 = RollD20();
    const int32 Roll2 = RollD20();
    return FMath::Min(Roll1, Roll2);
}

int32 UCRPGCombatLibrary::GetModifier(float StatScore)
{
    // D&D 5e formula: floor((Score - 10) / 2)
    // FMath::FloorToInt rounds toward negative infinity — correct for negative values.
    // Example: Score 8 → (8 - 10) / 2 = -1.0 → floor = -1 (not 0).
    return FMath::FloorToInt((StatScore - 10.0f) / 2.0f);
}

int32 UCRPGCombatLibrary::CalculateDC(int32 Modifier)
{
    // Difficulty Class: the number a target must meet or beat to resist an effect.
    // Base 8 is the D&D 5e standard — even a character with +0 modifier
    // has a DC of 8, meaning a target needs to roll 8+ to resist.
    return 8 + Modifier;
}

int32 UCRPGCombatLibrary::CalculateAC(float DexScore)
{
    // Armor Class: the number an attacker must meet or beat to land a hit.
    // Base 10 + DEX modifier. Equipment bonuses added on top later.
    return 10 + GetModifier(DexScore);
}

float UCRPGCombatLibrary::CalculateAttackDamage(const ABaseCharacter* Attacker, bool bHeavy)
{
    if (!Attacker) return 0.f;
    const float Base = bHeavy ? Attacker->HeavyDamage : Attacker->AttackDamage;
    return FMath::Max(1.f, Base + GetModifier(Attacker->GetStrength()));
}

int32 UCRPGCombatLibrary::CalculateHitChance(const ABaseCharacter* Attacker, const ABaseCharacter* Target)
{
    if (!Attacker || !Target) return 0;

    const int32 STRMod   = GetModifier(Attacker->GetStrength());
    const int32 TargetAC = CalculateAC(Target->GetDexterity());

    // Hit needs d20 + STRMod >= AC, i.e. d20 >= AC - STRMod.
    const int32 Need = TargetAC - STRMod;

    // Clamped to 5%/95% — the 5e convention that a nat 1 always misses and a
    // nat 20 always hits.
    float P = FMath::Clamp((21.f - Need) / 20.f, 0.05f, 0.95f);

    // Mirrors GA_BasicAttack: Advantage = 2d20 keep-highest, Disadvantage =
    // keep-lowest, and having both cancels out.
    const UAbilitySystemComponent* ASC = Attacker->GetAbilitySystemComponent();
    const bool bAdv = ASC && ASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Advantage")));
    const bool bDis = ASC && ASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Confused")));

    if (bAdv && !bDis)      P = 1.f - (1.f - P) * (1.f - P);
    else if (bDis && !bAdv) P = P * P;

    return FMath::RoundToInt(P * 100.f);
}