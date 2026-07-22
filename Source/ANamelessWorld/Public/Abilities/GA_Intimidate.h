// GA_Intimidate.h
// PURPOSE: Protagonist debuff ability — finds the crack in an enemy's resolve
//          and forces them to hesitate. Applies State.Stunned, causing them
//          to lose their next turn. Contested INT vs WIS roll added Session 13.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Intimidate.generated.h"

class ABaseCharacter;

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

    // Read-only access for the range-ring indicator (Block K) — so the ring reads
    // the ability's real AoE radius instead of a duplicated number.
    float GetIntimidateRadius() const { return IntimidateRadius; }

protected:

    // Impact damage applied when a shoved enemy slams into something solid.
    // Set to GE_DamageInstant in BP_GA_Intimidate — reuses the existing effect.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    TSubclassOf<UGameplayEffect> DamageEffect;

    // How far from Nameless an enemy can be and still get caught in the fear (cm).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    float IntimidateRadius = 600.f;

    // Flings one enemy away from Caster, up to its MoveRange, stopping at the
    // first solid hit (wall / shelf / other enemy). Impact damage on collision,
    // mutual damage if it slammed into another enemy. Nameless is never hit.
    void DisplaceEnemy(ABaseCharacter* Enemy, AActor* Caster);

    // Applies DamageEffect to one victim's ASC.
    void ApplyImpactDamage(ABaseCharacter* Victim);    

    // GameplayEffect that applies State.Stunned to the target enemy.
    // Assigned in BP_GA_Intimidate's Class Defaults.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Ability")
    TSubclassOf<UGameplayEffect> IntimidateEffectClass;
};