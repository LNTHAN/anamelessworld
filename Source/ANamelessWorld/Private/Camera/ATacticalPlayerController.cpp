// ATacticalPlayerController.cpp

#include "Camera/ATacticalPlayerController.h"
#include "Characters/APlayerCharacter.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/ATacticalCameraPawn.h"
#include "Interactables/AInteractableActor.h"
#include "Utilities/UCRPGCombatLibrary.h"


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
    InputComponent->BindAction("Interact", IE_Pressed, this,
        &ATacticalPlayerController::OnInteractPressed);    
    InputComponent->BindAction("Confirm", IE_Pressed, this,
        &ATacticalPlayerController::OnConfirmPressed);
}

void ATacticalPlayerController::ArmAbility(FName TagName)
{
    if (!ControlledCharacter || !ControlledCharacter->TurnManager) return;

    // Same gate FireAbility checks — fail fast here instead of arming
    // something that would just refuse to fire on click anyway.
    if (ControlledCharacter->TurnManager->GetCurrentState() != ETurnState::PlayerTurn) return;
    if (!ControlledCharacter->bActionAvailable) return;

    ArmedAbilityTag = TagName;
    PendingTarget = nullptr;

    // Targeted abilities (and Interact) wait for a click; a self-cast AoE
    // like Intimidate has nothing to click, so it arms straight to Confirming.
    TargetingPhase = AbilityRequiresTarget(TagName)
        ? ETargetingPhase::Targeting
        : ETargetingPhase::Confirming;
}

void ATacticalPlayerController::OnMoveClicked()
{
    if (!ControlledCharacter) return;

    // Dialogue up: LMB advances the line (mouse-only play).
    if (ControlledCharacter->IsDialogueActive())
    {
        ControlledCharacter->AdvanceDialogue();
        return;
    }

    // Interact: one-click arm. Trace ECC_Visibility (shelf blocks it, capsules don't).
    if (ArmedAbilityTag == FName("Action.Interact"))
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            if (AInteractableActor* Object = Cast<AInteractableActor>(Hit.GetActor()))
            {
                if (ControlledCharacter->TryInteract(Object)) ResetTargeting();
            }
        }
        return;
    }

    // Confirming: any LMB commits. To re-pick, RMB back then click a new target.
    if (TargetingPhase == ETargetingPhase::Confirming)
    {
        ConfirmPendingAction();
        return;
    }

    // Targeting: the click must land on a character to stage it.
    if (TargetingPhase == ETargetingPhase::Targeting)
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Pawn, false, Hit))
        {
            if (ABaseCharacter* Target = Cast<ABaseCharacter>(Hit.GetActor()))
            {
                // Cast-range gate: too far → don't stage, stay in Targeting so the
                // player can move closer or pick a nearer target. No Action spent.
                const float Dist = FVector::Dist(
                    ControlledCharacter->GetActorLocation(), Target->GetActorLocation());
                if (Dist > ControlledCharacter->ConfuseCastRange)
                {
                    UE_LOG(LogTemp, Log,
                        TEXT("Confuse: %s is %.0f away (> %.0f cast range) — move closer."),
                        *Target->GetName(), Dist, ControlledCharacter->ConfuseCastRange);
                    return;
                }

                PendingTarget = Target;
                TargetingPhase = ETargetingPhase::Confirming;
            }
        }
        return;
    }

    // Idle: this click is a move destination.
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit)) return;
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
    // Back one beat: while confirming a targeted ability, drop the staged
    // target but stay armed so the player can re-pick. Any other state
    // (Targeting, or confirming a self-cast AoE) cancels fully to Idle.
    if (TargetingPhase == ETargetingPhase::Confirming &&
        AbilityRequiresTarget(ArmedAbilityTag))
    {
        PendingTarget = nullptr;
        TargetingPhase = ETargetingPhase::Targeting;
        return;
    }

    ResetTargeting();
}

void ATacticalPlayerController::ConfirmPendingAction()
{
    if (TargetingPhase != ETargetingPhase::Confirming) return;
    if (!ControlledCharacter) return;

    const FName TagToFire = ArmedAbilityTag;

    if (AbilityRequiresTarget(TagToFire))
    {
        // Targeted ability: we must have a staged target to fire at.
        if (!PendingTarget) return;
        ControlledCharacter->FireAbilityAtTarget(TagToFire, PendingTarget);
    }
    else
    {
        // Self-cast AoE (Intimidate): no target needed. FireAbilityAtTarget
        // still takes a target arg, so pass Nameless himself — GA_Intimidate
        // ignores it and sweeps everyone in its radius.
        ControlledCharacter->FireAbilityAtTarget(TagToFire, ControlledCharacter);
    }

    ResetTargeting(); // consumed — back to Idle
}

void ATacticalPlayerController::OnConfirmPressed()
{
    ConfirmPendingAction();
}

