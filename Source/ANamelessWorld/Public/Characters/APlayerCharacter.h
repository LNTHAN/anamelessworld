// APlayerCharacter.h
// PURPOSE: The human-controlled character. Inherits all GAS/stat/ability
//          functionality from ABaseCharacter and adds keyboard input.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "APlayerCharacter.generated.h"

class UDialogueComponent;

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

    // Called by command menu buttons to fire a specific ability.
    // Emboldens a target ally — applies State.Advantage to their next roll.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void UseEmbolden();

    // Intimidates a target enemy — applies State.Stunned, losing their next turn.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void UseIntimidate();

    // Provokes a target enemy — applies State.Enraged, forcing them to attack
    // the protagonist with Disadvantage on their next turn.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void UseProvoke();

    // Cycles CurrentTarget to the next living enemy in the combatant list.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void CycleTarget();

    // Adds a single target to the AllTargets array.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void AddTarget(ABaseCharacter* NewTarget);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Components")
    UDialogueComponent* DialogueComp;

    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    void AdvanceDialogue();

    // All possible targets — set by Level Blueprint after combat starts.
    UPROPERTY(BlueprintReadWrite, Category = "ANW|Combat")
    TArray<ABaseCharacter*> AllTargets;

    // Reference to the turn manager — assigned in BeginPlay via the level.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    UTurnManager* TurnManager;

    // The enemy to target when attacking.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    ABaseCharacter* CurrentTarget;

private:
    void FireAbility(const FName& TagName);

    FTimerHandle TurnEndTimerHandle;
    void EndTurnNow();
};
