// UTurnManager.cpp
// PROJECT: A Nameless World
// PURPOSE: Implementation of the combat turn referee.
//          Read UTurnManager.h first — all the "why" comments live there.

#include "TurnManager/UTurnManager.h"
#include "Characters/ABaseCharacter.h"
#include "AbilitySystemComponent.h"  
// Full include here (not just forward declaration) because in this file
// we actually CALL functions on ABaseCharacter — GetDexterity(), IsAlive(), etc.
// The .h only stored a pointer, so a forward declaration was enough there.

void UTurnManager::AddCombatant(ABaseCharacter* NewCombatant)
{
    // Defensive check: never add a nullptr to the roster.
    // If someone accidentally calls AddCombatant(nullptr), this stops
    // a crash later when we try to call functions on that null pointer.
    if (NewCombatant == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("UTurnManager::AddCombatant — tried to add a nullptr. Ignored."));
        return;
    }

    // Defensive check: don't add the same character twice.
    // Contains() scans the array and returns true if the pointer is already in it.
    if (Combatants.Contains(NewCombatant))
    {
        UE_LOG(LogTemp, Warning, TEXT("UTurnManager::AddCombatant — %s is already in the roster. Ignored."),
               *NewCombatant->GetName());
        return;
    }

    // All clear — add them to the roster.
    // Order doesn't matter here; StartCombat() will sort by initiative.
    Combatants.Add(NewCombatant);

    UE_LOG(LogTemp, Log, TEXT("UTurnManager::AddCombatant — %s joined the fight. Roster size: %d"),
           *NewCombatant->GetName(), Combatants.Num());
}

void UTurnManager::StartCombat()
{
    // Guard: need at least 2 combatants for a fight to make sense.
    if (Combatants.Num() < 2)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UTurnManager::StartCombat — not enough combatants (%d). Need at least 2."),
            Combatants.Num());
        return;
    }

    // Set round number to 1 — combat has officially begun.
    RoundNumber = 1;

    // Roll initiative for everyone and sort the Combatants array.
    // After this line, Combatants[0] is the fastest character in the fight.
    SortByInitiative();

    // Start at the first slot.
    CurrentTurnIndex = 0;

    // Fire the round-started delegate so any UI listening can show "Round 1".
    OnRoundStarted.Broadcast(RoundNumber);

    UE_LOG(LogTemp, Log, TEXT("UTurnManager::StartCombat — Round %d begins. %d combatants."),
        RoundNumber, Combatants.Num());

    // Begin the first turn — sets the state and fires OnTurnStarted.
    // This must be last: by the time BeginTurn() fires its delegate,
    // everything above (round number, sorted order, index) must already be set.
    BeginTurn();
}

void UTurnManager::EndTurn()
{
    // Guard: can't end a turn if combat hasn't started.
    if (CurrentState == ETurnState::WaitingToStart ||
        CurrentState == ETurnState::GameOver)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UTurnManager::EndTurn — %s finished their turn."),
        *GetCurrentCombatant()->GetName());

    // ── Step 1: Advance the index FIRST (before removing anyone) ─────────────
    // We must use the current array size here. If we removed dead characters
    // first, the array would be smaller and the modulo would give a wrong result.
    int32 PreviousIndex = CurrentTurnIndex;
    CurrentTurnIndex = (CurrentTurnIndex + 1) % Combatants.Num();

    // ── Step 2: Check if the index wrapped — if so, a new round just started ─
    if (CurrentTurnIndex <= PreviousIndex)
    {
        RoundNumber++;
        UE_LOG(LogTemp, Log, TEXT("UTurnManager: Round %d begins."), RoundNumber);
        OnRoundStarted.Broadcast(RoundNumber);
    }

    // ── Step 3: Check if combat is over ──────────────────────────────────────
    if (CheckCombatOver())
    {
        CurrentState = ETurnState::GameOver;

        // Was it the player or the enemies who survived?
        bool bPlayerWon = Combatants.ContainsByPredicate([](const ABaseCharacter* C)
        {
            return C && C->IsAlive() && C->IsPlayerCharacter();
        });

        UE_LOG(LogTemp, Log, TEXT("UTurnManager: Combat over. Player won: %s"),
            bPlayerWon ? TEXT("YES") : TEXT("NO"));

        OnCombatEnded.Broadcast(bPlayerWon);
        return; // Stop here — no next turn to begin.
    }

    // ── Step 4: Remove dead combatants (adjusts index if needed) ─────────────
    RemoveDeadCombatants();

    // ── Step 5: Begin the next turn ───────────────────────────────────────────
    BeginTurn();
}

