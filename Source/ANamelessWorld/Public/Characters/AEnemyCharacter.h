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
};