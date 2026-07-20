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

#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"

#include "Characters/APlayerCharacter.h"

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


    // ── Step 4: Attack roll — d20 + STR modifier vs target AC ─────────────
    // Hit/miss is resolved before damage is applied.
    // AC = 10 + target DEX modifier. Attacker rolls d20 + STR modifier.
    // Meet or beat AC = hit. Below AC = miss.

    const UCRPGAttributeSet* AttackerAttributes = Cast<UCRPGAttributeSet>(
        ActorInfo->AbilitySystemComponent->GetAttributeSet(
            UCRPGAttributeSet::StaticClass()));

    const UCRPGAttributeSet* TargetAttributes = Cast<UCRPGAttributeSet>(
        TargetASC->GetAttributeSet(UCRPGAttributeSet::StaticClass()));

    if (!AttackerAttributes || !TargetAttributes)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_BasicAttack: Could not read AttributeSets."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Trigger attack animation on the caster.
    ABaseCharacter* Caster = Cast<ABaseCharacter>(
        GetAvatarActorFromActorInfo());
    if (Caster)
    {
        Caster->SetIsAttacking(true);
        CachedCaster = Caster;
    }

    // Check for State.Advantage (from GA_Embolden) and State.Confused (from
    // Confuse — forces the confused attacker's own hit to roll with
    // Disadvantage). D&D rule: if both are present, they cancel out.
    const bool bHasAdvantage = ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Advantage")));
    const bool bHasDisadvantage = ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Confused")));

    int32 RawRoll;
    if (bHasAdvantage && !bHasDisadvantage)
    {
        RawRoll = UCRPGCombatLibrary::RollWithAdvantage();
    }
    else if (bHasDisadvantage && !bHasAdvantage)
    {
        RawRoll = UCRPGCombatLibrary::RollWithDisadvantage();
    }
    else
    {
        RawRoll = UCRPGCombatLibrary::RollD20();
    }

    const int32 STRModifier = UCRPGCombatLibrary::GetModifier(
        AttackerAttributes->GetStrength());

    const int32 TargetAC = UCRPGCombatLibrary::CalculateAC(
        TargetAttributes->GetDexterity());

    const int32 FinalRoll = RawRoll + STRModifier;

    UE_LOG(LogTemp, Log,
        TEXT("GA_BasicAttack: %s rolled %d + %d (STR) = %d vs AC %d.%s"),
        *GetAvatarActorFromActorInfo()->GetName(),
        RawRoll, STRModifier, FinalRoll, TargetAC,
        bHasDisadvantage ? TEXT(" [Disadvantage]") : (bHasAdvantage ? TEXT(" [Advantage]") : TEXT("")));

    if (FinalRoll >= TargetAC)
    {
        // Hit — apply damage.
        if (DamageEffectClass)
        {
            FGameplayEffectContextHandle ContextHandle =
                ActorInfo->AbilitySystemComponent->MakeEffectContext();
            ContextHandle.AddSourceObject(this);

            FGameplayEffectSpecHandle SpecHandle =
                ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                    DamageEffectClass, GetAbilityLevel(), ContextHandle);

                        if (SpecHandle.IsValid())
            {
                const float FinalDamage = UCRPGCombatLibrary::CalculateAttackDamage(
                    Cast<ABaseCharacter>(GetAvatarActorFromActorInfo()), bIsHeavyAttack);

                // Negative because the GE's Health modifier ADDs this value.
                SpecHandle.Data->SetSetByCallerMagnitude(
                    FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -FinalDamage);

                TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

                UE_LOG(LogTemp, Log, TEXT("GA_BasicAttack: Hit! %s took %.0f damage."),
                    *TargetCharacter->GetName(), FinalDamage);
            }
        }
    }
    else
    {
        // Miss — no damage applied.
        UE_LOG(LogTemp, Log,
            TEXT("GA_BasicAttack: Miss! %s dodged the attack."),
            *TargetCharacter->GetName());
    }

    // Store context so FinishAttack() can call EndAbility after the timer fires.
    PendingHandle = Handle;
    PendingActorInfo = ActorInfo;
    PendingActivationInfo = ActivationInfo;

    // Delay the animation reset and EndAbility so the attack animation has
    // time to play. AttackAnimDuration is set in Blueprint Class Defaults.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AttackTimerHandle,
            this,
            &UGA_BasicAttack::FinishAttack,
            AttackAnimDuration,
            false);
    }
    else
    {
        FinishAttack();
    }
}

void UGA_BasicAttack::FinishAttack()
{
    UE_LOG(LogTemp, Log, TEXT("GA_BasicAttack::FinishAttack — fired. CachedCaster: %s"),
        CachedCaster ? *CachedCaster->GetName() : TEXT("NULL"));

    ABaseCharacter* Caster = Cast<ABaseCharacter>(GetAvatarActorFromActorInfo());
    if (Caster)
    {
        Caster->SetIsAttacking(false);
    }

    EndAbility(PendingHandle, PendingActorInfo, PendingActivationInfo, true, false);

    UWorld* World = GetWorld();
    UE_LOG(LogTemp, Log, TEXT("GA_BasicAttack::FinishAttack — GetWorld() is %s. Setting FinishTurn timer."),
        World ? TEXT("valid") : TEXT("NULL"));

    if (World)
    {
        World->GetTimerManager().SetTimer(
            PostAttackTimerHandle,
            this,
            &UGA_BasicAttack::FinishTurn,
            1.0f,
            false);
    }
    else
    {
        FinishTurn();
    }
}

void UGA_BasicAttack::FinishTurn()
{
    // EndTurn is now handled by APlayerCharacter::EndTurnNow() and
    // AEnemyCharacter::EndTurnNow() via their own timers.
    // FinishTurn() is kept as a placeholder — no action needed here.
}

