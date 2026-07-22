// AEnemyCharacter.h
// PURPOSE: AI-controlled enemy. Inherits GAS from ABaseCharacter.
//          On its turn, automatically attacks the player.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ABaseCharacter.h"
#include "TurnManager/UTurnManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "AEnemyCharacter.generated.h"

class UDecalComponent;
class UMaterialInterface;
class USplineMeshComponent;
class UMaterialInstanceDynamic; 

UENUM(BlueprintType)
enum class ETargetAura : uint8
{
    None,
    Targetable,   // affectable by the armed ability → red
    Immune        // in range but immune → grey
};

UCLASS()
class ANAMELESSWORLD_API AEnemyCharacter : public ABaseCharacter
{
    GENERATED_BODY()

public:

    AEnemyCharacter();

    virtual void BeginPlay() override;

    // Called by UTurnManager (via OnTurnStarted delegate) when it's this enemy's turn.
    UFUNCTION()
    void ExecuteAITurn(ABaseCharacter* ActiveCombatant);

    // Called from the Level Blueprint to wire up combat references.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    virtual void SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InPlayerTarget) 
    override;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    UTurnManager* TurnManager;

    // The player character to attack. Set in the editor.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ANW|Combat")
    ABaseCharacter* PlayerTarget;

    // 0 = never use heavy strike, 100 = always use heavy strike.
    // Default 30 = 30% chance per turn. Override per-instance in the editor.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ANW|Combat")
    int32 HeavyStrikeChance = 30;

    // How close (cm) this enemy must be to its target to attack. Inside this
    // range it strikes; outside it, it spends the turn moving closer. ~200 ≈
    // melee reach. EditAnywhere so ranged enemies later can set it large.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Combat")
    float AttackRange = 200.f;

    // Intermission at the start of this enemy's turn, in seconds, before it acts.
    // Lets the camera finish focusing so the attack animation is actually seen.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Combat")
    float TurnStartDelay = 1.5f;    

    // How long the forecast panel shows before this enemy's hit lands, in seconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Combat")
    float ForecastDelay = 1.2f;

    // Red threat zone = how far this enemy can move AND still strike this turn
    // (radius ≈ MoveRange + AttackRange). Shown to the player on hover.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|UI")
    UDecalComponent* ThreatRangeDecal;

    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetThreatRangeVisible(bool bVisible);

    // Whole-body aura overlays (red = affectable, grey = immune). Set in BP.
    UPROPERTY(EditDefaultsOnly, Category = "ANW|UI")
    UMaterialInterface* AuraTargetMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "ANW|UI")
    UMaterialInterface* AuraImmuneMaterial;

    // Applies (or clears, with None) the target-aura overlay on this enemy's mesh.
    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetTargetAura(ETargetAura State);

    // The unit this enemy will attack next: nearest combatant if Confused, else its
    // ally target. Today PlayerTarget = Nameless; when a party exists this becomes
    // "nearest reachable ally" — ONLY this line changes, not the threat-line system.
    UFUNCTION(BlueprintCallable, Category = "ANW|AI")
    ABaseCharacter* GetIntendedTarget() const;

    // True while State.Confused is on this enemy (drives the yellow line).
    UFUNCTION(BlueprintCallable, Category = "ANW|AI")
    bool IsConfused() const;

    // Move+attack reach — within this radius its target gets struck this turn.
    UFUNCTION(BlueprintCallable, Category = "ANW|AI")
    float GetThreatRadius() const { return MoveRange + AttackRange; }

    // Draws/updates this enemy's arced intent line to Target (colour + opacity), or
    // hides it. Driven every frame by the controller.
    UFUNCTION(BlueprintCallable, Category = "ANW|UI")
    void SetThreatLine(bool bShow, AActor* Target, FLinearColor Color, float Opacity);

private:
    FTimerHandle TurnEndTimerHandle;

    void EndTurnAfterDelay();
    void EndTurnNow();

    FTimerHandle TurnStartTimerHandle;

    // The real turn logic (decide + attack), run after the TurnStartDelay beat.
    void PerformAITurn();

    // The target chosen this turn, and the attack to use once in range. Held as
    // members because the move is async — PerformAITurn decides them, but the
    // attack happens later, in OnMoveCompleted, after the walk finishes.
    UPROPERTY()
    ABaseCharacter* PendingTarget = nullptr;
    FGameplayTag PendingAttackTag;

    // Decide attack-or-move for PendingTarget: in range → strike now; else walk.
    void EngageTarget();

    // Fire PendingAttackTag at PendingTarget, then end the turn after the beat.
    void AttackTarget();

    // Runs after ForecastDelay: hides the forecast and fires the actual strike.
    void ExecuteStrike();

    FTimerHandle ForecastTimerHandle;

    // Ask the AIController to path toward PendingTarget, stopping at AttackRange.
    void MoveTowardTarget();

    // AIController's arrival callback — bound to ReceiveMoveCompleted. The walk
    // is done, so re-check range and either strike or end the turn.
    UFUNCTION()
    void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

    // Finds the nearest OTHER living combatant to this one (via TurnManager's
    // roster) — Confuse's targeting rule: attack whoever's closest, not
    // necessarily the protagonist.
    ABaseCharacter* FindNearestOtherCombatant() const;

protected:
    // Arced ground ribbon showing who this enemy will attack (red = natural, yellow = confused).
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|UI")
    USplineMeshComponent* ThreatLineMesh;

    // Base line material (params: LineColor vector, LineOpacity scalar). Set in BP.
    UPROPERTY(EditDefaultsOnly, Category = "ANW|UI")
    UMaterialInterface* ThreatLineMaterial;

    UPROPERTY()
    UMaterialInstanceDynamic* ThreatLineMID;

    // Line look — tune live in the editor after building.
    UPROPERTY(EditAnywhere, Category = "ANW|UI")
    float LineThickness = 0.06f;   // spline cross-section scale (~6uu — thinner = more "line")

    UPROPERTY(EditAnywhere, Category = "ANW|UI")
    float LineHeight = 50.f;        // height above each unit's centre (neck-ish, not the belt)
};