// ─────────────────────────────────────────────────────────────────────────────
// GETTERS — read-only queries, no state changes (all marked const in the .h)
// ─────────────────────────────────────────────────────────────────────────────
ABaseCharacter* UTurnManager::GetCurrentCombatant() const
{
    // Guard: if nobody has been added yet, or combat hasn't started, return nothing.
    // Callers (HUD, camera) must always check for nullptr before using this.
    if (Combatants.Num() == 0)
    {
        return nullptr;
    }

    // CurrentTurnIndex is kept in bounds by the modulo in EndTurn(),
    // but we double-check here defensively. IsValidIndex returns false
    // if the index is negative or >= array size.
    if (!Combatants.IsValidIndex(CurrentTurnIndex))
    {
        return nullptr;
    }

    // All checks passed — return whoever is currently acting.
    return Combatants[CurrentTurnIndex];
}

ETurnState UTurnManager::GetCurrentState() const
{
    // Simply expose the private CurrentState to outside systems.
    // Outside systems can READ this but never WRITE it directly —
    // only BeginTurn() and EndTurn() are allowed to change it.
    return CurrentState;
}

int32 UTurnManager::GetRoundNumber() const
{
    // Simply expose the private RoundNumber.
    // Starts at 0 before combat begins, becomes 1 when StartCombat() is called.
    return RoundNumber;
}

TArray<ABaseCharacter*> UTurnManager::GetTurnOrder() const
{
    TArray<ABaseCharacter*> Ordered;
    for (int32 i = CurrentTurnIndex; i < Combatants.Num(); i++)
        Ordered.Add(Combatants[i]);
    for (int32 i = 0; i < CurrentTurnIndex; i++)
        Ordered.Add(Combatants[i]);
    return Ordered;
}

int32 UTurnManager::RollInitiative(ABaseCharacter* Character) const
{
    // Roll a random number between 1 and 20 (the d20).
    // FMath::RandRange is inclusive on both ends — RandRange(1, 20) can return 1 or 20.
    int32 Roll = FMath::RandRange(1, 20);

    // Get the raw Dexterity score and compute the modifier.
    // Formula: floor((Dexterity - 10) / 2)
    // FMath::FloorToInt rounds DOWN — important for negative modifiers.
    // e.g. Dex 9: (9-10)/2 = -0.5 → floor = -1 (not 0)
    float Dex = Character->GetDexterity();
    int32 Modifier = FMath::FloorToInt((Dex - 10.0f) / 2.0f);

    UE_LOG(LogTemp, Log,
        TEXT("%s initiative: rolled %d + modifier %d = %d"),
        *Character->GetName(), Roll, Modifier, Roll + Modifier);

    return Roll + Modifier;
}

void UTurnManager::SortByInitiative()
{
    // Step 1: Roll initiative for every combatant and store the scores.
    // TMap<Key, Value> — like a dictionary. Here: character pointer → their roll.
    // We store scores first so each character rolls exactly once.
    // (If we rolled inside the sort comparator, the same character could roll
    // multiple times and get different numbers — the sort would be meaningless.)
    TMap<ABaseCharacter*, int32> InitiativeScores;

    for (ABaseCharacter* Character : Combatants)
    {
        if (Character && Character->IsAlive())
        {
            int32 Score = RollInitiative(Character);
            InitiativeScores.Add(Character, Score);
        }
    }

    // Step 2: Sort the Combatants array using those stored scores.
    // Combatants.Sort() rearranges the array in-place.
    // It needs a COMPARATOR — a rule that says "should A come before B?"
    // We give it a LAMBDA: a small anonymous function written right here inline.
    //
    // Lambda anatomy:
    //   [&InitiativeScores]   — "capture": borrow access to the scores map from outside
    //   (A, B)                — the two elements being compared
    //   { return A > B; }     — return true if A should come BEFORE B
    //
    // Returning ScoreA > ScoreB means: higher score → earlier in the array → acts first.
    // NOTE: TArray<ABaseCharacter*>::Sort dereferences pointers before passing to
    // the comparator — so parameters are references (ABaseCharacter&), not pointers.
    // We use &A and &B to get the pointer back for the TMap lookup.
    Combatants.Sort([&InitiativeScores](const ABaseCharacter& A, const ABaseCharacter& B)
    {
        // FindRef returns the stored value, or 0 if the key isn't found (safe default).
        int32 ScoreA = InitiativeScores.FindRef(&A);
        int32 ScoreB = InitiativeScores.FindRef(&B);
        return ScoreA > ScoreB; // Higher score = goes first (lower index in array)
    });

    // Log the final order so we can verify it in the Output Log during testing.
    UE_LOG(LogTemp, Log, TEXT("UTurnManager: Initiative order set —"));
    for (int32 i = 0; i < Combatants.Num(); i++)
    {
        if (Combatants[i])
        {
            UE_LOG(LogTemp, Log, TEXT("  [%d] %s (score: %d)"),
                i,
                *Combatants[i]->GetName(),
                InitiativeScores.FindRef(Combatants[i]));  // pointer key — correct here
        }
    }
}

