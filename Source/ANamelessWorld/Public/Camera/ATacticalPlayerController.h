// ATacticalPlayerController.h
// PURPOSE: The tactical "brain". It drives the camera rig, but keeps a
//          reference to Nameless so mouse clicks and keys still command him.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ATacticalPlayerController.generated.h"

class APlayerCharacter;
class ABaseCharacter;

// The targeting mode's three beats. Idle = normal (click moves). Targeting =
// an ability is armed, waiting for the player to click a target. Confirming =
// a target is staged (or the ability is self-cast), waiting for Space/Confirm.
UENUM(BlueprintType)
enum class ETargetingPhase : uint8
{
    Idle,
    Targeting,
    Confirming
};

UCLASS()
class ANAMELESSWORLD_API ATacticalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // Where player commands are set up (the mouse/keys we listen for).
    virtual void SetupInputComponent() override;

    // Our sticky note pointing to Nameless — the character we command.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    APlayerCharacter* ControlledCharacter;

    // Idle when NAME_None. Otherwise the tag of the ability waiting for a
    // target click — the whole "modal input" state fits in one FName.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    FName ArmedAbilityTag = NAME_None;

    // Which beat of the targeting flow we're in. Idle/Targeting/Confirming.
    // ArmedAbilityTag says WHICH ability; this says WHERE in the flow we are.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    ETargetingPhase TargetingPhase = ETargetingPhase::Idle;

    // The staged target, chosen on the first click but not yet fired at.
    // Null while Targeting, and stays null for self-cast AoE (e.g. Intimidate)
    // which has no character to click — those go straight to Confirming.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    ABaseCharacter* PendingTarget = nullptr;

    // Arms an ability: the next left-click will target it instead of moving.
    // Called by command-menu buttons and by the 1/2/3 hotkeys. Refuses outside
    // PlayerTurn or with no Action left — same gate FireAbility checks anyway,
    // but failing here gives the player instant feedback instead of a dead click.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")

    void ArmAbility(FName TagName);

    // Commits the staged action: fires ArmedAbilityTag at PendingTarget, then
    // returns to Idle. No-op unless we're in the Confirming phase. Bound to the
    // Confirm key and callable from a Confirm button.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void ConfirmPendingAction();

    // Subscribes the camera to TurnManager's turn-started broadcasts, so it
    // can auto-focus on whoever's turn it is. Must be called from the Level
    // Blueprint BEFORE Start Combat fires (see .cpp comment) — otherwise the
    // very first combatant's turn gets missed.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void BindToTurnManager();
    
private:
    // Left-click: behavior depends on ArmedAbilityTag (see .cpp).
    void OnMoveClicked();

    // Pass-through commands to Nameless.
    void OnAttackPressed();
    void OnAdvanceDialogue();

    // RMB/Esc: un-arms whatever ability is armed. No-op if already Idle.
    void OnCancelPressed();

    // 1/2/3 hotkeys — thin wrappers so BindAction (no payload support) can
    // each arm a specific ability tag.
    void OnAbilityOnePressed();   // Ability.Support.Embolden
    void OnAbilityTwoPressed();   // Ability.Debuff.Intimidate
    void OnAbilityThreePressed(); // Ability.Debuff.Confuse

    UFUNCTION()
    void OnCombatTurnStarted(ABaseCharacter* ActiveCombatant);
    
    // Key 4 — arms Interact mode (sentinel tag Action.Interact). The next click
    // rigs a bookshelf instead of targeting a character.
    void OnInteractPressed();    

    // Confirm key (Space): thin wrapper → ConfirmPendingAction().
    void OnConfirmPressed();

    // True if this ability needs the player to click a character first
    // (Confuse, Interact). False for self-centered AoE (Intimidate), which
    // skips Targeting and arms straight into Confirming.
    bool AbilityRequiresTarget(FName TagName) const;

    // Clears all targeting state back to Idle in one call.
    void ResetTargeting();
};