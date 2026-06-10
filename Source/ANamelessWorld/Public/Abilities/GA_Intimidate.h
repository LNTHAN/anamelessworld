// GA_Intimidate.h
// PURPOSE: Protagonist debuff ability — finds the crack in an enemy's resolve
//          and forces them to hesitate. Applies State.Stunned, causing them
//          to lose their next turn. Contested INT vs WIS roll added Session 13.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Intimidate.generated.h"

UCLASS()
class ANAMELESSWORLD_API UGA_Intimidate : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UGA_Intimidate();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:

    // GameplayEffect that applies State.Stunned to the target enemy.
    // Assigned in BP_GA_Intimidate's Class Defaults.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    TSubclassOf<UGameplayEffect> IntimidateEffectClass;
};