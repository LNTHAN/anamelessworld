// GA_Intimidate.cpp
// PURPOSE: Applies State.Stunned to a target enemy, causing them to lose
//          their next turn. The contested INT vs WIS roll that determines
//          success or failure is wired in Session 13.

#include "Abilities/GA_Intimidate.h"
#include "AbilitySystemComponent.h"
#include "Characters/ABaseCharacter.h"


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

UGA_Intimidate::UGA_Intimidate()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// ════════════════════════════════════════════════════════════════════════════
// ACTIVATE ABILITY
// ════════════════════════════════════════════════════════════════════════════

void UGA_Intimidate::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // ── Step 1: Commit ─────────────────────────────────────────────────────
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: CommitAbility failed — not enough Mana or on cooldown."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 2: Find the target enemy ─────────────────────────────────────
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: No target in TriggerEventData."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const ABaseCharacter* TargetCharacter =
        Cast<ABaseCharacter>(TriggerEventData->Target);

    if (!TargetCharacter || !TargetCharacter->IsAlive())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: Target is not a living ABaseCharacter."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 3: Apply State.Stunned to the target's ASC ───────────────────
    // Stunned is applied to the ENEMY's ASC. UTurnManager will check for
    // this tag at the start of each turn and skip the character if present.
    UAbilitySystemComponent* TargetASC =
        TargetCharacter->GetAbilitySystemComponent();

    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: Target has no AbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (IntimidateEffectClass)
    {
        FGameplayEffectContextHandle ContextHandle =
            ActorInfo->AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle =
            ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                IntimidateEffectClass, GetAbilityLevel(), ContextHandle);

        if (SpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            const FString TargetName = TargetCharacter->GetName();
            UE_LOG(LogTemp, Log,
                TEXT("GA_Intimidate: %s is now Stunned — loses next turn."),
                *TargetName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: IntimidateEffectClass is null. Assign GE_Intimidate in the Blueprint Class Defaults."));
    }

    // ── Step 4: End cleanly ────────────────────────────────────────────────
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}