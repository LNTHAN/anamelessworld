// AEnemyCharacter.cpp

#include "Characters/AEnemyCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"
#include "TurnManager/UTurnManager.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"
#include "TimerManager.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Camera/ATacticalPlayerController.h"
#include "Kismet/GameplayStatics.h"

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
    // Decide this turn's target + which attack to use, then engage.
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
    const bool bIsConfused = MyASC && MyASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Confused")));

    if (bIsConfused)
    {
        // Confused: strike the NEAREST living combatant (ally/boss/anyone) with a
        // forced Heavy Strike. GA_BasicAttack applies the Disadvantage itself.
        PendingTarget = FindNearestOtherCombatant();
        PendingAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy"));
        if (PendingTarget)
        {
            UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s is Confused — engaging nearest combatant %s."),
                *GetName(), *PendingTarget->GetName());
        }
    }
    else
    {
        // Normal: engage the player target, Basic/Heavy by roll.
        PendingTarget = PlayerTarget;
        const int32 AbilityRoll = FMath::RandRange(0, 99);
        const bool bUseHeavy = (AbilityRoll < HeavyStrikeChance);
        PendingAttackTag = bUseHeavy
            ? FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Heavy"))
            : FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Basic"));

        UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s chose %s (roll %d, threshold %d)."),
            *GetName(), bUseHeavy ? TEXT("Heavy Strike") : TEXT("Basic Attack"),
            AbilityRoll, HeavyStrikeChance);
    }

    // No living target — skip the turn cleanly (still end it, or the turn stalls).
    if (!PendingTarget || !PendingTarget->IsAlive())
    {
        UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s has no living target."), *GetName());
        EndTurnAfterDelay();
        return;
    }

    EngageTarget();
}

void AEnemyCharacter::EngageTarget()
{
    if (!PendingTarget) { EndTurnAfterDelay(); return; }

    const float Dist = FVector::Dist(GetActorLocation(), PendingTarget->GetActorLocation());
    if (Dist <= AttackRange)
    {
        // Already in reach — strike where we stand, no movement.
        AttackTarget();
    }
    else
    {
        // Out of reach — close the gap; the strike happens on arrival (Step 3).
        UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s target %.0f away (> %.0f) — moving in."),
            *GetName(), Dist, AttackRange);
        MoveTowardTarget();
    }
}

void AEnemyCharacter::AttackTarget()
{
    if (!PendingTarget || !PendingTarget->IsAlive()) { EndTurnAfterDelay(); return; }

    FaceActor(PendingTarget);   // turn to face before the preview + hit

    // Show the forecast (this enemy = attacker/left, PendingTarget = right),
    // then strike after a short beat so the player can read the incoming hit.
    if (ATacticalPlayerController* PC = Cast<ATacticalPlayerController>(
            UGameplayStatics::GetPlayerController(this, 0)))
    {
        PC->ShowAttackForecast(this, PendingTarget, PendingAttackTag.GetTagName());
    }

    GetWorldTimerManager().SetTimer(
        ForecastTimerHandle, this, &AEnemyCharacter::ExecuteStrike, ForecastDelay, false);
}

void AEnemyCharacter::ExecuteStrike()
{
    // Hide the forecast the instant the hit resolves.
    if (ATacticalPlayerController* PC = Cast<ATacticalPlayerController>(
            UGameplayStatics::GetPlayerController(this, 0)))
    {
        PC->HideAttackForecast();
    }

    if (!PendingTarget || !PendingTarget->IsAlive()) { EndTurnAfterDelay(); return; }

    FGameplayEventData Payload;
    Payload.Target = PendingTarget;

    SetIsAttacking(true);
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, PendingAttackTag, Payload);

    EndTurnAfterDelay();
}

