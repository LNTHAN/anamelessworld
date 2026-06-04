// APlayerCharacter.h
// PURPOSE: The human-controlled character. Inherits all GAS/stat/ability
//          functionality from ABaseCharacter and adds keyboard input.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "APlayerCharacter.generated.h"

UCLASS()
class ANAMELESSWORLD_API APlayerCharacter : public ABaseCharacter
{
    GENERATED_BODY()

public:

    APlayerCharacter();

    virtual void BeginPlay() override;

    // UE5 calls this once when the player controller possesses this pawn.
    // We bind key presses to ability functions here.
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;

    // Called when the player presses the attack key.
    // Fires the Ability.Attack.Basic event targeting the first living enemy.
    void OnAttackPressed();

    // Called from the Level Blueprint to wire up combat references.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InTarget);

    // Reference to the turn manager — assigned in BeginPlay via the level.
    // APlayerCharacter calls EndTurn() on this after attacking.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    UTurnManager* TurnManager;

    // The enemy to target when attacking.
    // Set in the editor per-instance (drag the enemy actor in).
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    ABaseCharacter* CurrentTarget;
};