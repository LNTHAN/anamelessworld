// UTurnManager.h
// PROJECT: A Nameless World
// PURPOSE: The referee of combat. Tracks who is fighting, whose turn it is,
//          and what state combat is in. Pure logic — no position in the world.
//          Other systems (camera, HUD, AI) react to its delegate broadcasts.
//
// DEPENDENCIES: ABaseCharacter.h (forward declared here; full include in .cpp)
// DEPENDENTS:   APlayerCharacter and AEnemyCharacter will call EndTurn() when
//               their action completes. The HUD will read GetCurrentCombatant().

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
// UObject/NoExportTypes.h — base include for classes that inherit from UObject
// but do NOT need Actor features (no position, no mesh, no physics).
// Compare: ABaseCharacter used GameFramework/Character.h because it IS in the world.
// UTurnManager is pure logic, so UObject is the right parent.

#include "UTurnManager.generated.h"
// Always last. UHT generates this file at build time for the reflection system
// (UPROPERTY, UFUNCTION, UENUM all depend on it).


// ── Forward Declaration ───────────────────────────────────────────────────────
// We only store ABaseCharacter* (a pointer) in this header.
// A pointer is just a memory address — the compiler only needs to know the
// class name exists, not its full layout. This avoids pulling in ABaseCharacter.h
// here and keeps compile times faster. The full #include goes in UTurnManager.cpp.
class ABaseCharacter;


// ── ETurnState ────────────────────────────────────────────────────────────────
// The "traffic light" for combat. At any moment, combat is in exactly ONE state.
// UTurnManager is responsible for switching between them.
//
// BlueprintType — lets Blueprint graphs read and branch on this enum (useful
// for UI widgets that want to show different things per state).
UENUM(BlueprintType)
enum class ETurnState : uint8
{
    // Combat has not started yet. Combatants are still being registered.
    WaitingToStart  UMETA(DisplayName = "Waiting To Start"),

    // It is the player's turn. The game is waiting for player input.
    // HUD should show the command window (attack / item / etc.)
    PlayerTurn      UMETA(DisplayName = "Player Turn"),

    // It is an enemy's turn. The AI is choosing and executing an action.
    // Player input should be ignored in this state.
    EnemyTurn       UMETA(DisplayName = "Enemy Turn"),

    // An action is currently playing out (animation, VFX, damage numbers).
    // This is the LOCK state — nobody can act until the animation finishes.
    // Even "animation skip" compresses this state's duration, not removes it.
    Animating       UMETA(DisplayName = "Animating"),

    // Combat is finished. Used by the game to show victory/defeat screen.
    GameOver        UMETA(DisplayName = "Game Over"),
};


// ── Delegate Signatures ───────────────────────────────────────────────────────
// Delegates are "announcement systems". UTurnManager broadcasts events and any
// other system can subscribe (listen) without UTurnManager knowing they exist.
// This keeps the architecture clean — UTurnManager never calls the camera or HUD
// directly. It just shouts "a turn started!" and whoever cares reacts.
//
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(TypeName, ParamType, ParamName)
//   Dynamic    — can be bound in Blueprint as well as C++
//   Multicast  — multiple listeners can subscribe at the same time
//   OneParam   — this broadcast carries one piece of data as payload

// Fired when a combatant begins their turn.
// Payload: the ABaseCharacter* who is now acting.
// Who listens: Camera (pan to them), HUD (show their stats + commands)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStarted, ABaseCharacter*, ActiveCombatant);

// Fired when a new round begins (the index wrapped back to 0).
// Payload: the round number (1, 2, 3...).
// Who listens: UI that shows "Round 2" banner
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, RoundNumber);

// Fired when combat ends.
// Payload: true if the player won, false if the player lost.
// Who listens: Victory / defeat screen
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEnded, bool, bPlayerWon);


// ── UTurnManager ──────────────────────────────────────────────────────────────
/**
 * UTurnManager
 *
 * The referee of combat. Owns:
 *   1. Combatants[]    — the roster: every character in the current fight
 *   2. CurrentTurnIndex — which slot in the roster is currently acting
 *   3. CurrentState    — the ETurnState traffic light
 *
 * Lifecycle of a fight:
 *   1. Call AddCombatant() for every fighter (player + all enemies)
 *   2. Call StartCombat() — rolls initiative, sorts roster, begins Turn 1
 *   3. Each character calls EndTurn() when their action is done
 *   4. UTurnManager advances the index and fires OnTurnStarted again
 *   5. When all enemies (or the player) are dead → fires OnCombatEnded
 *
 * HOW TO CREATE: UTurnManager is a UObject, not an Actor.
 *   Use NewObject<UTurnManager>(this) in your GameMode or a manager Actor.
 *   Do NOT place it in the level — it has no physical existence.
 */
UCLASS(BlueprintType)
class ANAMELESSWORLD_API UTurnManager : public UObject
{
    GENERATED_BODY()

public:

    // ── Delegates (public so other systems can subscribe) ─────────────────────

    // Broadcast when a combatant's turn begins.
    // Subscribe: OnTurnStarted.AddDynamic(this, &YourClass::YourFunction)
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager|Events")
    FOnTurnStarted OnTurnStarted;

    // Broadcast when a new round starts (index wrapped to 0).
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager|Events")
    FOnRoundStarted OnRoundStarted;

    // Broadcast when combat ends. Pass true = player won, false = player lost.
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager|Events")
    FOnCombatEnded OnCombatEnded;


