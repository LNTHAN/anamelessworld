// GA_Intimidate.cpp
// PURPOSE: Applies State.Stunned to a target enemy, causing them to lose
//          their next turn. The contested INT vs WIS roll that determines
//          success or failure is wired in Session 13.

#include "Abilities/GA_Intimidate.h"
#include "AbilitySystemComponent.h"
#include "Characters/ABaseCharacter.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"


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

    // ── Step 3: Contested roll — INT vs target WIS DC ─────────────────────
    // The protagonist's INT modifier sets how hard it is to resist.
    // The target's WIS modifier sets the DC they need to beat.
    // If the protagonist's roll meets or beats the DC, the stun lands.

    UAbilitySystemComponent* TargetASC =
        TargetCharacter->GetAbilitySystemComponent();

    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: Target has no AbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Get protagonist's INT modifier from their AttributeSet.
    const UCRPGAttributeSet* CasterAttributes = Cast<UCRPGAttributeSet>(
        ActorInfo->AvatarActor->FindComponentByClass<UAbilitySystemComponent>()
            ->GetAttributeSet(UCRPGAttributeSet::StaticClass()));

    // Get target's WIS modifier to build the DC.
    const UCRPGAttributeSet* TargetAttributes = Cast<UCRPGAttributeSet>(
        TargetASC->GetAttributeSet(UCRPGAttributeSet::StaticClass()));

    if (!CasterAttributes || !TargetAttributes)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GA_Intimidate: Could not read AttributeSets. Ending ability."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const int32 INTModifier = UCRPGCombatLibrary::GetModifier(
        CasterAttributes->GetIntelligence());

    const int32 WisdomDC = UCRPGCombatLibrary::CalculateDC(
        UCRPGCombatLibrary::GetModifier(TargetAttributes->GetWisdom()));

    // Roll d20 + INT modifier and compare to the target's WIS DC.
    const int32 RawRoll = UCRPGCombatLibrary::RollD20();
    const int32 FinalRoll = RawRoll + INTModifier;

    UE_LOG(LogTemp, Log,
        TEXT("GA_Intimidate: Rolled %d + %d (INT) = %d vs DC %d."),
        RawRoll, INTModifier, FinalRoll, WisdomDC);

    if (FinalRoll >= WisdomDC)
    {
        // Success — apply the stun.
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
                    TEXT("GA_Intimidate: Success! %s is now Stunned — loses next turn."),
                    *TargetName);
            }
        }
    }
    else
    {
        // Failure — the target resisted. Mana is still spent.
        const FString TargetName = TargetCharacter->GetName();
        UE_LOG(LogTemp, Log,
            TEXT("GA_Intimidate: Failed. %s resisted the intimidation."),
            *TargetName);
    }

    // ── Step 4: End cleanly ────────────────────────────────────────────────
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}