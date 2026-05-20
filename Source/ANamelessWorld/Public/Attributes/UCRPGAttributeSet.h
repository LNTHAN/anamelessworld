// UCRPGAttributeSet.h
// PROJECT: A Nameless World
// PURPOSE: Declares all stat attributes for every character.
//          GAS reads, modifies, and replicates these values during gameplay.
//          This is the "stat sheet" — Health, Mana, the six D&D core stats, XP, Level.
//
// CODING ORDER NOTE:
//   This is built FIRST because ABaseCharacter #includes it.
//   If this file does not exist, ABaseCharacter cannot compile.

#pragma once
// Tells the compiler: only include this file once per build, even if multiple
// .cpp files try to #include it. Prevents "already defined" linker errors.

#include "CoreMinimal.h"
// UE5 essential types: FString, FName, TArray, FVector, etc.
// Always the first include in any UE5 header.

#include "AttributeSet.h"
// The GAS base class we inherit from.
// Registering as a UAttributeSet tells GAS "this object holds attributes."

#include "AbilitySystemComponent.h"
// Required by the ATTRIBUTE_ACCESSORS macro below.
// The macro references UAbilitySystemComponent internally.

#include "UCRPGAttributeSet.generated.h"
// MUST be the LAST include in every UE5 header.
// Unreal Header Tool (UHT) generates this file at build time.
// It contains the reflection data that makes UPROPERTY and UFUNCTION work.
// Without it, the engine cannot track, save, or replicate this class.


// ── ATTRIBUTE_ACCESSORS Macro ────────────────────────────────────────────────
//
// This #define runs BEFORE the compiler (preprocessor stage).
// It expands one line into four auto-generated helper functions per attribute.
//
// For Health it generates:
//   GetHealth()          — returns CurrentValue (the live, in-game value)
//   SetHealth(float val) — sets CurrentValue
//   InitHealth(float val)— sets BaseValue (called once on spawn)
//   GetHealthAttribute() — returns an FGameplayAttribute handle
//                          (GAS uses this to identify the attribute in effects)
//
// Without this macro: ~20 lines of boilerplate per attribute × 12 attributes = 240 lines.
// With it: 1 line per attribute.
//
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)            \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)  \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                 \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                 \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


// ── Class Declaration ────────────────────────────────────────────────────────

UCLASS()
// Registers this class with UE5's reflection system.
// Required for the engine to spawn it, replicate it, and show it in the editor.
// Every class that uses UPROPERTY or UFUNCTION must have UCLASS().

