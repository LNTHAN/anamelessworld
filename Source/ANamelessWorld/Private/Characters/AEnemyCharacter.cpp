// AEnemyCharacter.cpp

#include "Characters/AEnemyCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"

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

    // Guard: need a living target.
    if (!PlayerTarget || !PlayerTarget->IsAlive()) return;

    UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s takes their turn."), *GetName());

    // ── Check State.Enraged ────────────────────────────────────────────────
    // If Provoke was used on this enemy, they must attack the protagonist
    // with Disadvantage. If they miss, they damage themselves.
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
    const bool bIsEnraged = MyASC && MyASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Enraged")));

    // ── Pick ability (Basic or Heavy) ──────────────────────────────────────
    const int32 AbilityRoll = FMath::RandRange(0, 99);
    const bool bUseHeavy = (AbilityRoll < HeavyStrikeChance);

    FGameplayTag AttackTag = bUseHeavy
        ? FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy"))
        : FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Basic"));

    UE_LOG(LogTemp, Log,
        TEXT("AEnemyCharacter: %s chose %s (roll %d, threshold %d).%s"),
        *GetName(),
        bUseHeavy ? TEXT("Heavy Strike") : TEXT("Basic Attack"),
        AbilityRoll, HeavyStrikeChance,
        bIsEnraged ? TEXT(" [Enraged — rolling with Disadvantage]") : TEXT(""));

    // ── Enraged: resolve hit/miss here, self-damage on miss ───────────────
    if (bIsEnraged)
    {
        const UCRPGAttributeSet* MyAttributes = Cast<UCRPGAttributeSet>(
            MyASC->GetAttributeSet(UCRPGAttributeSet::StaticClass()));

        UAbilitySystemComponent* TargetASC = PlayerTarget->GetAbilitySystemComponent();
        const UCRPGAttributeSet* TargetAttributes = TargetASC
            ? Cast<UCRPGAttributeSet>(TargetASC->GetAttributeSet(
                UCRPGAttributeSet::StaticClass()))
            : nullptr;

        if (MyAttributes && TargetAttributes)
        {
            // Roll with Disadvantage — Enraged enemies are reckless and off-balance.
            const int32 RawRoll = UCRPGCombatLibrary::RollWithDisadvantage();
            const int32 STRModifier = UCRPGCombatLibrary::GetModifier(
                MyAttributes->GetStrength());
            const int32 TargetAC = UCRPGCombatLibrary::CalculateAC(
                TargetAttributes->GetDexterity());
            const int32 FinalRoll = RawRoll + STRModifier;

            const FString MyName = GetName();
            UE_LOG(LogTemp, Log,
                TEXT("AEnemyCharacter: %s enraged roll %d + %d (STR) = %d vs AC %d."),
                *MyName, RawRoll, STRModifier, FinalRoll, TargetAC);

            if (FinalRoll < TargetAC)
            {
                // Miss — the enraged enemy damages themselves instead.
                // Fire the ability with SELF as the target so GA_BasicAttack
                // applies the damage effect to this enemy's own ASC.
                UE_LOG(LogTemp, Log,
                    TEXT("AEnemyCharacter: %s missed while Enraged — damages self!"),
                    *MyName);

                FGameplayEventData SelfPayload;
                SelfPayload.Target = this;

                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                    this, AttackTag, SelfPayload);

                if (TurnManager) TurnManager->EndTurn();
                return;
            }

            // Hit despite Disadvantage — fall through to normal attack below.
            UE_LOG(LogTemp, Log,
                TEXT("AEnemyCharacter: %s hit despite being Enraged."), *MyName);
        }
    }

    // ── Normal attack — fire ability at the player ─────────────────────────
    FGameplayEventData Payload;
    Payload.Target = PlayerTarget;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, AttackTag, Payload);

    if (TurnManager)
    {
        TurnManager->EndTurn();
    }
}