void UTurnManager::BeginTurn()
{
    // Safety check — should never be empty here, but defensive programming.
    if (!Combatants.IsValidIndex(CurrentTurnIndex))
    {
        UE_LOG(LogTemp, Error,
            TEXT("UTurnManager::BeginTurn — CurrentTurnIndex %d is out of bounds."),
            CurrentTurnIndex);
        return;
    }

    ABaseCharacter* ActiveCharacter = Combatants[CurrentTurnIndex];
    if (ActiveCharacter == nullptr) return;

    // Fresh turn → refill this combatant's Move + Action stocks.
    ActiveCharacter->ResetTurnResources();

    // Nav-avoidance: everyone carves the navmesh so others path AROUND them —
    // EXCEPT whoever's turn it is, so the mover doesn't route around (or churn
    // on) its own footprint. The change settles during the turn intermission /
    // the player's think-time, before anyone actually moves.
    for (ABaseCharacter* Combatant : Combatants)
    {
        if (Combatant) Combatant->SetNavObstacleEnabled(true);
    }
    ActiveCharacter->SetNavObstacleEnabled(false);

    // Determine whose turn it is.
    // APlayerCharacter will override a function we check here — for now we use
    // a simple tag check. IsPlayerControlled() returns true for the player pawn.
    if (ActiveCharacter->IsPlayerCharacter())
    {
        CurrentState = ETurnState::PlayerTurn;
        UE_LOG(LogTemp, Log, TEXT("UTurnManager: PLAYER TURN — %s"), *ActiveCharacter->GetName());
    }
    else
    {
        CurrentState = ETurnState::EnemyTurn;
        UE_LOG(LogTemp, Log, TEXT("UTurnManager: ENEMY TURN — %s"), *ActiveCharacter->GetName());
    }

    // Check if the current combatant has the State.Stunned tag.
    // If so, skip their turn and advance to the next combatant.
    // State.Stunned is applied by GA_Intimidate for 1 turn.
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (ASC && ASC->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Stunned"))))
    {
        const FString StunnedName = ActiveCharacter->GetName();
        UE_LOG(LogTemp, Log, TEXT("UTurnManager: %s is Stunned — skipping turn."),
            *StunnedName);

        // End the turn immediately — this calls BeginTurn() again for the next combatant.
        EndTurn();
        return;
    }

    // Broadcast to all listeners: a new turn has started.
    // Camera, HUD, AI controller all react to this event.
    OnTurnStarted.Broadcast(ActiveCharacter);
}

bool UTurnManager::CheckCombatOver() const
{
    bool bPlayerAlive = false;
    bool bEnemyAlive = false;

    for (ABaseCharacter* Character : Combatants)
    {
        if (Character == nullptr || !Character->IsAlive()) continue;

        // IsPlayerCharacter() tells us if this is the human player's character.
        // Anything else is treated as an enemy for win/loss purposes.
        if (Character->IsPlayerCharacter())
        {
            bPlayerAlive = true;
        }
        else
        {
            bEnemyAlive = true;
        }
    }

    // Combat is over when one side has no living combatants.
    return !bPlayerAlive || !bEnemyAlive;
}

void UTurnManager::RemoveDeadCombatants()
{
    // Iterate BACKWARDS so that removing an element doesn't cause us to
    // skip the next one. (Removing index 1 shifts index 2 down to 1 —
    // a forward loop would jump over it. Backwards avoids this entirely.)
    for (int32 i = Combatants.Num() - 1; i >= 0; i--)
    {
        ABaseCharacter* Character = Combatants[i];

        if (Character == nullptr || !Character->IsAlive())
        {
            UE_LOG(LogTemp, Log, TEXT("UTurnManager: Removing dead combatant at index %d (%s)"),
                i, Character ? *Character->GetName() : TEXT("nullptr"));

            Combatants.RemoveAt(i);

            // Compensate for the shift: if the removed slot was BELOW the current
            // index, everything above it shifted down by 1 — so our index is now
            // pointing one slot too high. Subtract 1 to stay on the right character.
            if (i < CurrentTurnIndex)
            {
                CurrentTurnIndex--;
            }
        }
    }
}