class ANAMELESSWORLD_API UCRPGAttributeSet : public UAttributeSet
// ANAMELESSWORLD_API  — DLL export marker. Makes this class visible to other
//                       modules (e.g. GAS, the editor). Generated from project name.
// : public UAttributeSet — Inherits from GAS's base attribute class.
//                          GAS will automatically find and register this AttributeSet
//                          when it is attached to an AbilitySystemComponent.
{
    GENERATED_BODY()
    // Pastes in the auto-generated reflection code from .generated.h.
    // Must be the FIRST line inside every UCLASS body. Non-negotiable.

public:

    // ── Constructor ──────────────────────────────────────────────────────────
    UCRPGAttributeSet();
    // Sets default starting values for all attributes.
    // Called once when the character spawns.
    // Body is in the .cpp file (One Definition Rule — bodies live in .cpp).


    // ════════════════════════════════════════════════════════════════════════
    // VITAL STATS
    // These are modified constantly during gameplay.
    // ════════════════════════════════════════════════════════════════════════

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Vitals")
    // BlueprintReadOnly: Blueprints can read this value for UI display,
    //                    but cannot write it directly (GAS owns writes).
    // Category: organises this in the editor Details panel.
    FGameplayAttributeData Health;
    // FGameplayAttributeData stores TWO floats:
    //   BaseValue    — the "true" health before buffs (e.g. 100)
    //   CurrentValue — health after all active effects (e.g. 80 after a debuff)
    // GAS uses this split so when a buff expires, it snaps CurrentValue
    // back to BaseValue cleanly — no need to track "what it was before."
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Vitals")
    FGameplayAttributeData MaxHealth;
    // Stored separately so the UI can display "80 / 100" without extra math.
    // Also used in PostGameplayEffectExecute to clamp Health to this ceiling.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, MaxHealth)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Vitals")
    FGameplayAttributeData Mana;
    // Resource spent to activate abilities (GA_Fireball, GA_Heal, etc.)
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Mana)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Vitals")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, MaxMana)


    // ════════════════════════════════════════════════════════════════════════
    // D&D CORE STATS
    // Scores stored here. Modifier calculated when needed.
    // Formula: Modifier = floor((Score - 10) / 2)
    // Example: Intelligence 16  →  Modifier +3  →  Debuff DC = 8 + 3 = 11
    // ════════════════════════════════════════════════════════════════════════

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Strength;
    // STR — physical attack damage, carrying capacity (inventory weight later)
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Strength)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Dexterity;
    // DEX — turn order (Initiative roll), Armor Class (AC = 10 + DEX modifier)
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Dexterity)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Intelligence;
    // INT — protagonist's PRIMARY stat. Debuff DC, puzzle interactions.
    // Higher INT = harder for enemies to resist the protagonist's word attacks.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Intelligence)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Wisdom;
    // WIS — saving throws vs debuffs and Enrage.
    // Enemies with high WIS resist the protagonist's manipulation.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Wisdom)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Charisma;
    // CHA — buff potency (how much allies benefit from protagonist's words),
    //        dialogue checks in the world outside combat.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Charisma)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|CoreStats")
    FGameplayAttributeData Constitution;
    // CON — max health scaling (LevelUp adds CON modifier × 5 to MaxHealth),
    //        poison resistance saving throws.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, Constitution)


    // ════════════════════════════════════════════════════════════════════════
    // PROGRESSION
    // ════════════════════════════════════════════════════════════════════════

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Progression")
    FGameplayAttributeData XP;
    // Current experience points. GE_GainXP adds to this after combat.
    // When XP reaches threshold, UTurnManager (or GameMode) triggers LevelUp.
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, XP)

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Attributes|Progression")
    FGameplayAttributeData CharacterLevel;
    // Current character level (starts at 1, max TBD).
    // Used to scale ability damage: base + (Level * multiplier).
    ATTRIBUTE_ACCESSORS(UCRPGAttributeSet, CharacterLevel)


    // ════════════════════════════════════════════════════════════════════════
    // GAS OVERRIDES
    // ════════════════════════════════════════════════════════════════════════

    // Called automatically by GAS AFTER every GameplayEffect modifies an attribute.
    // We override it to do two things:
    //   1. CLAMP — keep values in valid ranges (Health never < 0 or > MaxHealth)
    //   2. TRIGGER — when Health hits 0, call Die() on the owning character
    // The body (actual code) is in UCRPGAttributeSet.cpp.
    virtual void PostGameplayEffectExecute(
        const FGameplayEffectModCallbackData& Data) override;


    // ════════════════════════════════════════════════════════════════════════
    // HELPER FUNCTIONS
    // ════════════════════════════════════════════════════════════════════════

    // Calculates the D&D 5e modifier from a raw stat score.
    // Formula: floor((Score - 10) / 2)
    // Examples: Score 10 → +0 | Score 14 → +2 | Score 8 → -1 | Score 16 → +3
    // Used by: combat roll system, ability DC calculation, Initiative rolls.
    // Body is in the .cpp — it contains actual arithmetic.
    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    // BlueprintCallable: Blueprints (e.g. UI widgets) can call this directly.
    int32 GetModifier(float StatScore) const;
    // const — this function promises not to modify any member variables.
    //         Good practice for "read-only" calculations like this.
};
