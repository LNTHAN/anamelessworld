// ATacticalPlayerController.h
// PURPOSE: The tactical "brain". It drives the camera rig, but keeps a
//          reference to Nameless so mouse clicks and keys still command him.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ATacticalPlayerController.generated.h"

class APlayerCharacter;

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

private:
    // Left-click: find the floor spot under the mouse, tell Nameless to walk there.
    void OnMoveClicked();

    // Pass-through commands to Nameless.
    void OnAttackPressed();
    void OnAdvanceDialogue();
};