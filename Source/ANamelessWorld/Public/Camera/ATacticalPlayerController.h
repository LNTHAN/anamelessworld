// ATacticalPlayerController.h
// PURPOSE: The tactical "brain". It drives the camera rig, but keeps a
//          reference to Nameless so mouse clicks and keys still command him.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ATacticalPlayerController.generated.h"

class APlayerCharacter;
class ABaseCharacter;

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

    // Arms an ability: the next left-click will target it instead of moving.
    // Called by command-menu buttons and by the 1/2/3 hotkeys. Refuses outside
    // PlayerTurn or with no Action left — same gate FireAbility checks anyway,
    // but failing here gives the player instant feedback instead of a dead click.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")

    void ArmAbility(FName TagName);

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
};