// APlayerCharacter.cpp

#include "Characters/APlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"

APlayerCharacter::APlayerCharacter()
{
    // Input is handled by SetupPlayerInputComponent — nothing to do here.
}

void APlayerCharacter::BeginPlay()
{
    // Run ABaseCharacter::BeginPlay() first — it initialises GAS and grants abilities.
    Super::BeginPlay();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind the Space bar to OnAttackPressed.
    // "Attack" is an Action Mapping we'll register in Project Settings.
    // IE_Pressed = fire when the key goes DOWN (not held, not released).
    PlayerInputComponent->BindAction(
        "Attack", IE_Pressed, this, &APlayerCharacter::OnAttackPressed);
}

void APlayerCharacter::SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InTarget)
{
    TurnManager = InTurnManager;
    CurrentTarget = InTarget;
}

void APlayerCharacter::OnAttackPressed()
{
    // Guard: only act on the player's turn.
    if (!TurnManager) return;
    if (TurnManager->GetCurrentState() != ETurnState::PlayerTurn) return;

    // Guard: need a living target.
    if (!CurrentTarget || !CurrentTarget->IsAlive()) return;

    // Build the event payload — this is how GA_BasicAttack receives its target.
    FGameplayEventData Payload;
    Payload.Target = CurrentTarget;

    // Fire the gameplay event. This activates BP_GA_BasicAttack via its trigger tag.
    FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(
        FName("Ability.Attack.Basic"));

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, AttackTag, Payload);

    // Tell UTurnManager the player's turn is done.
    TurnManager->EndTurn();
}