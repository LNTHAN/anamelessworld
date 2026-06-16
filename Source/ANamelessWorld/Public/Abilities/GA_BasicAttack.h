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
#include "TimerManager.h"
// UGameplayAbility — the GAS base class for all abilities.
// Provides ActivateAbility(), CommitAbility(), EndAbility(), and
// all the GAS plumbing (cost checking, cooldown, tag requirements).

#include "GA_BasicAttack.generated.h"
// Always last. UHT generates this at build time.


// ── Forward Declarations ───────────────────────────────────────────────────────
class ABaseCharacter;

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
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    // ── Attack animation duration ──────────────────────────────────────────
    // How long to wait before resetting bIsAttacking and ending the ability.
    // Set this to roughly match your attack animation clip length.
    UPROPERTY(EditDefaultsOnly, Category = "ANW|Ability")
    float AttackAnimDuration = 0.8f;

private:
    FTimerHandle AttackTimerHandle;
    FTimerHandle PostAttackTimerHandle;
    ABaseCharacter* CachedCaster = nullptr;

    // Stored per-activation context so the timer callback can call EndAbility.
    FGameplayAbilitySpecHandle PendingHandle;
    const FGameplayAbilityActorInfo* PendingActorInfo = nullptr;
    FGameplayAbilityActivationInfo PendingActivationInfo;

    UFUNCTION()
    void FinishAttack();

    void FinishTurn();
};
