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

#include "Components/WidgetComponent.h"

#include "UI/UHealthBarWidget.h"

#include "Data/CRPGCharacterRow.h"

#include "UI/UFloatingCombatText.h"

#include "TimerManager.h"

#include "TurnManager/UTurnManager.h"

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

    // ── Floating HP bar ──────────────────────────────────────────────────────
    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
    HealthBarWidget->SetupAttachment(RootComponent);          // ride on the capsule
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);    // billboard: always faces camera, constant size
    HealthBarWidget->SetDrawSize(FVector2D(150.f, 18.f));     // pixel size of the bar
    HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // above the head (capsule half-height is 88)
    HealthBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision); // never blocks clicks/traces    
    // The camera boom must never collide with characters. If a pivot lands on a
    // unit (e.g. the establishing shot's battlefield center), the spring arm would
    // otherwise yank the camera tight against them. Walls still block the boom.
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

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

    // Prefer the DataTable row (spreadsheet-driven stats); fall back to the legacy
    // stat-init GameplayEffect only when no row is assigned.
    if (!InitStatsFromRow())
    {
        InitDefaultAttributes();
    }

    // Then grant abilities so they can reference the now-initialised stats.
    InitDefaultAbilities();

    // Tell the floating bar whose HP to display. InitWidget() forces the widget
    // instance to exist now (Screen-space components otherwise create it lazily
    // on first render), so the cast below succeeds on frame one.
    if (HealthBarWidget)
    {
        HealthBarWidget->InitWidget();
        if (UHealthBarWidget* Bar = Cast<UHealthBarWidget>(HealthBarWidget->GetUserWidgetObject()))
        {
            Bar->OwnerCharacter = this;
        }
    }
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

bool ABaseCharacter::InitStatsFromRow()
{
    if (!AbilitySystemComponent || CharacterRowHandle.IsNull()) return false;

    const FCRPGCharacterRow* Row =
        CharacterRowHandle.GetRow<FCRPGCharacterRow>(TEXT("InitStatsFromRow"));
    if (!Row) return false;   // handle set but row missing / wrong struct

    // Cache the row's DisplayName so GetUnitName can prefer it over the Data Asset.
    CachedDisplayName = Row->DisplayName;

    // Max pools first, then current starts full. Init* sets the attribute's base
    // value directly (no GameplayEffect, no clamp callback) — right for spawn-time.
    Attributes->InitMaxHealth(Row->MaxHealth);
    Attributes->InitMaxMana(Row->MaxMana);
    Attributes->InitHealth(Row->MaxHealth);
    Attributes->InitMana(Row->MaxMana);

    // The six D&D scores (int in the sheet, applied as float attributes).
    Attributes->InitStrength(Row->Strength);
    Attributes->InitDexterity(Row->Dexterity);
    Attributes->InitConstitution(Row->Constitution);
    Attributes->InitIntelligence(Row->Intelligence);
    Attributes->InitWisdom(Row->Wisdom);
    Attributes->InitCharisma(Row->Charisma);

    // Non-attribute tunable that also lives in the row.
    MoveRange = Row->MoveRange;
    AttackDamage = Row->AttackDamage;
    HeavyDamage  = Row->HeavyDamage;

    // Data-driven status immunity: a unit whose row flags bStatusImmune owns the
    // Immunity.Status tag permanently. Confuse's GameplayEffect will refuse to apply
    // to a tag-holder (Step 2), and Intimidate's C++ sweep will skip them (Step 3).
    // Capability flag, not an archetype — any unit can carry it.
    if (Row->bStatusImmune)
    {
        AbilitySystemComponent->AddLooseGameplayTag(
            FGameplayTag::RequestGameplayTag(FName("Immunity.Status")));
    }

    return true;
}

// ════════════════════════════════════════════════════════════════════════════
// DEATH
// ════════════════════════════════════════════════════════════════════════════

