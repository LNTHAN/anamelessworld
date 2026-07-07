// AEnemyCharacter.h
// PURPOSE: AI-controlled enemy. Inherits GAS from ABaseCharacter.
//          On its turn, automatically attacks the player.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "AEnemyCharacter.generated.h"

UCLASS()
class ANAMELESSWORLD_API AEnemyCharacter : public ABaseCharacter
{
    GENERATED_BODY()

public:

    AEnemyCharacter();

    virtual void BeginPlay() override;

    // Called by UTurnManager (via OnTurnStarted delegate) when it's this enemy's turn.
    UFUNCTION()
    void ExecuteAITurn(ABaseCharacter* ActiveCombatant);

    // Called from the Level Blueprint to wire up combat references.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InPlayerTarget);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    UTurnManager* TurnManager;

    // The player character to attack. Set in the editor.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    ABaseCharacter* PlayerTarget;

    // 0 = never use heavy strike, 100 = always use heavy strike.
    // Default 30 = 30% chance per turn. Override per-instance in the editor.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ANW|Combat")
    int32 HeavyStrikeChance = 30;

    // Intermission at the start of this enemy's turn, in seconds, before it acts.
    // Lets the camera finish focusing so the attack animation is actually seen.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Combat")
    float TurnStartDelay = 1.5f;    

private:
    FTimerHandle TurnEndTimerHandle;

    void EndTurnAfterDelay();
    void EndTurnNow();

    FTimerHandle TurnStartTimerHandle;

    // The real turn logic (decide + attack), run after the TurnStartDelay beat.
    void PerformAITurn();
        
    // Finds the nearest OTHER living combatant to this one (via TurnManager's
    // roster) — Confuse's targeting rule: attack whoever's closest, not
    // necessarily the protagonist.
    ABaseCharacter* FindNearestOtherCombatant() const;
};