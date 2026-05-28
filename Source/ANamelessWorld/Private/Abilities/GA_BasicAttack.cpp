// GA_BasicAttack.cpp
// PROJECT: A Nameless World
// PURPOSE: Implements the BasicAttack ability lifecycle.
//          Flow: CommitAbility (spend mana) → find target → apply damage → EndAbility.

#include "Abilities/GA_BasicAttack.h"
// Always include our own header first.

#include "AbilitySystemComponent.h"
// Needed for MakeEffectContext(), MakeOutgoingSpec(), ApplyGameplayEffectSpecToSelf().

#include "Characters/ABaseCharacter.h"
// Full include (not just forward declaration) because we call IsAlive() on the target.

// Note: AbilitySystemBlueprintLibrary not needed — we call GetAbilitySystemComponent()
// directly on ABaseCharacter since it implements IAbilitySystemInterface.


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

UGA_BasicAttack::UGA_BasicAttack()
{
    // InstancingPolicy controls how many copies of this ability object exist.
    //
    // InstancedPerActor — one ability object lives on the character for its whole life.
    // This is the correct choice for most abilities: the object persists, so you can
    // store state on it (e.g. "how many hits in this combo") between activations.
    //
    // The alternative is NonInstanced (one shared object for ALL characters — can't
    // store per-character state) or InstancedPerExecution (new object every activation
    // — expensive and rarely needed).
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// ════════════════════════════════════════════════════════════════════════════
// ACTIVATE ABILITY
// ════════════════════════════════════════════════════════════════════════════

void UGA_BasicAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
// This is the function GAS calls after all its internal checks pass.
// We must do exactly three things, in order:
//   1. CommitAbility()  — spend Mana / start cooldown
//   2. Apply the damage
//   3. EndAbility()     — tell GAS we are done
//
// RULE: every code path (including early returns on failure) MUST call EndAbility().
// If any path skips it, the ability is permanently "in progress" and nothing else runs.
{
    // ── Step 1: Commit (spend Mana, apply cooldown) ────────────────────────
    //
    // CommitAbility() checks: does the character have enough Mana?
    // Is the ability off cooldown? If both pass, it deducts Mana and starts
    // the cooldown timer. If either fails, it returns false.
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        // Not enough Mana (or on cooldown). Ability fizzles — tell GAS to clean up.
        //
        // EndAbility parameters:
        //   Handle, ActorInfo, ActivationInfo — pass through unchanged (GAS internals)
        //   bReplicateEndAbility = true  — tell network clients this ability ended
        //   bWasCancelled = true         — flag it as cancelled (not a clean finish)
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: CommitAbility failed (not enough Mana or on cooldown)."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
        // IMPORTANT: return after EndAbility — don't fall through to the damage code.
    }


    // ── Step 2: Find the target ────────────────────────────────────────────
    //
    // TriggerEventData is the payload passed in when the ability was activated.
    // In our turn system, UTurnManager (or the player input handler) will call
    // SendGameplayEventToActor() on the attacker, with the target stored in
    // TriggerEventData->Target. We read it out here.
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        // No target was provided — can't attack thin air.
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: No target in TriggerEventData. Did you call SendGameplayEventToActor?"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Cast from AActor* to ABaseCharacter* so we can call IsAlive().
    // Cast<T>() returns nullptr if the actor is not actually an ABaseCharacter —
    // safe to call, never crashes (unlike C-style casts).
    //
    // WHY const? TriggerEventData->Target is declared as "const AActor*" —
    // a read-only pointer. Cast preserves that const, so the result is
    // "const ABaseCharacter*". We keep it const because we only read from it
    // (IsAlive, GetName, GetAbilitySystemComponent — all const functions).
    const ABaseCharacter* TargetCharacter =
        Cast<ABaseCharacter>(TriggerEventData->Target);

    if (!TargetCharacter || !TargetCharacter->IsAlive())
    {
        // Target is not a character, or they died between selection and this frame.
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: Target is not a living ABaseCharacter. Ending ability."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    // ── Step 3: Get the target's AbilitySystemComponent ───────────────────
    //
    // GE_DamageInstant must be applied TO the target's ASC, not our own.
    // We call GetAbilitySystemComponent() directly on TargetCharacter because
    // ABaseCharacter implements IAbilitySystemInterface and exposes it as const.
    // This is cleaner than the Blueprint library helper and avoids the const issue.
    UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();

    if (!TargetASC)
    {
        // Should never happen for any ABaseCharacter, but we check defensively.
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: Target has no AbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }


    // ── Step 4: Apply GE_DamageInstant to the target ──────────────────────
    //
    // Applying a GameplayEffect is always three steps:
    //   a) Make a context  — who is doing this and why?
    //   b) Make a spec     — a configured "instance" of the effect class
    //   c) Apply the spec  — execute the math on the target's ASC
    if (DamageEffectClass)
    // Guard: only apply if a damage effect was assigned in the Blueprint.
    // If DamageEffectClass is null, the ability fires but deals no damage.
    // This would be a designer error (forgot to assign GE_DamageInstant),
    // not a code error, so we don't crash — just log and finish cleanly.
    {
        // a) Context — "who is applying this effect?"
        //    ActorInfo->AbilitySystemComponent is OUR ASC (the attacker's).
        //    We use it to build a context that says "this attacker hit this target."
        FGameplayEffectContextHandle ContextHandle =
            ActorInfo->AbilitySystemComponent->MakeEffectContext();

        ContextHandle.AddSourceObject(this);
        // AddSourceObject tags the ability itself as the source.
        // GAS logs and damage number systems use this to know "what caused this hit."

        // b) Spec — a configured instance of the effect class at our ability's level.
        //    GetAbilityLevel() returns 1 by default (we set it to 1 in InitDefaultAbilities).
        //    If DamageEffectClass scales with level (e.g. "damage = 10 + level * 5"),
        //    that scaling is defined in the GE Blueprint — we just pass the level here.
        FGameplayEffectSpecHandle SpecHandle =
            ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                DamageEffectClass,      // Which effect (GE_DamageInstant Blueprint)
                GetAbilityLevel(),      // At this level
                ContextHandle);         // With this context

        // c) Apply — execute the effect on the TARGET's ASC (not our own).
        if (SpecHandle.IsValid())
        {
            // ApplyGameplayEffectSpecToSelf applies TO the ASC it's called on.
            // We call it on TargetASC → the target takes the damage.
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            // *SpecHandle.Data.Get() — unwraps the shared pointer to get
            // the raw FGameplayEffectSpec reference that the function expects.

            // UE5.6 has strict compile-time format string checking — function calls
            // inside UE_LOG can confuse the validator. Extract to local variables first.
            const FString AttackerName = GetAvatarActorFromActorInfo()
                ? GetAvatarActorFromActorInfo()->GetName()
                : TEXT("Unknown");
            const FString TargetName = TargetCharacter->GetName();
            UE_LOG(LogTemp, Log,
                TEXT("GA_BasicAttack: %s attacked %s."),
                *AttackerName, *TargetName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: DamageEffectClass is null. Assign GE_DamageInstant in the Blueprint Class Defaults."));
    }


    // ── Step 5: End the ability (clean success path) ───────────────────────
    //
    // bReplicateEndAbility = true  — clients are notified
    // bWasCancelled = false        — this was a clean finish, not a cancel
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
