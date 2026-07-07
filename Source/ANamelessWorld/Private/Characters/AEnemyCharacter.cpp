// AEnemyCharacter.cpp

#include "Characters/AEnemyCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"
#include "TimerManager.h"

AEnemyCharacter::AEnemyCharacter()
{
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Combat setup is handled by SetupCombat(), called from the Level Blueprint.
    // If a CharacterData asset is assigned, override the hardcoded HeavyStrikeChance.
    // This lets the Data Asset be the single source of truth for AI behaviour tuning.
    if (CharacterData)
    {
        HeavyStrikeChance = CharacterData->HeavyStrikeChance;
    }
}

void AEnemyCharacter::SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InPlayerTarget)
{
    TurnManager = InTurnManager;
    PlayerTarget = InPlayerTarget;

    // Subscribe to OnTurnStarted now that we have a valid TurnManager.
    TurnManager->OnTurnStarted.AddDynamic(this, &AEnemyCharacter::ExecuteAITurn);
}

void AEnemyCharacter::ExecuteAITurn(ABaseCharacter* ActiveCombatant)
{
    // Only act if it's THIS enemy's turn.
    if (ActiveCombatant != this) return;

    // Don't act instantly — the turn just started and the camera is only now
    // easing toward us. Wait a beat so it settles, then PerformAITurn() runs the
    // decision + attack and the animation actually gets seen.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TurnStartTimerHandle, this, &AEnemyCharacter::PerformAITurn,
            TurnStartDelay, false);
    }
}

void AEnemyCharacter::PerformAITurn()
{
    // Guard: need a living target.
    if (!PlayerTarget || !PlayerTarget->IsAlive()) return;

    UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s takes their turn."), *GetName());

    // ── Check State.Confused ───────────────────────────────────────────────
    // If Confuse was used on this enemy, they attack the NEAREST living
    // combatant (ally or boss, not necessarily the protagonist) with a
    // forced Heavy Strike (the damage buff) — GA_BasicAttack itself applies
    // the accuracy penalty (Disadvantage) automatically because State.Confused
    // is set on this ASC.
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
    const bool bIsConfused = MyASC && MyASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Confused")));

    if (bIsConfused)
    {
        ABaseCharacter* ConfusedTarget = FindNearestOtherCombatant();
        if (ConfusedTarget)
        {
            UE_LOG(LogTemp, Log,
                TEXT("AEnemyCharacter: %s is Confused — attacking nearest combatant %s with a forced Heavy Strike."),
                *GetName(), *ConfusedTarget->GetName());

            FGameplayEventData Payload;
            Payload.Target = ConfusedTarget;

            SetIsAttacking(true);
            UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                this, FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy")), Payload);
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("AEnemyCharacter: %s is Confused but no other living combatant to attack."),
                *GetName());
        }

        EndTurnAfterDelay();
        return;
    }

    // ── Normal turn: pick ability (Basic or Heavy) ─────────────────────────
    const int32 AbilityRoll = FMath::RandRange(0, 99);
    const bool bUseHeavy = (AbilityRoll < HeavyStrikeChance);

    FGameplayTag AttackTag = bUseHeavy
        ? FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy"))
        : FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Basic"));

    UE_LOG(LogTemp, Log,
        TEXT("AEnemyCharacter: %s chose %s (roll %d, threshold %d)."),
        *GetName(),
        bUseHeavy ? TEXT("Heavy Strike") : TEXT("Basic Attack"),
        AbilityRoll, HeavyStrikeChance);

    FGameplayEventData Payload;
    Payload.Target = PlayerTarget;

    SetIsAttacking(true);
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, AttackTag, Payload);

    EndTurnAfterDelay();
}

ABaseCharacter* AEnemyCharacter::FindNearestOtherCombatant() const
{
    if (!TurnManager) return nullptr;

    ABaseCharacter* Nearest = nullptr;
    float NearestDistSq = 0.f;

    for (ABaseCharacter* Combatant : TurnManager->GetTurnOrder())
    {
        if (!Combatant || Combatant == this || !Combatant->IsAlive()) continue;

        const float DistSq = FVector::DistSquared(GetActorLocation(), Combatant->GetActorLocation());
        if (!Nearest || DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            Nearest = Combatant;
        }
    }

    return Nearest;
}

void AEnemyCharacter::EndTurnAfterDelay()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TurnEndTimerHandle, this, &AEnemyCharacter::EndTurnNow, 2.0f, false);
    }
}

void AEnemyCharacter::EndTurnNow()
{
    SetIsAttacking(false);
    if (TurnManager)
    {
        TurnManager->EndTurn();
    }
}