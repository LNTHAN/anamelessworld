// GA_Embolden.cpp
// PURPOSE: Applies the State.Advantage tag to a target ally via a GameplayEffect.
//          The protagonist picks an ally, this fires, and that ally gets boosted
//          on their next action. CHA modifier bonus wired in Session 13.

#include "Abilities/GA_Embolden.h"
#include "AbilitySystemComponent.h"
#include "Characters/ABaseCharacter.h"


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

UGA_Embolden::UGA_Embolden()
{
    // One ability object per character, persists for their whole life.
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// ════════════════════════════════════════════════════════════════════════════
// ACTIVATE ABILITY
// ════════════════════════════════════════════════════════════════════════════

void UGA_Embolden::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // ── Step 1: Commit (spend Mana, apply cooldown) ────────────────────────
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Embolden: CommitAbility failed — not enough Mana or on cooldown."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 2: Find the target ally ──────────────────────────────────────
    // Target is passed in via SendGameplayEventToActor, same pattern as BasicAttack.
    // For Embolden the target is an ally, but the code is identical — we just
    // read whoever was stored in TriggerEventData->Target.
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Embolden: No target in TriggerEventData."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const ABaseCharacter* TargetCharacter =
        Cast<ABaseCharacter>(TriggerEventData->Target);

    if (!TargetCharacter || !TargetCharacter->IsAlive())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Embolden: Target is not a living ABaseCharacter."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── Step 3: Apply the Embolden effect to the target's ASC ─────────────
    // We apply to the TARGET's ASC (the ally being buffed), not our own.
    UAbilitySystemComponent* TargetASC =
        TargetCharacter->GetAbilitySystemComponent();

    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Embolden: Target has no AbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    ABaseCharacter* Caster = Cast<ABaseCharacter>(GetAvatarActorFromActorInfo());
    if (Caster) Caster->SetIsAttacking(true);

    if (EmboldenerEffectClass)
    {
        FGameplayEffectContextHandle ContextHandle =
            ActorInfo->AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle =
            ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                EmboldenerEffectClass, GetAbilityLevel(), ContextHandle);

        if (SpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            const FString TargetName = TargetCharacter->GetName();
            UE_LOG(LogTemp, Log,
                TEXT("GA_Embolden: Emboldened %s — State.Advantage applied."),
                *TargetName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Embolden: EmboldenerEffectClass is null. Assign GE_Embolden in the Blueprint Class Defaults."));
    }

    // ── Step 4: Delay end so attack animation has time to play ────────────
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        FTimerDelegate Delegate;
        Delegate.BindLambda([this, Handle, ActorInfo, ActivationInfo, Caster]()
        {
            if (Caster) Caster->SetIsAttacking(false);
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        });
        World->GetTimerManager().SetTimer(TimerHandle, Delegate, 0.8f, false);
    }
    else
    {
        if (Caster) Caster->SetIsAttacking(false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}