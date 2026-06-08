// UCRPGCharacterData.h
// PURPOSE: Holds all per-character configuration in one place.
//          Reference this from any character Blueprint — no more per-Blueprint GAS wiring.
//          Ability parameters (damage, cooldown, etc.) live in GAS assets (GameplayEffect,
//          GameplayAbility), not here. This asset only controls which abilities a character
//          gets and which attribute effect initialises their stats.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UCRPGCharacterData.generated.h"

class UGameplayAbility;
class UGameplayEffect;

UCLASS(BlueprintType)
class ANAMELESSWORLD_API UCRPGCharacterData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    // Display name shown in UI ("Aria", "Goblin", "Dark Golem")
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Identity")
    FText CharacterName;

    // GameplayEffect that sets starting attribute values (Health, Mana, STR, etc.)
    // Replaces ABaseCharacter::DefaultAttributeEffect
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|GAS")
    TSubclassOf<UGameplayEffect> AttributeEffect;

    // Abilities to grant on spawn. Replaces ABaseCharacter::DefaultAbilities
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|GAS")
    TArray<TSubclassOf<UGameplayAbility>> Abilities;

    // 0-100. Chance per turn the enemy uses Heavy Strike instead of Basic Attack.
    // Only used by AEnemyCharacter and ABossCharacter — ignored for player.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Combat")
    int32 HeavyStrikeChance = 0;
};
