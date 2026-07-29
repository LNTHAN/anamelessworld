// ATacticalPlayerController.h
// PURPOSE: The tactical "brain". It drives the camera rig, but keeps a
//          reference to Nameless so mouse clicks and keys still command him.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ATacticalPlayerController.generated.h"

class APlayerCharacter;
class ABaseCharacter;
class AEnemyCharacter;

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

// The middle-arrow contents for the staged action. Card fields (portrait, name,
// HP, MP, Lv, EXP) bind off ControlledCharacter / PendingTarget in UMG — this
// struct is ONLY the two %s, the damage, and the status name.
USTRUCT(BlueprintType)
struct FAbilityForecast
{
    GENERATED_BODY()

    // False when nothing is staged — UMG uses this to hide the panel.
    UPROPERTY(BlueprintReadOnly) bool bValid = false;

    // Row 1: chance it connects, and chance it inflicts its status (0–100).
    UPROPERTY(BlueprintReadOnly) int32 HitChance = 100;
    UPROPERTY(BlueprintReadOnly) int32 InflictChance = 0;

    // Row 2: direct damage. bDealsDamage = false → UMG shows "--" (Nameless).
    UPROPERTY(BlueprintReadOnly) int32 Damage = 0;
    UPROPERTY(BlueprintReadOnly) bool bDealsDamage = false;

    // Row 3: the status inflicted, e.g. "Confused". Empty = none.
    UPROPERTY(BlueprintReadOnly) FText StatusName;

    // The ability's display name for the arrow header, e.g. "Confuse".
    UPROPERTY(BlueprintReadOnly) FText AbilityName;
};

UCLASS()
class ANAMELESSWORLD_API ATacticalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    // Per-frame: track which enemy the cursor is over, to toggle its threat zone.
    virtual void PlayerTick(float DeltaTime) override;

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

    // The unit whose detail card the inspect panel shows. Null = fall back to
    // the active ally (its card auto-shows on its turn). Set by clicking a unit.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    ABaseCharacter* InspectedUnit = nullptr;

    // LEFT "name card": the current-turn unit (ally OR enemy). The panel polls
    // this every frame. Null → hide.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    ABaseCharacter* GetNameCardUnit() const;

    // RIGHT card: the ability TARGET while any forecast is up; otherwise the
    // clicked unit (unless it's already the current-turn unit). Null → hide.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    ABaseCharacter* GetTargetCardUnit() const;

    // Arms an ability: the next left-click will target it instead of moving.
    // Called by command-menu buttons and by the 1/2/3 hotkeys. Refuses outside
    // PlayerTurn or with no Action left — same gate FireAbility checks anyway,
    // but failing here gives the player instant feedback instead of a dead click.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")

    void ArmAbility(FName TagName);

    // Builds the forecast for whatever's staged. Invalid (bValid=false) unless
    // we're Confirming. UMG calls this to fill the arrow.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    FAbilityForecast GetPendingForecast() const;

    // Attacker/target for an enemy's pre-strike forecast. The widget reads these
    // (via GetActiveForecast) so one panel serves both player and enemy previews.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    ABaseCharacter* ForecastAttacker = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Combat")
    ABaseCharacter* ForecastTarget = nullptr;

    // Called by AEnemyCharacter just before it strikes: builds + shows the forecast.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void ShowAttackForecast(ABaseCharacter* Attacker, ABaseCharacter* Target, FName AttackTag);

    // Clears the enemy forecast (called right before the hit lands).
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void HideAttackForecast();

    // Unified read for the forecast widget. Returns true if ANY forecast is live
    // (player Confirming OR an enemy pre-strike) and fills attacker/target/data.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    bool GetActiveForecast(
    FAbilityForecast& OutForecast,
    ABaseCharacter*& OutAttacker,
    ABaseCharacter*& OutTarget) const;

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

    // Eases the camera onto Nameless for the result-screen reveal, called from
    // WBP_GameResult when the dissolve completes. Clears any follow target so the
    // walk-trailing logic doesn't fight the focus.
    UFUNCTION(BlueprintCallable, Category = "ANW|Camera")
    void FocusOnPlayerForResult();

    // Eases the camera onto any unit. Used by the turn-order strip: clicking a
    // slot jumps the view to that combatant. Clears the follow target so the
    // walk-trailing logic doesn't drag the camera back off them.
    UFUNCTION(BlueprintCallable, Category = "ANW|Camera")
    void FocusOnUnit(ABaseCharacter* Unit);
    
private:

    // Shows the right ally indicator for the current turn + targeting state.
    void UpdateRangeIndicators();

    // Draws every enemy's intent line each frame (red natural / yellow confused).
    void UpdateThreatLines();

    // Toggles the hovered enemy's threat zone.
    void UpdateHoveredEnemy();
    AEnemyCharacter* HoveredEnemy = nullptr;

    // Click-to-inspect: pin a unit's card (+ an enemy's threat zone). Clicking the
    // same enemy again un-pins it; clicking a fresh unit swaps the card in.
    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void ToggleInspect(ABaseCharacter* Unit);

    // Drops every pinned zone + clears the inspected unit (move-click / turn-start).
    void ClearInspection();

    // Enemies whose threat zone is PINNED on (survives the cursor leaving them).
    // Distinct from HoveredEnemy — the single, transient mouse-over zone.
    UPROPERTY()
    TSet<AEnemyCharacter*> PinnedEnemies;

    // Sets red/grey/none auras on enemies for the armed status ability.
    void UpdateTargetAuras();
    
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

    // True while an enemy pre-strike forecast is showing.
    bool bExternalForecastActive = false;

    // The enemy forecast data, built in ShowAttackForecast.
    FAbilityForecast ExternalForecast;

    // Shared forecast math for both player and enemy. One place = they can't drift.
    FAbilityForecast BuildForecast(
    ABaseCharacter* Attacker,
    ABaseCharacter* Target,
    FName AbilityTag) const;
};