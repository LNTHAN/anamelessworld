// ABaseCharacter.cpp
// PROJECT: A Nameless World
// PURPOSE: Implements ABaseCharacter — creates components, wires up GAS,
//          handles BeginPlay initialisation and the death sequence.

#include "Characters/ABaseCharacter.h"
// Always include our own header first.
// ── Full includes (we call methods on these here, so we need the full definition) ──

#include "AbilitySystemComponent.h"
// Needed to call AbilitySystemComponent->GiveAbility() and ->ApplyGameplayEffect...()

#include "GameplayEffect.h"
// Needed for the DefaultAttributeEffect application in InitDefaultAttributes().

#include "GameplayAbilitySpec.h"
// Provides FGameplayAbilitySpec — the struct GAS uses to represent a granted ability.

#include "Inventory/UInventoryComponent.h"

// TODO Session 3: #include "Inventory/UInventoryComponent.h"
// TODO Session 4: #include "Dialogue/UDialogueComponent.h"
// These headers do not exist yet. Until they are written, the components below
// are created with a stub placeholder comment.


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

ABaseCharacter::ABaseCharacter()
// Runs once when the character class is loaded — before BeginPlay.
// Job: create all components and configure them.
// RULE: Never put gameplay logic here. Constructor = creation only.
{
    // ── GAS: AbilitySystemComponent ─────────────────────────────────────────
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
        TEXT("AbilitySystemComponent"));
    // CreateDefaultSubobject<T>(Name):
    //   • Asks UE5's memory system to build a T
    //   • Registers it with the Garbage Collector (so it is not deleted mid-game)
    //   • Attaches it to this actor
    //   • Returns the pointer (address) to the new component
    // TEXT("AbilitySystemComponent") — internal name for editor and debugging.

    // Replication mode: how the ASC syncs across a network.
    // Mixed = server authoritative, clients get meaningful updates.
    // For a single-player CRPG this makes no real difference,
    // but it is the correct default for a GAS character.
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


    // ── GAS: UCRPGAttributeSet ───────────────────────────────────────────────
    Attributes = CreateDefaultSubobject<UCRPGAttributeSet>(TEXT("Attributes"));
    // The AttributeSet does NOT need to be manually registered with the ASC.
    // UE5's reflection system detects any UAttributeSet subclass created via
    // CreateDefaultSubobject and registers it automatically.
    // After this line: AbilitySystemComponent->GetSet<UCRPGAttributeSet>() works.


    // ── Inventory ────────────────────────────────────────────────────────────
    // Session 3 complete — UInventoryComponent now exists, enabling this.
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));


    // ── Dialogue ─────────────────────────────────────────────────────────────
    // TODO Session 4: replace with real UDialogueComponent once written.
    // DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("Dialogue"));


    // ── Character defaults ───────────────────────────────────────────────────
    // PrimaryActorTick.bCanEverTick = false prevents the engine calling Tick()
    // every frame. We do not need per-frame updates — GAS and turn logic handle
    // timing. Disabling tick improves performance, especially with many characters.
    PrimaryActorTick.bCanEverTick = false;
}


// ════════════════════════════════════════════════════════════════════════════
// GAS INTERFACE
// ════════════════════════════════════════════════════════════════════════════

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
// Required by IAbilitySystemInterface.
// GAS calls this on every actor it interacts with to locate the ASC.
// Without it: GAS cannot grant abilities, apply effects, or check tags.
{
    return AbilitySystemComponent;
    // Just returns the pointer. Simple on purpose.
    // The complexity lives inside the ASC itself — not here.
}


// ════════════════════════════════════════════════════════════════════════════
// LIFECYCLE
// ════════════════════════════════════════════════════════════════════════════

void ABaseCharacter::BeginPlay()
// Called once when this character appears in the world (after the constructor).
// Constructor = creation. BeginPlay = "the game has started, do your setup."
{
    // Always call the parent version first.
    // ACharacter::BeginPlay() does important internal setup we must not skip.
    Super::BeginPlay();

    // Initialise attributes first — we need valid stat values before granting
    // abilities (some ability costs reference Mana, which must exist).
    InitDefaultAttributes();

    // Then grant abilities so they can reference the now-initialised stats.
    InitDefaultAbilities();
}


// ════════════════════════════════════════════════════════════════════════════
// INITIALISATION
// ════════════════════════════════════════════════════════════════════════════

void ABaseCharacter::InitDefaultAbilities()
// Grants every ability in the DefaultAbilities array to the AbilitySystemComponent.
// Designers add ability classes (e.g. GA_BasicAttack) in the Blueprint Class Defaults.
// Code does not need to know the specific abilities — it just grants the list.
{
    // Guard: only the server grants abilities in a networked game.
    // HasAuthority() returns true on the server and in single-player.
    // In single-player this is always true — the check is safe and future-proof.
    if (!HasAuthority()) return;

    // Guard: nothing to do if the ASC is null or the list is empty.
    if (!AbilitySystemComponent) return;
    if (DefaultAbilities.IsEmpty()) return;

    // Iterate the list and grant each ability.
    for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    // Range-for loop: runs once per item in the array.
    // TSubclassOf<UGameplayAbility>& — a reference to the class pointer (not a copy).
    {
        if (!AbilityClass) continue;
        // Skip any null entries (a designer may have left a slot blank).

        // Build an ability spec: class + starting level + optional input binding.
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
        // Level 1 = the ability starts at its base power.
        // Input binding (-1) = no key bound yet. We will bind in APlayerCharacter.

        AbilitySystemComponent->GiveAbility(AbilitySpec);
        // GiveAbility registers the ability with the ASC.
        // The character can now call TryActivateAbility() to use it.
    }
}

