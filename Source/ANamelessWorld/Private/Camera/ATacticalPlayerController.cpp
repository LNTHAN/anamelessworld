// ATacticalPlayerController.cpp

#include "Camera/ATacticalPlayerController.h"
#include "Characters/APlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

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
}

void ATacticalPlayerController::OnMoveClicked()
{
    if (!ControlledCharacter) return;

    // Find what's under the mouse. If it's the floor, remember the spot.
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        return; // Clicked empty space — do nothing.
    }

    // Hand the spot to Nameless; he checks range and walks there himself.
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