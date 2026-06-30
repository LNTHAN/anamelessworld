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

    PlayerInputComponent->BindAction(
        "MoveClick", IE_Pressed, this, &APlayerCharacter::OnMoveClicked);
}

void APlayerCharacter::OnMoveClicked()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    // Find what's under the mouse cursor on the Visibility channel.
    FHitResult Hit;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        // Path this character to the clicked floor point, around obstacles,
        // using the NavMesh. (Step 3 will reject clicks outside move range.)
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, Hit.Location);
    }
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
    if (!CurrentTarget || !CurrentTarget->IsAlive()) return;

    FGameplayEventData Payload;
    Payload.Target = CurrentTarget;

    FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName);
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Tag, Payload);

    // End the player's turn after a delay so the animation has time to play.
    // All player abilities go through FireAbility, so this covers every action.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TurnEndTimerHandle, this, &APlayerCharacter::EndTurnNow, 2.0f, false);
    }
}

void APlayerCharacter::EndTurnNow()
{
    if (TurnManager)
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
