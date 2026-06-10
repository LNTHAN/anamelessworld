// GA_Embolden.h
// PURPOSE: Protagonist buff ability — emboldens one ally by granting them
//          the State.Advantage tag. The CHA modifier bonus to their next
//          damage roll is wired in Session 13 when the d20 system is built.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Embolden.generated.h"

UCLASS()
class ANAMELESSWORLD_API UGA_Embolden : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UGA_Embolden();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:

    // GameplayEffect that applies State.Advantage to the target ally.
    // Assigned in BP_GA_Embolden's Class Defaults.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    TSubclassOf<UGameplayEffect> EmboldenerEffectClass;
};