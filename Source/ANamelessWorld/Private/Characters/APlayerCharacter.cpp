// APlayerCharacter.cpp

#include "Characters/APlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Dialogue/UDialogueComponent.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"
#include "TimerManager.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

APlayerCharacter::APlayerCharacter()
{
    DialogueComp = CreateDefaultSubobject<UDialogueComponent>(TEXT("Dialogue"));
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Show the cursor and enable click hit-testing so the player can click
    // the battlefield to move (tactical top-down control).
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction(
        "Attack", IE_Pressed, this, &APlayerCharacter::OnAttackPressed);

    PlayerInputComponent->BindAction(
        "AdvanceDialogue", IE_Pressed, this, &APlayerCharacter::AdvanceDialogue);
}

void APlayerCharacter::TryMoveTo(const FVector& Destination)
{
    // Turn gate: only move on the player's turn.
    if (!TurnManager || TurnManager->GetCurrentState() != ETurnState::PlayerTurn)
    {
        return;
    }

    // Action economy: only if the Move stock is still available this turn.
    if (!bMoveAvailable)
    {
        UE_LOG(LogTemp, Log, TEXT("APlayerCharacter: no move left this turn."));
        return;
    }
    
    // Ask the navigation system for the ACTUAL walking route from where we
    // stand to the target spot — routed around obstacles, not a straight line.
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
        GetWorld(), GetActorLocation(), Destination, this);

    // Reject if: there's no path, it's invalid, or it's "partial" (the spot
    // can't actually be reached — e.g. off the walkable floor).
    if (!Path || !Path->IsValid() || Path->IsPartial())
    {
        return;
    }

    // The tactical rule: route length must be within MoveRange. Length is
    // measured along the floor, so walls and detours count against the budget.
    if (Path->GetPathLength() > MoveRange)
    {
        UE_LOG(LogTemp, Log,
            TEXT("APlayerCharacter: click out of range (%.0f > %.0f)."),
            Path->GetPathLength(), MoveRange);
        return;
    }

    // In range — spend the Move stock, then walk there via the AIController.
    bMoveAvailable = false;
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), Destination);
}

void APlayerCharacter::FireAbilityAtTarget(const FName& TagName, ABaseCharacter* Target)
{
    CurrentTarget = Target;
    FireAbility(TagName);
}

void APlayerCharacter::SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InTarget)
{
    TurnManager = InTurnManager;
    CurrentTarget = InTarget;
}

// ── Shared ability firing logic ───────────────────────────────────────────────

void APlayerCharacter::FireAbility(const FName& TagName)
{
    if (!TurnManager) return;
    if (TurnManager->GetCurrentState() != ETurnState::PlayerTurn) return;

    // Action economy: only if the Action stock is still available this turn.
    if (!bActionAvailable)
    {
        UE_LOG(LogTemp, Log, TEXT("APlayerCharacter: no action left this turn."));
        return;
    }

    if (!CurrentTarget || !CurrentTarget->IsAlive()) return;

    // Spend the Action stock. The turn does NOT end here anymore — the player
    // may still move (if unspent) and ends the turn explicitly.
    bActionAvailable = false;

    FGameplayEventData Payload;
    Payload.Target = CurrentTarget;

    FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName);
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Tag, Payload);
}

void APlayerCharacter::EndPlayerTurn()
{
    // Only end our turn when it's actually our turn.
    if (TurnManager && TurnManager->GetCurrentState() == ETurnState::PlayerTurn)
    {
        TurnManager->EndTurn();
    }
}

// ── Public ability functions (called by command menu buttons) ─────────────────

void APlayerCharacter::OnAttackPressed()
{
    if (DialogueComp && DialogueComp->IsDialogueActive())
    {
        AdvanceDialogue();
        return;
    }

    FireAbility(FName("Ability.Attack.Basic"));
}

void APlayerCharacter::AdvanceDialogue()
{
    if (!DialogueComp || !DialogueComp->IsDialogueActive()) return;

    DialogueComp->AdvanceLine();
}

void APlayerCharacter::UseEmbolden()
{
    // Targets an ally — for now CurrentTarget is used.
    // Session 13 will add a separate ally-targeting flow.
    FireAbility(FName("Ability.Support.Embolden"));
}

void APlayerCharacter::UseIntimidate()
{
    // Targets the currently selected enemy.
    FireAbility(FName("Ability.Debuff.Intimidate"));
}

void APlayerCharacter::UseProvoke()
{
    // Targets the currently selected enemy.
    FireAbility(FName("Ability.Debuff.Provoke"));
}

// ── Target cycling ────────────────────────────────────────────────────────────

void APlayerCharacter::AddTarget(ABaseCharacter* NewTarget)
{
    if (NewTarget) AllTargets.Add(NewTarget);
}

void APlayerCharacter::CycleTarget()
{
    if (AllTargets.Num() == 0) return;

    // Find the index of the current target.
    int32 CurrentIndex = AllTargets.IndexOfByKey(CurrentTarget);

    // Search forward for the next living target, wrapping around.
    for (int32 i = 1; i <= AllTargets.Num(); i++)
    {
        int32 NextIndex = (CurrentIndex + i) % AllTargets.Num();
        if (AllTargets[NextIndex] && AllTargets[NextIndex]->IsAlive())
        {
            CurrentTarget = AllTargets[NextIndex];
            UE_LOG(LogTemp, Log, TEXT("APlayerCharacter: Target switched to %s."),
                *CurrentTarget->GetName());
            return;
        }
    }
}
