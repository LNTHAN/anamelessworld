// ATacticalPlayerController.cpp

#include "Camera/ATacticalPlayerController.h"
#include "Characters/APlayerCharacter.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/ATacticalCameraPawn.h"
#include "Interactables/AInteractableActor.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Characters/AEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"


void ATacticalPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Show the cursor and let clicks hit things in the world.
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // Reassert Game-and-UI input on every level start. Open Level (e.g. the
    // result screen's Retry) carries the viewport's input mode over from the
    // previous map — if that was UI Only (set when the result screen showed),
    // clicks and keys never reach gameplay and the opening dialogue stalls.
    // This clears that carry-over so a fresh battle always accepts input.
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

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

    UpdateRangeIndicators();
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

    // Idle: a click on a living unit inspects it; a click on the ground moves.
    // Trace pawns first (ECC_Pawn) — capsules block it, the floor doesn't.
    FHitResult PawnHit;
    if (GetHitResultUnderCursor(ECC_Pawn, false, PawnHit))
    {
        if (ABaseCharacter* Unit = Cast<ABaseCharacter>(PawnHit.GetActor()))
        {
            if (Unit->IsAlive())   // a corpse falls through to a normal move
            {
                ToggleInspect(Unit);
                return;
            }
        }
    }

    // Empty ground: drop any inspection, then move.
    FHitResult FloorHit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, FloorHit)) return;
    ClearInspection();
    ControlledCharacter->TryMoveTo(FloorHit.Location);

    UpdateRangeIndicators();   // move may have been spent → hide the move zone
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

    UpdateRangeIndicators();
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
    ClearInspection();   // pins + card don't carry across turns

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

void ATacticalPlayerController::UpdateThreatLines()
{
    if (!ControlledCharacter || !ControlledCharacter->TurnManager) return;

    UTurnManager* TM = ControlledCharacter->TurnManager;
    const bool bPlayerTurn = TM->GetCurrentState() == ETurnState::PlayerTurn;
    ABaseCharacter* Active = TM->GetCurrentCombatant();

    const FLinearColor Red(1.0f, 0.03f, 0.04f, 1.0f);
    const FLinearColor Yellow(1.0f, 0.62f, 0.0f, 1.0f);

    for (ABaseCharacter* C : TM->GetTurnOrder())
    {
        AEnemyCharacter* E = Cast<AEnemyCharacter>(C);
        if (!E) continue;

        bool bShow = false;
        FLinearColor Color = Red;
        float Opacity = 1.0f;
        ABaseCharacter* Target = E->IsAlive() ? E->GetIntendedTarget() : nullptr;

        if (Target && Target->IsAlive())
        {
            const bool bInReach = FVector::Dist(
                E->GetActorLocation(), Target->GetActorLocation()) <= E->GetThreatRadius();

            if (E->IsConfused())
            {
                // Yellow: from cast until the start of its OWN turn. Solid if it will
                // strike this turn, faded if it can only close distance.
                if (E != Active)
                {
                    bShow = true;
                    Color = Yellow;
                    Opacity = bInReach ? 1.0f : 0.35f;
                }
            }
            else if (bPlayerTurn && bInReach)
            {
                // Red: natural threat, shown for planning, imminent only.
                bShow = true;
                Color = Red;
                Opacity = 1.0f;
            }
        }

        E->SetThreatLine(bShow, Target, Color, Opacity);
    }
}

void ATacticalPlayerController::UpdateRangeIndicators()
{
    if (!ControlledCharacter) return;

    const bool bPlayerTurn = ControlledCharacter->TurnManager
        && ControlledCharacter->TurnManager->GetCurrentState() == ETurnState::PlayerTurn;

    // Move zone: your turn AND nothing armed (Idle).
    ControlledCharacter->SetMoveRangeVisible(
        bPlayerTurn && ControlledCharacter->bMoveAvailable
        && TargetingPhase == ETargetingPhase::Idle);

    // Ability ring: your turn AND an armed ability that HAS a ring (radius > 0),
    // while picking a target or confirming. Radius = the ability's real range.
    const bool bTargetingFlow =
        TargetingPhase == ETargetingPhase::Targeting ||
        TargetingPhase == ETargetingPhase::Confirming;
    const float RingRadius = bTargetingFlow
        ? ControlledCharacter->GetAbilityRingRadius(ArmedAbilityTag) : 0.f;

    const bool bShowRing = bPlayerTurn && RingRadius > 0.f;
    if (bShowRing) ControlledCharacter->SetAbilityRingRadius(RingRadius);
    ControlledCharacter->SetAbilityRangeVisible(bShowRing);

    UpdateTargetAuras();
}

