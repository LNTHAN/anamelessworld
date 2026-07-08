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

#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"

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

    // Let environmental trigger spheres (e.g. the rigged bookshelf) detect us
    // walking in. Overlap events fire only if BOTH sides generate them — the
    // shelf's sphere already does; the capsule doesn't by default.
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
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

    // Register each character as a dynamic navmesh obstacle so OTHERS path around
    // them. Must be here, not the constructor — at construction the capsule isn't
    // registered with the world/nav system yet, so this silently no-ops there.
    // The TurnManager toggles it off for whoever's moving (see SetNavObstacleEnabled).
    GetCapsuleComponent()->bDynamicObstacle = true;
    GetCapsuleComponent()->SetCanEverAffectNavigation(true);

    // Initialise attributes first — we need valid stat values before granting
    // abilities (some ability costs reference Mana, which must exist).
    InitDefaultAttributes();

    // Then grant abilities so they can reference the now-initialised stats.
    InitDefaultAbilities();
}


void ABaseCharacter::SetNavObstacleEnabled(bool bEnabled)
{
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        // SetCanEverAffectNavigation only dirties the navmesh when the value
        // actually changes, so calling this every turn is cheap.
        Capsule->SetCanEverAffectNavigation(bEnabled);
    }
}


// ════════════════════════════════════════════════════════════════════════════
// INITIALISATION
// ════════════════════════════════════════════════════════════════════════════

void ABaseCharacter::InitDefaultAbilities()
{
    if (!AbilitySystemComponent) return;

    const TArray<TSubclassOf<UGameplayAbility>>& AbilitiesToGrant =
        (CharacterData && CharacterData->Abilities.Num() > 0)
            ? CharacterData->Abilities
            : DefaultAbilities;

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitiesToGrant)
    {
        if (!AbilityClass) continue;
        FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
        AbilitySystemComponent->GiveAbility(Spec);
    }
}

void ABaseCharacter::InitDefaultAttributes()
{
    if (!AbilitySystemComponent) return;

    // Prefer CharacterData if set; fall back to the legacy DefaultAttributeEffect.
    // This lets existing Blueprints keep working before Data Assets are assigned.
    TSubclassOf<UGameplayEffect> EffectToApply =
        (CharacterData && CharacterData->AttributeEffect)
            ? CharacterData->AttributeEffect
            : DefaultAttributeEffect;

    // Nothing to apply — character has no attribute initialisation configured yet.
    if (!EffectToApply) return;

    // Build a context so GAS knows who is applying this effect (self).
    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);

    // Create the effect spec at level 1 — level scaling not used yet.
    FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
        EffectToApply, 1.0f, Context);

    if (Spec.IsValid())
    {
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
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
    SetIsDead(true);
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

void ABaseCharacter::ResetTurnResources()
{
    bMoveAvailable = true;
    bActionAvailable = true;
}

float ABaseCharacter::GetHealth() const
{
    if (Attributes == nullptr) return 0.f;
    return Attributes->GetHealth();
}

float ABaseCharacter::GetMaxHealth() const
{
    if (Attributes == nullptr) return 1.f;
    return Attributes->GetMaxHealth();
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

void ABaseCharacter::SetIsAttacking(bool bAttacking)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(FName("bIsAttacking"));
    if (Property)
    {
        bool* ValuePtr = Property->ContainerPtrToValuePtr<bool>(AnimInstance);
        if (ValuePtr) *ValuePtr = bAttacking;
    }
}

void ABaseCharacter::SetIsDead(bool bDead)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(
        FName("bIsDead"));
    if (Property)
    {
        bool* ValuePtr = Property->ContainerPtrToValuePtr<bool>(AnimInstance);
        if (ValuePtr) *ValuePtr = bDead;
    }
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
