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
#include "Interactables/AInteractableActor.h"
#include "Components/DecalComponent.h"
#include "Abilities/GA_Intimidate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlayerCharacter::APlayerCharacter()
{
    DialogueComp = CreateDefaultSubobject<UDialogueComponent>(TEXT("Dialogue"));

    MoveReachISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MoveReachISM"));
    MoveReachISM->SetupAttachment(RootComponent);
    // Absolute so the tiles stay put in the world — they must NOT follow or rotate
    // with Nameless (he faces targets, and moving hides them anyway).
    MoveReachISM->SetUsingAbsoluteLocation(true);
    MoveReachISM->SetUsingAbsoluteRotation(true);
    MoveReachISM->SetUsingAbsoluteScale(true);
    MoveReachISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MoveReachISM->SetCanEverAffectNavigation(false);
    MoveReachISM->SetVisibility(false);

    // Engine unit plane (100×100uu, faces up) as the tile mesh; material set in BP.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMesh.Succeeded()) MoveReachISM->SetStaticMesh(PlaneMesh.Object);

    AbilityRangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("AbilityRangeDecal"));
    AbilityRangeDecal->SetupAttachment(RootComponent);
    AbilityRangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    AbilityRangeDecal->SetVisibility(false);
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

bool APlayerCharacter::TryInteract(AInteractableActor* Target)
{
    if (!Target) return false;

    // Same turn gate as every other action.
    if (!TurnManager || TurnManager->GetCurrentState() != ETurnState::PlayerTurn)
        return false;

    // Action economy: interacting IS your action for the turn.
    if (!bActionAvailable)
    {
        UE_LOG(LogTemp, Log, TEXT("APlayerCharacter: no action left to interact."));
        return false;
    }

    // Must be within reach to rig it. Straight-line distance is fine — the shelf
    // is right there; this isn't a pathed move.
    const float Dist = Target->GetDistanceToBody(GetActorLocation());    
    
    if (Dist > InteractRange)
    {
        UE_LOG(LogTemp, Log,
            TEXT("APlayerCharacter: too far to interact (%.0f > %.0f)."),
            Dist, InteractRange);
        return false;   // stay armed — player can walk closer and click again
    }

    bActionAvailable = false;   // spend the Action stock
    Target->Arm();              // stage 1 — the shelf is now live
    return true;
}

void APlayerCharacter::SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InTarget)
{
    Super::SetupCombat(InTurnManager, InTarget);  // caches CachedTurnManager → Die() reports instantly
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

    FaceActor(CurrentTarget);  
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
    }
    // Basic Attack retired — Nameless deals no direct damage (DESIGN_RATIONALE §6).
    // GA_BasicAttack itself stays: enemies still use it to attack Nameless.
}

bool APlayerCharacter::IsDialogueActive() const
{
    return DialogueComp && DialogueComp->IsDialogueActive();
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

void APlayerCharacter::SetMoveRangeVisible(bool bVisible)
{
    if (!MoveReachISM) return;

    if (bVisible)
    {
        if (!MoveReachISM->IsVisible())   // recompute only on hidden → shown
        {
            ComputeMoveReachable();
            MoveReachISM->SetVisibility(true);
        }
    }
    else
    {
        MoveReachISM->SetVisibility(false);
        MoveReachISM->ClearInstances();
    }
}

void APlayerCharacter::ComputeMoveReachable()
{
    if (!MoveReachISM) return;
    MoveReachISM->ClearInstances();

    UWorld* World = GetWorld();
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav) return;

    const FVector Origin = GetActorLocation();
    const float Step = 50.f;                       // grid spacing (cm) — tune for density
    const int32 Cells = FMath::CeilToInt(MoveRange / Step);
    const float TileScale = Step / 100.f;          // engine Plane is 100uu

    for (int32 X = -Cells; X <= Cells; ++X)
    {
        for (int32 Y = -Cells; Y <= Cells; ++Y)
        {
            const FVector Sample = Origin + FVector(X * Step, Y * Step, 0.f);

            // Cheap straight-line cull (skips the square's corners past the radius).
            if (FVector::Dist2D(Origin, Sample) > MoveRange) continue;

            // Is this spot actually on the navmesh? (excludes walls / gaps / shelves)
            FNavLocation NavLoc;
            if (!Nav->ProjectPointToNavigation(Sample, NavLoc,
                    FVector(Step * 0.5f, Step * 0.5f, 200.f)))
                continue;

            // Terrain-exact part: real path distance must fit the move budget.
            UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
                World, Origin, NavLoc.Location);
            if (!Path || !Path->IsValid() || Path->IsPartial()) continue;
            if (Path->GetPathLength() > MoveRange) continue;

            // Reachable → drop a tile just above the floor (world-space transform).
            const FTransform T(FRotator::ZeroRotator,
                NavLoc.Location + FVector(0.f, 0.f, 2.f),
                FVector(TileScale, TileScale, 1.f));
            MoveReachISM->AddInstance(T, /*bWorldSpace=*/true);
        }
    }
}

void APlayerCharacter::SetAbilityRangeVisible(bool bVisible)
{
    if (AbilityRangeDecal) AbilityRangeDecal->SetVisibility(bVisible);
}

void APlayerCharacter::SetAbilityRingRadius(float Radius)
{
    if (AbilityRangeDecal)
    {
        AbilityRangeDecal->DecalSize = FVector(512.f, Radius, Radius);
        AbilityRangeDecal->MarkRenderStateDirty();
    }
}

float APlayerCharacter::GetAbilityRingRadius(FName Tag) const
{
    // Confuse: same cast-range the targeting gate already enforces.
    if (Tag == FName("Ability.Debuff.Confuse"))
    {
        return ConfuseCastRange;
    }

    // Intimidate: read the real AoE radius off the granted ability — no duplicate.
    if (Tag == FName("Ability.Debuff.Intimidate"))
    {
        if (AbilitySystemComponent)
        {
            for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
            {
                if (const UGA_Intimidate* Intim = Cast<UGA_Intimidate>(Spec.Ability))
                {
                    return Intim->GetIntimidateRadius();
                }
            }
        }
    }

    // Interact: how close Nameless must stand to rig a shelf.
    if (Tag == FName("Action.Interact"))
    {
        return InteractRange;
    }

    return 0.f;   // no ring for this ability (e.g. Interact)
}
