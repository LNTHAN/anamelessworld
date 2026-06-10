// GA_Provoke.h
// PURPOSE: Protagonist enrage ability — gets under an enemy's skin and forces
//          them to attack the protagonist with Disadvantage. If they miss,
//          they damage themselves. The redirect + self-damage logic is wired
//          in Session 13 when the d20 system is built. For now, applies
//          State.Enraged to the target enemy.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Provoke.generated.h"

UCLASS()
class ANAMELESSWORLD_API UGA_Provoke : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UGA_Provoke();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:

    // GameplayEffect that applies State.Enraged to the target enemy.
    // Assigned in BP_GA_Provoke's Class Defaults.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    TSubclassOf<UGameplayEffect> ProvokeEffectClass;
};