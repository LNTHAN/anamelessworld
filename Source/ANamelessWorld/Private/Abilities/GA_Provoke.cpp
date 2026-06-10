// GA_Provoke.cpp
// PURPOSE: Applies State.Enraged to a target enemy, flagging them to attack
//          the protagonist on their next turn with Disadvantage. The actual
//          redirect logic and self-damage on miss are wired in Session 13.

#include "Abilities/GA_Provoke.h"
#include "AbilitySystemComponent.h"
#include "Characters/ABaseCharacter.h"


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

UGA_Provoke::UGA_Provoke()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// ════════════════════════════════════════════════════════════════════════════
// ACTIVATE ABILITY
// ════════════════════════════════════════════════════════════════════════════

void UGA_Provoke::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // ── Step 1: Commit ─────────────────────────────────────────────────────
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Provoke: CommitAbility failed — not enough Mana or on cooldown."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 2: Find the target enemy ─────────────────────────────────────
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Provoke: No target in TriggerEventData."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const ABaseCharacter* TargetCharacter =
        Cast<ABaseCharacter>(TriggerEventData->Target);

    if (!TargetCharacter || !TargetCharacter->IsAlive())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Provoke: Target is not a living ABaseCharacter."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 3: Apply State.Enraged to the target's ASC ───────────────────
    // Enraged is applied to the ENEMY's ASC. UTurnManager checks for this
    // tag on their turn and forces them to target the protagonist instead
    // of their normal target. Redirect logic added in Session 13.
    UAbilitySystemComponent* TargetASC =
        TargetCharacter->GetAbilitySystemComponent();

    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Provoke: Target has no AbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (ProvokeEffectClass)
    {
        FGameplayEffectContextHandle ContextHandle =
            ActorInfo->AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle =
            ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                ProvokeEffectClass, GetAbilityLevel(), ContextHandle);

        if (SpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            const FString TargetName = TargetCharacter->GetName();
            UE_LOG(LogTemp, Log,
                TEXT("GA_Provoke: %s is now Enraged — will attack protagonist with Disadvantage."),
                *TargetName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Provoke: ProvokeEffectClass is null. Assign GE_Provoke in the Blueprint Class Defaults."));
    }

    // ── Step 4: End cleanly ────────────────────────────────────────────────
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}