    // ── Public API ────────────────────────────────────────────────────────────
    // These are the only functions outside systems should ever call.

    /**
     * Register a character into the upcoming fight.
     * Must be called for EVERY combatant before StartCombat().
     * Order does not matter here — StartCombat() will sort by initiative.
     *
     * @param NewCombatant  The character joining the fight. Must not be nullptr.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void AddCombatant(ABaseCharacter* NewCombatant);

    // Auto-register every ABaseCharacter in the world + wire each one's combat.
    // Replaces per-enemy Level BP wiring. Call once, before StartCombat.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat", meta = (WorldContext = "WorldContextObject"))
    void RegisterWorldCombatants(UObject* WorldContextObject);
    
    /**
     * Begin the fight. Call this once all combatants have been added.
     * Internally: rolls initiative for everyone, sorts the Combatants array,
     * sets RoundNumber to 1, then calls BeginTurn() on index 0.
     * After this returns, OnTurnStarted will have already fired.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void StartCombat();

    // Called by ABaseCharacter::Die(). Ends combat immediately if this death
    // emptied one side — so the result screen appears on the killing blow.
    void NotifyCombatantDied(ABaseCharacter* DeadCombatant);

    /**
     * Signal that the current combatant has finished their action.
     * Call this from APlayerCharacter (after ability resolves) or
     * AEnemyCharacter (after AI finishes). UTurnManager handles the rest:
     * removes dead characters, checks for combat over, advances the index.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void EndTurn();

    /**
     * Returns the character currently taking their turn.
     * Returns nullptr if combat has not started or Combatants is empty.
     * Use this to: aim abilities at the right target, focus the camera, etc.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    ABaseCharacter* GetCurrentCombatant() const;

    /**
     * Returns the current ETurnState (PlayerTurn, EnemyTurn, Animating, etc.)
     * UI and input systems check this before accepting player commands.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    ETurnState GetCurrentState() const;

    /**
     * Returns which round we are in (starts at 1, increments when the
     * index wraps back to 0 after everyone has taken a turn).
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    int32 GetRoundNumber() const;

    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    TArray<ABaseCharacter*> GetTurnOrder() const;


private:

    // ── Internal State ────────────────────────────────────────────────────────
    // Private means only UTurnManager's own functions can touch these.
    // Outside systems read them through the getter functions above (GetCurrentState, etc.)

    // Everyone in the current fight, sorted by initiative after StartCombat().
    // We store POINTERS (*) — TurnManager does not own these characters.
    // The world (level) owns them. We just reference them.
    // UPROPERTY() is required: without it the garbage collector cannot see this
    // array and may delete the characters while we still hold pointers to them.
    UPROPERTY()
    TArray<ABaseCharacter*> Combatants;

    // Which slot in Combatants[] is currently taking their turn.
    // Starts at 0. Advances each turn via: (CurrentTurnIndex + 1) % Combatants.Num()
    // That formula wraps 0→1→2→0 like a clock, never going out of bounds.
    UPROPERTY()
    int32 CurrentTurnIndex = 0;

    // Which round we are in. Starts at 1.
    // Increments every time CurrentTurnIndex wraps back to 0 (everyone has acted).
    UPROPERTY()
    int32 RoundNumber = 0;

    // The current state of combat. Starts as WaitingToStart.
    // Only BeginTurn() and EndTurn() should write to this.
    UPROPERTY()
    ETurnState CurrentState = ETurnState::WaitingToStart;


    // ── Private Helpers ───────────────────────────────────────────────────────
    // Internal machinery. Not part of the public contract.
    // Other classes should never call these — only UTurnManager's own functions do.

    /**
     * Roll initiative for one character: d20 (random 1–20) + Dexterity modifier.
     * The modifier is (Dexterity - 10) / 2, which you built in UCRPGAttributeSet.
     * Higher result = acts sooner in the sorted order.
     *
     * @param Character  The character rolling. Must not be nullptr.
     * @return           The initiative score (integer, can be negative for low Dex).
     */
    int32 RollInitiative(ABaseCharacter* Character) const;

    /**
     * Sort the Combatants array from highest to lowest initiative score.
     * Called once inside StartCombat() after all characters have rolled.
     * This sorted order persists for the entire fight (no re-rolling each round).
     */
    void SortByInitiative();

    /**
     * Start the turn for whoever is at CurrentTurnIndex.
     * Sets CurrentState to PlayerTurn or EnemyTurn depending on who they are.
     * Fires the OnTurnStarted delegate so other systems can react.
     * Called at the end of StartCombat() and at the end of EndTurn().
     */
    void BeginTurn();

    // Shared victory/defeat resolution: set GameOver, work out who won, broadcast.
    void EndCombat();
    
    /**
     * Check whether combat should end.
     * Combat ends when: all enemies are dead (player wins), OR the player is dead.
     * Returns true if the fight is over, false if it should continue.
     */
    bool CheckCombatOver() const;

    /**
     * Remove any characters with 0 (or less) Health from the Combatants array.
     * Called AFTER advancing CurrentTurnIndex in EndTurn() — never before.
     *
     * Order matters: advance first, remove second.
     * If we removed first, the array would shrink before the modulo runs,
     * producing a wrong next-index (e.g. wrapping to Goblin instead of Player).
     *
     * After removing, we adjust CurrentTurnIndex by -1 for each removed entry
     * whose array position was BELOW the new current index, to compensate for
     * the shift. Without this, the index would point past the end of the array.
     */
    void RemoveDeadCombatants();
};
