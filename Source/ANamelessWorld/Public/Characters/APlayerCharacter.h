// APlayerCharacter.h
// PURPOSE: The human-controlled character. Inherits all GAS/stat/ability
//          functionality from ABaseCharacter and adds keyboard input.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "APlayerCharacter.generated.h"

class UDialogueComponent;
class AInteractableActor;
class UDecalComponent;
class UInstancedStaticMeshComponent;

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

    // This IS the player's character.
    virtual bool IsPlayerCharacter() const override { return true; }

    // Called when the player presses the attack key.
    // Fires the Ability.Attack.Basic event targeting the first living enemy.
    void OnAttackPressed();

    // Walk to a floor spot the controller already picked. Checks the move-range
    // rule first (route length must fit inside MoveRange), then paths there.
    void TryMoveTo(const FVector& Destination);

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

    // Fires an ability at an explicit target — called by the controller when a
    // click lands on a valid target while an ability is armed. Reuses the same
    // gating as every other ability call (PlayerTurn + Action stock) because it
    // just sets CurrentTarget and calls the existing FireAbility().
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void FireAbilityAtTarget(const FName& TagName, ABaseCharacter* Target);

    // How close Nameless must stand to rig an interactable, in cm (~200 ≈ 2 m).
    // He physically arms it — no rigging a shelf from across the room.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Combat")
    float InteractRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ANW|Combat")
    float ConfuseCastRange = 600.f;

    // ── Range indicators (Block K) ───────────────────────────────────────────
    // Terrain-exact move zone: one tile per reachable navmesh cell (FE-style).
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|UI")
    UInstancedStaticMeshComponent* MoveReachISM;

    // Red ring for whatever targeted/AoE ability is armed (Confuse cast range,
    // Intimidate AoE, future abilities). Sized per-ability by the controller.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|UI")
    UDecalComponent* AbilityRangeDecal;

    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetMoveRangeVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetAbilityRangeVisible(bool bVisible);

    // Resize the ability ring to a radius (cm), just before showing it.
    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetAbilityRingRadius(float Radius);

    // Ring radius for an armed ability tag, read from the ability's own range so
    // the indicator can't drift from the effect. 0 = this ability has no ring.
    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    float GetAbilityRingRadius(FName Tag) const;

    // Interact command: rig the given object. Gated like every other action
    // (PlayerTurn + Action stock) plus a range check. Returns true only if it
    // actually armed (spent the Action) — the controller uses that to decide
    // whether to un-arm Interact mode or let the player move closer and retry.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    bool TryInteract(AInteractableActor* Target);

    // Ends the player's turn — the explicit "I'm done" command the End Turn
    // button calls. Allowed any time during the player's own turn (stocks
    // are skippable), ignored otherwise.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void EndPlayerTurn();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Components")
    UDialogueComponent* DialogueComp;

    // True while a dialogue line is on screen — lets the controller consume an
    // LMB as "advance the line" so the game plays mouse-only.
    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    bool IsDialogueActive() const;

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

    // Rebuilds the reachable-tile set for the current position + MoveRange.
    void ComputeMoveReachable();
};
