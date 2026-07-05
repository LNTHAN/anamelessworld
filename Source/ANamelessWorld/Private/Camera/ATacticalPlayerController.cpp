// ATacticalPlayerController.cpp

#include "Camera/ATacticalPlayerController.h"
#include "Characters/APlayerCharacter.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/ATacticalCameraPawn.h"

void ATacticalPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Show the cursor and let clicks hit things in the world.
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // Look through the level and grab Nameless so we can command him later.
    ControlledCharacter = Cast<APlayerCharacter>(
        UGameplayStatics::GetActorOfClass(this, APlayerCharacter::StaticClass()));
}

void ATacticalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // These names match the ActionMappings in DefaultInput.ini.
    InputComponent->BindAction("MoveClick", IE_Pressed, this,
        &ATacticalPlayerController::OnMoveClicked);
    InputComponent->BindAction("Attack", IE_Pressed, this,
        &ATacticalPlayerController::OnAttackPressed);
    InputComponent->BindAction("AdvanceDialogue", IE_Pressed, this,
        &ATacticalPlayerController::OnAdvanceDialogue);
    InputComponent->BindAction("Cancel", IE_Pressed, this,
        &ATacticalPlayerController::OnCancelPressed);
    InputComponent->BindAction("AbilityOne", IE_Pressed, this,
        &ATacticalPlayerController::OnAbilityOnePressed);
    InputComponent->BindAction("AbilityTwo", IE_Pressed, this,
        &ATacticalPlayerController::OnAbilityTwoPressed);
    InputComponent->BindAction("AbilityThree", IE_Pressed, this,
        &ATacticalPlayerController::OnAbilityThreePressed);
}

void ATacticalPlayerController::ArmAbility(FName TagName)
{
    if (!ControlledCharacter || !ControlledCharacter->TurnManager) return;

    // Same gate FireAbility checks — fail fast here instead of arming
    // something that would just refuse to fire on click anyway.
    if (ControlledCharacter->TurnManager->GetCurrentState() != ETurnState::PlayerTurn) return;
    if (!ControlledCharacter->bActionAvailable) return;

    ArmedAbilityTag = TagName;
}

void ATacticalPlayerController::OnMoveClicked()
{
    if (!ControlledCharacter) return;

    // Armed: this click picks a TARGET, not a destination.
    if (ArmedAbilityTag != NAME_None)
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Pawn, false, Hit))
        {
            if (ABaseCharacter* Target = Cast<ABaseCharacter>(Hit.GetActor()))
            {
                ControlledCharacter->FireAbilityAtTarget(ArmedAbilityTag, Target);
                ArmedAbilityTag = NAME_None; // consumed — back to Idle
            }
        }
        // Missed (empty space or a non-target actor): stay armed, try again.
        // Armed clicks never fall through to movement.
        return;
    }

    // Idle: this click is a move destination.
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        return; // Clicked empty space — do nothing.
    }
    ControlledCharacter->TryMoveTo(Hit.Location);
}

void ATacticalPlayerController::OnAttackPressed()
{
    if (ControlledCharacter) ControlledCharacter->OnAttackPressed();
}

void ATacticalPlayerController::OnAdvanceDialogue()
{
    if (ControlledCharacter) ControlledCharacter->AdvanceDialogue();
}

void ATacticalPlayerController::OnCancelPressed()
{
    ArmedAbilityTag = NAME_None;
}

void ATacticalPlayerController::OnAbilityOnePressed()
{
    ArmAbility(FName("Ability.Support.Embolden"));
}

void ATacticalPlayerController::OnAbilityTwoPressed()
{
    ArmAbility(FName("Ability.Debuff.Intimidate"));
}

void ATacticalPlayerController::OnAbilityThreePressed()
{
    ArmAbility(FName("Ability.Debuff.Provoke"));
}

void ATacticalPlayerController::BindToTurnManager()
{
    // ControlledCharacter->TurnManager only becomes valid once the Level
    // Blueprint calls SetupCombat() on the player — this function must run
    // AFTER that, but BEFORE TurnManager->StartCombat() fires, or the first
    // combatant's OnTurnStarted broadcast happens with nobody subscribed yet.
    if (!ControlledCharacter || !ControlledCharacter->TurnManager) return;

    ControlledCharacter->TurnManager->OnTurnStarted.AddDynamic(
        this, &ATacticalPlayerController::OnCombatTurnStarted);
}

void ATacticalPlayerController::OnCombatTurnStarted(ABaseCharacter* ActiveCombatant)
{
    if (!ActiveCombatant) return;

    if (ATacticalCameraPawn* CameraPawn = Cast<ATacticalCameraPawn>(GetPawn()))
    {
        // Offset above their head, not dead-center in their capsule — focusing
        // exactly on the capsule center puts the pivot inside their own
        // collision, which immediately triggers the spring arm's wall pull-in.
        CameraPawn->FocusOn(ActiveCombatant->GetActorLocation() + FVector(0.f, 0.f, 150.f));
    }
}