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


    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "ANW|State")
    bool bIsDead = false;
    // Prevents Die() being called more than once (e.g. two effects landing on the
    // same frame could both trigger death). Checked at the start of Die().
};