void ABaseCharacter::InitDefaultAttributes()
// Applies the DefaultAttributeEffect GameplayEffect to set starting stat values.
// WHY use a GameplayEffect instead of hardcoding in the constructor?
//   → Designers configure stats per character type in Blueprints (no recompile).
//   → Stats can be reapplied on respawn or load cleanly via GAS.
//   → Consistent with how all stat changes work — one system, not two.
{
    if (!AbilitySystemComponent || !DefaultAttributeEffect) return;
    // If no effect is assigned in the Blueprint Class Defaults, skip silently.
    // This is intentional: a bare ABaseCharacter has no stats until a child
    // class assigns a DefaultAttributeEffect.

    // Build a context for the effect — who is applying it and why.
    FGameplayEffectContextHandle ContextHandle =
        AbilitySystemComponent->MakeEffectContext();
    // MakeEffectContext() creates a context tagged "self-applied at startup."
    // GAS uses context to track the source of effects for logging and gameplay logic.

    ContextHandle.AddSourceObject(this);
    // Tag this actor as the source. "I am applying this effect to myself."

    // Create a spec (a configured instance) from the effect class.
    FGameplayEffectSpecHandle SpecHandle =
        AbilitySystemComponent->MakeOutgoingSpec(
            DefaultAttributeEffect,   // Which effect class
            Attributes->GetCharacterLevel(),               // Apply at the character's current level
            ContextHandle);           // Who is applying it

    // GetCharacterLevel() reads our CharacterLevel attribute from UCRPGAttributeSet.
    // Effect magnitudes can scale with level — e.g. MaxHealth = 80 + (CharacterLevel * 10).

    if (SpecHandle.IsValid())
    // IsValid() checks the spec was created successfully (effect class is not null).
    {
        // Apply the effect to ourselves. This triggers PostGameplayEffectExecute
        // in UCRPGAttributeSet, which clamps the values.
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}


// ════════════════════════════════════════════════════════════════════════════
// DEATH
// ════════════════════════════════════════════════════════════════════════════

void ABaseCharacter::Die()
// Called by UCRPGAttributeSet::PostGameplayEffectExecute when Health reaches 0.
// Handles the death sequence: guard check → state flag → animation → cleanup.
{
    // Guard: if we are already dead, do nothing.
    // Two GameplayEffects landing on the same frame could both trigger Die().
    // The bIsDead flag ensures the sequence only runs once.
    if (bIsDead) return;

    bIsDead = true;
    // Set the flag immediately so any re-entrant calls are blocked.

    // Disable further ability activation — a dead character cannot act.
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->CancelAllAbilities();
        // CancelAllAbilities() cleanly stops any in-progress abilities
        // (e.g. a cast that was interrupted by the killing blow).
    }

    // TODO Session 5 (Animation): Play death montage here.
    // GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);

    // TODO Session 4 (Turn system): Notify UTurnManager that this character is dead.
    // TurnManager->OnCharacterDied(this);

    // Log so we can verify the sequence fires correctly during development.
    UE_LOG(LogTemp, Warning,
        TEXT("ABaseCharacter::Die() — %s has died."),
        *GetName());
    // *GetName() — dereferences the FString to a raw TCHAR* that %s expects.
}


// ════════════════════════════════════════════════════════════════════════════
// UTURNMANAGER INTERFACE
// ════════════════════════════════════════════════════════════════════════════

bool ABaseCharacter::IsAlive() const
{
    // bIsDead is flipped to true inside Die() when Health hits 0.
    // We return the opposite — alive means NOT dead.
    return !bIsDead;
}

float ABaseCharacter::GetDexterity() const
{
    // Guard: Attributes should always exist after the constructor runs,
    // but we check defensively to avoid a crash if something went wrong at spawn.
    if (Attributes == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("%s: GetDexterity() called but Attributes is null. Returning 10 (neutral modifier)."),
            *GetName());
        return 10.0f;
        // Score 10 → modifier 0 → no bonus or penalty to initiative.
    }

    // ATTRIBUTE_ACCESSORS generated GetDexterity() on UCRPGAttributeSet in Session 1.
    // We call it here so UTurnManager never needs to touch Attributes directly.
    return Attributes->GetDexterity();
}


// ════════════════════════════════════════════════════════════════════════════
// NOTES FOR CHILD CLASSES
// ════════════════════════════════════════════════════════════════════════════
//
// APlayerCharacter (Session 3) overrides:
//   - SetupPlayerInputComponent() — binds keyboard/mouse input
//   - Die()                       — adds "show game over screen" logic
//   - BeginPlay()                 — adds camera setup
//
// AEnemyCharacter (Session 3) overrides:
//   - BeginPlay() — initialises AI controller
//   - Die()       — adds loot drop and XP grant to the player
//   - adds ExecuteAITurn() — AI decision logic