bool ATacticalPlayerController::AbilityRequiresTarget(FName TagName) const
{
    // Intimidate is a self-centered AoE — no character to click.
    // Everything else (Confuse, Interact) needs a click-target first.
    return TagName != FName("Ability.Debuff.Intimidate");
}

void ATacticalPlayerController::ResetTargeting()
{
    ArmedAbilityTag = NAME_None;
    TargetingPhase = ETargetingPhase::Idle;
    PendingTarget = nullptr;
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
    ArmAbility(FName("Ability.Debuff.Confuse"));
}

void ATacticalPlayerController::OnInteractPressed()
{
    // "Action.Interact" is a sentinel, not a real GameplayTag — it never reaches
    // FireAbility, it just flips OnMoveClicked into shelf-targeting mode. ArmAbility
    // still enforces the PlayerTurn + Action gate, same as the ability hotkeys.
    ArmAbility(FName("Action.Interact"));
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
    // A fresh turn always starts Idle — drop any ability/interact left armed from
    // a previous turn, or it hijacks every click (traces for a target that a plain
    // move-click will never satisfy) and the player can't move.
    ResetTargeting();

    if (!ActiveCombatant) return;

    if (ATacticalCameraPawn* CameraPawn = Cast<ATacticalCameraPawn>(GetPawn()))
    {
        // Offset above their head, not dead-center in their capsule — focusing
        // exactly on the capsule center puts the pivot inside their own
        // collision, which immediately triggers the spring arm's wall pull-in.
        CameraPawn->FocusOn(ActiveCombatant->GetActorLocation() + FVector(0.f, 0.f, 150.f));
        CameraPawn->SetFollowTarget(ActiveCombatant);
    }
}

FAbilityForecast ATacticalPlayerController::BuildForecast(
    ABaseCharacter* Attacker, ABaseCharacter* Target, FName AbilityTag) const
{
    FAbilityForecast F;

    if (AbilityTag == FName("Ability.Debuff.Confuse"))
    {
        F.bValid        = true;
        F.AbilityName   = FText::FromString(TEXT("Confuse"));
        F.HitChance     = 100;   // auto-applies today (a save may come in balance, J)
        F.InflictChance = 100;
        F.bDealsDamage  = false; // Nameless deals no direct damage → "--"
        F.StatusName    = FText::FromString(TEXT("Confused"));
    }
    else if (AbilityTag == FName("Ability.Attack.Basic"))
    {
        F.bValid       = true;
        F.AbilityName  = FText::FromString(TEXT("Strike"));
        F.HitChance = UCRPGCombatLibrary::CalculateHitChance(Attacker, Target);        
        F.bDealsDamage = true;
        F.Damage    = FMath::RoundToInt(
            UCRPGCombatLibrary::CalculateAttackDamage(Attacker, /*bHeavy=*/false));    }
    else if (AbilityTag == FName("Ability.Attack.Heavy"))
    {
        F.bValid       = true;
        F.AbilityName  = FText::FromString(TEXT("Heavy Strike"));
        F.HitChance    = UCRPGCombatLibrary::CalculateHitChance(Attacker, Target);
        F.bDealsDamage = true;
        F.Damage       = FMath::RoundToInt(
            UCRPGCombatLibrary::CalculateAttackDamage(Attacker, /*bHeavy=*/true));
    }
    // TODO: Intimidate (self-AoE) gets a highlight+summary, not this arrow.

    return F;
}

FAbilityForecast ATacticalPlayerController::GetPendingForecast() const
{
    if (TargetingPhase != ETargetingPhase::Confirming) return FAbilityForecast();
    return BuildForecast(ControlledCharacter, PendingTarget, ArmedAbilityTag);
}

void ATacticalPlayerController::ShowAttackForecast(
    ABaseCharacter* Attacker, ABaseCharacter* Target, FName AttackTag)
{
    ForecastAttacker       = Attacker;
    ForecastTarget         = Target;
    ExternalForecast       = BuildForecast(Attacker, Target, AttackTag);
    bExternalForecastActive = ExternalForecast.bValid;
}

void ATacticalPlayerController::HideAttackForecast()
{
    bExternalForecastActive = false;
    ForecastAttacker        = nullptr;
    ForecastTarget          = nullptr;
}

bool ATacticalPlayerController::GetActiveForecast(
    FAbilityForecast& OutForecast,
    ABaseCharacter*& OutAttacker,
    ABaseCharacter*& OutTarget) const
{
    // Player's Confirming phase wins if active.
    if (TargetingPhase == ETargetingPhase::Confirming)
    {
        OutForecast = GetPendingForecast();
        OutAttacker = ControlledCharacter;   // player = attacker (left)
        OutTarget   = PendingTarget;
        return OutForecast.bValid;
    }

    // Otherwise, an enemy pre-strike forecast, if one is showing.
    if (bExternalForecastActive)
    {
        OutForecast = ExternalForecast;
        OutAttacker = ForecastAttacker;      // enemy = attacker (left)
        OutTarget   = ForecastTarget;
        return true;
    }

    return false;
}