// Walks along PathPoints and returns the location exactly MaxDistance into the
// route (or the final point if the path is shorter). Used to clamp an enemy's
// per-turn travel to its MoveRange.
static FVector PointAlongPath(const TArray<FVector>& PathPoints, float MaxDistance)
{
    if (PathPoints.Num() == 0) return FVector::ZeroVector;

    float Remaining = MaxDistance;
    FVector Prev = PathPoints[0];
    for (int32 i = 1; i < PathPoints.Num(); ++i)
    {
        const float Seg = FVector::Dist(Prev, PathPoints[i]);
        if (Seg >= Remaining)
        {
            return Prev + (PathPoints[i] - Prev).GetSafeNormal() * Remaining;
        }
        Remaining -= Seg;
        Prev = PathPoints[i];
    }
    return PathPoints.Last();   // budget exceeds the whole path — go to its end
}

void AEnemyCharacter::MoveTowardTarget()
{
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AEnemyCharacter: %s has no AIController — can't move. Set AutoPossessAI on its Blueprint."),
            *GetName());
        EndTurnAfterDelay();
        return;
    }

    // Get the actual walking route to the target (routed around walls/shelves).
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    UNavigationPath* Path = NavSys ? NavSys->FindPathToLocationSynchronously(
        GetWorld(), GetActorLocation(), PendingTarget->GetActorLocation(), this) : nullptr;

    if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("AEnemyCharacter: %s could not path to %s — ending turn."),
            *GetName(), *PendingTarget->GetName());
        EndTurnAfterDelay();
        return;
    }

    // Stop a margin INSIDE attack range, not at its edge. MoveToLocation halts
    // within its acceptance radius, so aiming for the exact 200-line leaves the
    // enemy tens of cm short and perpetually just-out-of-range. Half the range in
    // gives slack that the acceptance wobble can't eat.
    const float StopDistance = AttackRange * 0.5f;
    const float StopShortOfTarget = FMath::Max(0.f, Path->GetPathLength() - StopDistance);
    const float TravelDist = FMath::Min(MoveRange, StopShortOfTarget);

    FVector Dest = PointAlongPath(Path->PathPoints, TravelDist);

    // Snap onto the navmesh so we never aim just off it (grinds against walls).
    FNavLocation Projected;
    if (NavSys->ProjectPointToNavigation(Dest, Projected, FVector(150.f, 150.f, 300.f)))
    {
        Dest = Projected.Location;
    }
    
    if (!AICon->ReceiveMoveCompleted.IsAlreadyBound(this, &AEnemyCharacter::OnMoveCompleted))
    {
        AICon->ReceiveMoveCompleted.AddDynamic(this, &AEnemyCharacter::OnMoveCompleted);
    }

    const EPathFollowingRequestResult::Type MoveResult = AICon->MoveToLocation(Dest, 50.f);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        EndTurnAfterDelay();
    }
    else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        // Reached the clamped point instantly — run the same arrival check.
        OnMoveCompleted(FAIRequestID::CurrentRequest, EPathFollowingResult::Success);
    }
    // RequestSuccessful: the walk is underway; OnMoveCompleted finishes the turn.
}

void AEnemyCharacter::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    // Resolve once — stop listening so a stray second broadcast can't run the
    // attack (or end-turn) twice. MoveTowardTarget rebinds next turn.
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->ReceiveMoveCompleted.RemoveDynamic(this, &AEnemyCharacter::OnMoveCompleted);
    }
        
    // Walk finished (arrived, blocked, or aborted). Re-check range: if we made it
    // into striking distance, attack; otherwise the turn's just over.
    if (PendingTarget && PendingTarget->IsAlive()
        && FVector::Dist(GetActorLocation(), PendingTarget->GetActorLocation()) <= AttackRange)
    {
        UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s reached %s — striking."),
            *GetName(), *PendingTarget->GetName());
        AttackTarget();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AEnemyCharacter: %s ended movement out of range — turn over."),
            *GetName());
        EndTurnAfterDelay();
    }
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

    // Confusion wears off after the enemy has taken its confused turn (its attack
    // already fired with Disadvantage before this). No-op if it wasn't confused.
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayTagContainer ConfusedTag(FGameplayTag::RequestGameplayTag(FName("State.Confused")));
        ASC->RemoveActiveEffectsWithGrantedTags(ConfusedTag);
    }

    if (TurnManager)
    {
        TurnManager->EndTurn();
    }
}