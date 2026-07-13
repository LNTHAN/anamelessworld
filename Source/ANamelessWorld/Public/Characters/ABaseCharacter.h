// ABaseCharacter.h
// PROJECT: A Nameless World
// PURPOSE: The central character class. Every character in the game (protagonist,
//          companions, enemies) IS an ABaseCharacter or inherits from one.
//          Owns all four components and implements the GAS interface.
//
// DEPENDENCIES: UCRPGAttributeSet.h must exist (built in Session 1).
// DEPENDENTS:   APlayerCharacter.h and AEnemyCharacter.h will inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
// ACharacter — gives us movement, mesh, collision, animation for free.

#include "AbilitySystemInterface.h"
// IAbilitySystemInterface — the GAS contract.
// Forces us to implement GetAbilitySystemComponent().
// GAS calls that function on any actor it encounters.

#include "Attributes/UCRPGAttributeSet.h"
// Full include (not just forward declaration) because we store a pointer
// AND need to call ATTRIBUTE_ACCESSORS-generated functions on it.

#include "Inventory/UInventoryComponent.h"
// remove from class declaration once UInventoryComponent is written.

#include "Data/UCRPGCharacterData.h"

#include "Animation/AnimInstance.h"
// For Animation

#include "ABaseCharacter.generated.h"
// Always last. UHT generates this at build time for reflection.


// ── Forward Declarations ─────────────────────────────────────────────────────
// We only hold pointers to these in this header — no need to #include the full
// headers here. Full #includes are in ABaseCharacter.cpp where we call methods.
class UAbilitySystemComponent;
// class UDialogueComponent;
class UGameplayEffect;    // Used for default attribute initialisation effect
class UGameplayAbility;   // Used for the DefaultAbilities array


// ── Class Declaration ─────────────────────────────────────────────────────────