void ABaseCharacter::SetupCombat(UTurnManager* InTurnManager, ABaseCharacter* InPlayerTarget)
{
    // Remember the referee so Die() can report deaths immediately. Subclasses that
    // override this (AEnemyCharacter) MUST call Super::SetupCombat() to keep this.
    CachedTurnManager = InTurnManager;
}

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

    // Guarantee the corpse blocks even if it died as the active mover (whose nav
    // obstacle was toggled OFF for its move). A dead body should always be solid.
    SetNavObstacleEnabled(true);

    if (HealthBarWidget)
    {
        HealthBarWidget->SetVisibility(false);
    }

    // TODO Session 5 (Animation): Play death montage here.
    // GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);

    // Tell the referee someone just died, so it can declare victory/defeat the
    // moment the killing blow lands — not only when a turn is manually ended.
    if (CachedTurnManager)
    {
        CachedTurnManager->NotifyCombatantDied(this);
    }

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

float ABaseCharacter::GetMana() const
{
    return Attributes->GetMana();
}

float ABaseCharacter::GetMaxMana() const
{
    return Attributes->GetMaxMana();
}

float ABaseCharacter::GetCharacterLevel() const
{
    return Attributes->GetCharacterLevel();
}

float ABaseCharacter::GetXP() const
{
    return Attributes->GetXP();
}

FText ABaseCharacter::GetUnitName() const
{
    // Row's DisplayName is the source of truth; fall back to the Data Asset name,
    // then the actor name, for characters that have no row.
    if (!CachedDisplayName.IsEmpty())
    {
        return CachedDisplayName;
    }
    return CharacterData ? CharacterData->CharacterName : FText::FromString(GetName());
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

float ABaseCharacter::GetStrength() const
{
    return Attributes ? Attributes->GetStrength() : 0.f;
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

void ABaseCharacter::PlayHitReact()
{
    if (bIsDead) return;                 // corpses don't flinch
    SetIsHit(true);
    // Auto-clear after the flinch window. Object+method timer self-invalidates
    // if this character is ever destroyed, so no dangling callback.
    GetWorldTimerManager().SetTimer(
        HitReactTimerHandle, this, &ABaseCharacter::EndHitReact, HitReactDuration, false);
}

void ABaseCharacter::EndHitReact()
{
    SetIsHit(false);
}

void ABaseCharacter::SetIsHit(bool bHit)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(FName("bIsHit"));
    if (Property)
    {
        bool* ValuePtr = Property->ContainerPtrToValuePtr<bool>(AnimInstance);
        if (ValuePtr) *ValuePtr = bHit;
    }
}

void ABaseCharacter::ShowCombatText(const FText& Text, FLinearColor Color)
{
    if (!FloatingTextClass) return;

    // A transient, screen-facing widget pinned over the head — same tech as the HP
    // bar, but spawned fresh per hit and destroyed when its rise/fade anim ends.
    UWidgetComponent* Popup = NewObject<UWidgetComponent>(this);
    Popup->SetWidgetSpace(EWidgetSpace::Screen);          // always faces camera, stays crisp
    Popup->SetWidgetClass(FloatingTextClass);
    Popup->SetDrawSize(FVector2D(200.f, 80.f));
    Popup->SetupAttachment(GetRootComponent());
    Popup->SetRelativeLocation(FVector(0.f, 0.f, 150.f)); // above the head
    Popup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Popup->RegisterComponent();                           // makes it live in the world
    Popup->InitWidget();                                  // force-create the UMG widget now

    if (UFloatingCombatText* Widget = Cast<UFloatingCombatText>(Popup->GetUserWidgetObject()))
    {
        Widget->Init(Text, Color);
    }

    // Self-clean after the anim finishes (timer must outlast the UMG rise/fade).
    FTimerHandle CleanupHandle;
    TWeakObjectPtr<UWidgetComponent> WeakPopup(Popup);
    GetWorldTimerManager().SetTimer(CleanupHandle, FTimerDelegate::CreateLambda([WeakPopup]()
    {
        if (WeakPopup.IsValid()) WeakPopup->DestroyComponent();
    }), 1.3f, false);
}

void ABaseCharacter::FaceActor(const AActor* Target)
{
    if (!Target) return;
    FVector Dir = Target->GetActorLocation() - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.IsNearlyZero())
    {
        SetActorRotation(Dir.Rotation());
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
