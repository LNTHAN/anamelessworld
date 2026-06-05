// AEnemyCharacter.cpp

#include "Characters/AEnemyCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"

AEnemyCharacter::AEnemyCharacter()
{
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Combat setup is handled by SetupCombat(), called from the Level Blueprint.
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

    // Build the attack payload with the player as target.
    FGameplayEventData Payload;
    Payload.Target = PlayerTarget;

    // Pick an ability based on HeavyStrikeChance (0–100).
    // FMath::RandRange(0, 99) gives a uniform roll — if it falls below
    // HeavyStrikeChance, the enemy uses the heavy attack instead.
    const int32 Roll = FMath::RandRange(0, 99);
    const bool bUseHeavy = (Roll < HeavyStrikeChance);

    FGameplayTag AttackTag = bUseHeavy
        ? FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy"))
        : FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Basic"));

    UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s chose %s (roll %d, threshold %d)."),
        *GetName(),
        bUseHeavy ? TEXT("Heavy Strike") : TEXT("Basic Attack"),
        Roll, HeavyStrikeChance);

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, AttackTag, Payload);

    // End this enemy's turn.
    if (TurnManager)
    {
        TurnManager->EndTurn();
    }
}