void ATacticalPlayerController::UpdateTargetAuras()
{
    if (!ControlledCharacter) return;

    const bool bPlayerTurn = ControlledCharacter->TurnManager
        && ControlledCharacter->TurnManager->GetCurrentState() == ETurnState::PlayerTurn;

    // Auras only for enemy-targeting status abilities (not Interact).
    const bool bStatusAbility =
        ArmedAbilityTag == FName("Ability.Debuff.Confuse") ||
        ArmedAbilityTag == FName("Ability.Debuff.Intimidate");
    const bool bTargetingFlow =
        TargetingPhase == ETargetingPhase::Targeting ||
        TargetingPhase == ETargetingPhase::Confirming;
    const bool bAurasActive = bPlayerTurn && bStatusAbility && bTargetingFlow;

    const float Radius = bAurasActive
        ? ControlledCharacter->GetAbilityRingRadius(ArmedAbilityTag) : 0.f;
    const FVector Origin = ControlledCharacter->GetActorLocation();
    const FGameplayTag ImmuneTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Status"));

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), Enemies);
    for (AActor* A : Enemies)
    {
        AEnemyCharacter* E = Cast<AEnemyCharacter>(A);
        if (!E) continue;

        ETargetAura State = ETargetAura::None;
        if (bAurasActive && E->IsAlive()
            && FVector::Dist(Origin, E->GetActorLocation()) <= Radius)
        {
            const bool bImmune = E->GetAbilitySystemComponent()
                && E->GetAbilitySystemComponent()->HasMatchingGameplayTag(ImmuneTag);
            State = bImmune ? ETargetAura::Immune : ETargetAura::Targetable;
        }
        E->SetTargetAura(State);
    }
}

ABaseCharacter* ATacticalPlayerController::GetNameCardUnit() const
{
    // Whoever's turn it is — ally or enemy. Attacker == current unit, so this
    // same slot serves both the idle name card AND the forecast's attacker.
    if (ControlledCharacter && ControlledCharacter->TurnManager)
    {
        ABaseCharacter* Active = ControlledCharacter->TurnManager->GetCurrentCombatant();
        if (Active && Active->IsAlive()) return Active;
    }
    return nullptr;
}

ABaseCharacter* ATacticalPlayerController::GetTargetCardUnit() const
{
    // A live forecast (player Confirming or enemy pre-strike) owns the right
    // card — it shows the ability's target.
    FAbilityForecast F;
    ABaseCharacter* Attacker = nullptr;
    ABaseCharacter* Target = nullptr;
    if (GetActiveForecast(F, Attacker, Target) && Target) return Target;

    // Otherwise the clicked unit — but not if it's the current-turn unit, which
    // the left name card already shows (don't draw the same unit twice).
    ABaseCharacter* Active = (ControlledCharacter && ControlledCharacter->TurnManager)
        ? ControlledCharacter->TurnManager->GetCurrentCombatant() : nullptr;
    if (InspectedUnit && InspectedUnit->IsAlive() && InspectedUnit != Active)
        return InspectedUnit;

    return nullptr;
}

void ATacticalPlayerController::ToggleInspect(ABaseCharacter* Unit)
{
    if (!Unit || !Unit->IsAlive()) return;

    if (AEnemyCharacter* E = Cast<AEnemyCharacter>(Unit))
    {
        // Enemy: the pin IS the toggle. Pinned → un-pin (zone off); else pin it on.
        if (PinnedEnemies.Contains(E))
        {
            PinnedEnemies.Remove(E);
            // Keep it lit only if the cursor is over it right now (hover owns it).
            if (E != HoveredEnemy) E->SetThreatRangeVisible(false);
            if (InspectedUnit == E) InspectedUnit = nullptr;   // card back to default
        }
        else
        {
            PinnedEnemies.Add(E);
            E->SetThreatRangeVisible(true);
            InspectedUnit = E;
        }
        return;
    }

    // Ally (no threat zone) — just toggle its card on/off.
    InspectedUnit = (InspectedUnit == Unit) ? nullptr : Unit;
}

void ATacticalPlayerController::ClearInspection()
{
    for (AEnemyCharacter* E : PinnedEnemies)
    {
        // Leave the hovered one lit — hover will manage it from here.
        if (E && E != HoveredEnemy) E->SetThreatRangeVisible(false);
    }
    PinnedEnemies.Empty();
    InspectedUnit = nullptr;
}

void ATacticalPlayerController::UpdateHoveredEnemy()
{
    AEnemyCharacter* NewHover = nullptr;

    // Only hover-threat in Idle — during ability targeting the ring + auras own
    // the board; a stray threat circle is just clutter.
    if (TargetingPhase == ETargetingPhase::Idle)
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Pawn, false, Hit))
        {
            if (AEnemyCharacter* E = Cast<AEnemyCharacter>(Hit.GetActor()))
            {
                if (E->IsAlive()) NewHover = E;
            }
        }
    }

    if (NewHover != HoveredEnemy)
    {
        // Don't hide the old one if it's pinned — the pin owns its zone now.
        if (HoveredEnemy && !PinnedEnemies.Contains(HoveredEnemy))
            HoveredEnemy->SetThreatRangeVisible(false);
        HoveredEnemy = NewHover;
        if (HoveredEnemy) HoveredEnemy->SetThreatRangeVisible(true);
    }
}

void ATacticalPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    UpdateHoveredEnemy();
    UpdateThreatLines();
}