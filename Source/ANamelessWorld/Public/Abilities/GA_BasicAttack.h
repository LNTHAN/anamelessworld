// GA_BasicAttack.h
// PROJECT: A Nameless World
// PURPOSE: The first combat ability — a simple physical attack that deals
//          damage to one target by applying GE_DamageInstant.
//
// DEPENDENCIES: UGameplayAbility (GAS base class)
// DEPENDENTS:   Blueprint subclass BP_GA_BasicAttack will inherit from this
//               and assign GE_DamageInstant to DamageEffectClass.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
// UGameplayAbility — the GAS base class for all abilities.
// Provides ActivateAbility(), CommitAbility(), EndAbility(), and
// all the GAS plumbing (cost checking, cooldown, tag requirements).

#include "GA_BasicAttack.generated.h"
// Always last. UHT generates this at build time.


// ── Class Declaration ──────────────────────────────────────────────────────────

UCLASS()
class ANAMELESSWORLD_API UGA_BasicAttack : public UGameplayAbility
// UGameplayAbility — gives us the full ability lifecycle for free.
// We only need to override ActivateAbility() with our specific logic.
{
    GENERATED_BODY()

public:

    // ── Constructor ────────────────────────────────────────────────────────
    UGA_BasicAttack();
    // Sets the instancing policy.
    // Body is in GA_BasicAttack.cpp.


    // ── Ability Lifecycle Override ─────────────────────────────────────────
    // This is the ONE function we override. GAS calls it automatically
    // after TryActivateAbility() passes all its internal checks.
    //
    // Parameters GAS passes in (we don't choose these — it's the fixed signature):
    //   Handle            — a unique ID for this specific activation instance
    //   ActorInfo         — info about who owns this ability (their ASC, avatar actor, etc.)
    //   ActivationInfo    — network info (server-confirmed, predicted, etc.)
    //   TriggerEventData  — optional payload; we use this to carry the TARGET actor
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;


protected:

    // ── Damage Effect ──────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    // EditDefaultsOnly: designers assign GE_DamageInstant here in the
    //                   Blueprint Class Defaults — no recompile needed to
    //                   swap in a different damage effect.
    TSubclassOf<UGameplayEffect> DamageEffectClass;
    // TSubclassOf<UGameplayEffect> — a pointer to a CLASS, not an instance.
    // Same pattern as DefaultAttributeEffect on ABaseCharacter.
    // At runtime, we create a spec from this class and apply it to the target.
    //
    // WHY a variable instead of hardcoding?
    //   GA_BasicAttack, GA_HeavyStrike, GA_PoisonStrike could all use this
    //   same C++ class — just with different DamageEffectClass values
    //   assigned in their respective Blueprints. One C++ class, many variants.
};