UCLASS()
class ANAMELESSWORLD_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
// ACharacter         — inherits a full character body with movement and physics
// IAbilitySystemInterface — GAS contract: promises GetAbilitySystemComponent() exists
// Multiple inheritance is fine here because IAbilitySystemInterface has no data —
// it is a pure interface (declarations only, zero implementation).
{
    GENERATED_BODY()

public:

    // ── Constructor ──────────────────────────────────────────────────────────
    ABaseCharacter();
    // Creates all components via CreateDefaultSubobject.
    // Sets up the ASC replication mode.
    // Body is in ABaseCharacter.cpp.


    // ── GAS Interface Requirement ────────────────────────────────────────────
    // IAbilitySystemInterface has exactly one pure virtual function.
    // We MUST override it or this class cannot compile.
    // GAS calls this on any actor to locate its AbilitySystemComponent.
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


    // ── UE5 Lifecycle Overrides ──────────────────────────────────────────────
    // BeginPlay is called once when the character appears in the world.
    // We use it to: initialise default attribute values and grant default abilities.
    virtual void BeginPlay() override;


    // ── Death ────────────────────────────────────────────────────────────────
    // Called by UCRPGAttributeSet::PostGameplayEffectExecute when Health hits 0.
    // Plays death animation, disables input, notifies UTurnManager.
    // virtual = APlayerCharacter and AEnemyCharacter can override with specific behaviour.
    UFUNCTION(BlueprintCallable, Category = "ANW|Character")
    virtual void Die();


    // ── Abilities ────────────────────────────────────────────────────────────
    // Grants every ability in the DefaultAbilities array to the ASC.
    // Called once in BeginPlay.
    void InitDefaultAbilities();

    // Applies the DefaultAttributeEffect to initialise base stat values.
    // Using a GameplayEffect for this (instead of hardcoding in the constructor)
    // means designers can tweak starting stats in the editor without recompiling.
    void InitDefaultAttributes();


    // ── UTurnManager Interface ───────────────────────────────────────────────
    // UTurnManager cannot touch Attributes or bIsDead directly (they are protected).
    // These two functions are the controlled window UTurnManager uses instead.

    // Returns true if this character is still alive.
    // UTurnManager calls this in RemoveDeadCombatants() and CheckCombatOver().
    UFUNCTION(BlueprintCallable, Category = "ANW|State")
    bool IsAlive() const;

    // True only for the human player's character. We can't use the engine's
    // IsPlayerControlled() anymore — since the tactical-camera refactor, the
    // PlayerController possesses the camera rig and Nameless is driven by an
    // AIController, so the engine no longer sees him as "player controlled."
    // This virtual is possession-proof: identity, not who's holding the strings.
    UFUNCTION(BlueprintCallable, Category = "ANW|State")
    virtual bool IsPlayerCharacter() const { return false; }

    // Returns the raw Dexterity score (e.g. 14.0).
    // UTurnManager uses this to calculate the initiative modifier for turn order.
    // Formula: modifier = floor((Dexterity - 10) / 2)
    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetDexterity() const;

    // Returns current Health — used by HUD to fill the health bar.
    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetHealth() const;

    // Returns MaxHealth — used by HUD to calculate Health/MaxHealth ratio.
    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetMana() const;

    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetMaxMana() const;

    // Named GetCharacterLevel, NOT GetLevel — AActor::GetLevel() already exists
    // (returns a ULevel*) and would clash.
    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetCharacterLevel() const;

    UFUNCTION(BlueprintCallable, Category = "ANW|Attributes")
    float GetXP() const;

    // Display name for HUD/cards. Falls back to the actor name if no data asset.
    UFUNCTION(BlueprintCallable, Category = "ANW|Identity")
    FText GetUnitName() const;

    // Sets bIsAttacking on the Animation Blueprint — called when an ability fires.
    UFUNCTION(BlueprintCallable, Category = "ANW|Animation")
    void SetIsAttacking(bool bAttacking);

    // Sets bIsDead on the Animation Blueprint — called when the character dies.
    UFUNCTION(BlueprintCallable, Category = "ANW|Animation")
    void SetIsDead(bool bDead);


    // ── Movement ─────────────────────────────────────────────────────────────
    // How far this character may travel in ONE turn, in world units (cm),
    // measured as path distance across the NavMesh — NOT straight-line.
    // This is the tactical "Speed" stat: later it can be derived from an
    // attribute; for now it's a per-character tunable in the editor.
    // ~500 ≈ 5 metres. Both the player and enemies read this.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Movement")
    float MoveRange = 500.f;

    // Instantly turn to face a target (horizontal only). Called before an
    // attack/cast so characters look at who they're acting on.
    UFUNCTION(BlueprintCallable, Category = "ANW|Combat")
    void FaceActor(const AActor* Target);

    // ── Action economy (per-turn stocks) ─────────────────────────────────────
    // Each turn a combatant gets ONE move and ONE action, spent independently
    // and in any order. Both refill when this combatant's turn begins.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Turn")
    bool bMoveAvailable = true;

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Turn")
    bool bActionAvailable = true;

    // Refill both stocks. The TurnManager calls this at the start of this
    // combatant's turn.
    UFUNCTION(BlueprintCallable, Category = "ANW|Turn")
    void ResetTurnResources();

    // Enables/disables this character carving the navmesh as an obstacle. The
    // TurnManager switches it OFF for whoever's moving and ON for everyone else,
    // so movers route around standing characters but never around themselves.
    void SetNavObstacleEnabled(bool bEnabled);


protected:

    // ── GAS Core Components ──────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Components|GAS")
    // VisibleAnywhere: shows in the editor Details panel (read-only there).
    // BlueprintReadOnly: Blueprints can read the pointer but not reassign it.
    UAbilitySystemComponent* AbilitySystemComponent;
    // The GAS brain. Manages all abilities, active effects, and gameplay tags.
    // Every character needs exactly one. Created in the constructor.

    UPROPERTY()
    // No editor/Blueprint specifiers — the AttributeSet is an implementation detail.
    // UPROPERTY() alone is enough to register it with the Garbage Collector.
    // Without UPROPERTY(), the GC doesn't know it exists and may delete it mid-game.
    UCRPGAttributeSet* Attributes;
    // Our stat sheet. Registered with the ASC automatically because it inherits
    // from UAttributeSet — the ASC discovers it via UE5's reflection system.


    // ── Gameplay Components ──────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Components")
    UInventoryComponent* InventoryComponent;
    // The backpack. Holds TArray<FInventoryItem>.
    // Defined in UInventoryComponent.h (written in Session 3).

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Components")
    //UDialogueComponent* DialogueComponent;
    // Connects to the DataTable-driven branching dialogue system.
    // Defined in UDialogueComponent.h (written in Session 4+).


    // ── GAS Configuration ────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Abilities")
    // EditDefaultsOnly: designers can edit this in the Blueprint Class Defaults,
    //                   but not on individual placed instances. Good for "template" data.
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
    // TSubclassOf<UGameplayAbility> — a pointer to a CLASS (not an instance).
    // Holds the ability blueprints to grant on spawn (e.g. GA_BasicAttack).
    // WHY TSubclassOf? Because abilities are created by GAS from the class definition,
    // not passed in as pre-made objects.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Attributes")
    TSubclassOf<UGameplayEffect> DefaultAttributeEffect;
    // A GameplayEffect that sets starting attribute values (Health=100, INT=14, etc.)
    // Designers configure this per character type in Blueprints — no code change needed
    // when adjusting stats. This is the "DataAsset instead of magic numbers" principle.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ANW|Data")
    UCRPGCharacterData* CharacterData;

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "ANW|State")
    bool bIsDead = false;
    // Prevents Die() being called more than once (e.g. two effects landing on the
    // same frame could both trigger death). Checked at the start of Die().
};
