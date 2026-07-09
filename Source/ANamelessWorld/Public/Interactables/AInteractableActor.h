// AInteractableActor.h
// PURPOSE: Base class for environmental objects Nameless weaponizes via Interact.
//          Chapter 1 = a rigged bookshelf: a two-stage proximity trap. Nameless
//          ARMS it (stage 1); when an ENEMY later steps into its AoE it SPRINGS
//          (stage 2), crushing EVERY living character in the blast — enemies,
//          allies, and Nameless himself (physics doesn't play favourites). It
//          can even reach the status-immune boss. Nameless doesn't spring his
//          own trap, but he IS crushed if he lingers in the AoE when it goes off.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AInteractableActor.generated.h"

// Pointers only — forward-declare here, #include in the .cpp.
class UStaticMeshComponent;
class USphereComponent;
class UGameplayEffect;
class UPrimitiveComponent;
class ABaseCharacter;
class USceneComponent;

UCLASS()
class ANAMELESSWORLD_API AInteractableActor : public AActor
{
    GENERATED_BODY()

public:
    AInteractableActor();
    // STAGE 1 — the Interact command calls this. Nameless rigs the object: it
    // becomes armed and starts watching its AoE for an enemy to wander in.
    // virtual so other worlds' interactables can arm/behave differently while
    // sharing the Interact input plumbing.
    UFUNCTION(BlueprintCallable, Category = "ANW|Interact")
    virtual void Arm();

    UFUNCTION(BlueprintCallable, Category = "ANW|Interact")
    float GetDistanceToBody(const FVector& FromLocation) const;

    // True if a world point is inside the shelf-aligned blast rectangle.
    bool IsInBlastZone(const FVector& WorldLoc) const;

    // Half-extents of the blast box in the shelf's local X/Y/Z — auto-sized to
    // the mesh, with the thin (front/back) axis expanded by ToppleReach.
    FVector GetBlastHalfExtents() const;

    // Fired the moment the trap springs, right after damage is applied. Has NO
    // C++ body — you implement it in the Blueprint to play the juice (camera
    // shake, particles, sound, the mesh toppling). C++ decides WHEN it detonates;
    // the Blueprint decides how it LOOKS.
    UFUNCTION(BlueprintImplementableEvent, Category = "ANW|Interact")
    void OnDetonated();

    // Fired the instant the trap TRIPS, before the crush — the "!!"/scared beat.
    // Implement in the Blueprint (VFX above the shelf, a creak, a wobble…).
    UFUNCTION(BlueprintImplementableEvent, Category = "ANW|Interact")
    void OnTelegraph();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void BeginPlay() override;

    // Deterministic proximity trigger. Physics overlap events proved unreliable
    // for the trap, so while armed we poll for a living enemy entering range.
    void CheckProximity();
    FTimerHandle ProximityTimerHandle;

    // STAGE 2 — springs the trap: crushes EVERY living character currently inside
    // the AoE (enemies, allies, and Nameless alike), then latches so it can never
    // fire again.
    void Detonate();

    // Bound to TriggerSphere's begin-overlap event. If we're armed and the actor
    // that just entered is a living ENEMY, spring the trap. Enemy-only here (not
    // the damage) so Nameless doesn't detonate it just by arming from inside the
    // AoE. Event-driven — no per-turn polling.
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Interact")
    USceneComponent* Pivot;

    // The visible body (bookshelf mesh) — assigned in the Blueprint.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Interact")
    UStaticMeshComponent* Mesh;

    // The AoE. Its radius IS the crush range, and its overlap events are how the
    // trap detects an enemy entering. Sized from TriggerRadius in BeginPlay.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Interact")
    USphereComponent* TriggerSphere;

    // Crush radius in cm (~300 ≈ 3 m). Pushed onto TriggerSphere in BeginPlay so
    // designers tune one number and both the AoE and the detector stay in sync.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Interact")
    float TriggerRadius = 300.f;

    // How far the shelf topples beyond its own body, front and back (cm). The
    // sideways coverage auto-matches the shelf's length from its mesh bounds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Interact")
    float ToppleReach = 250.f;

    // Damage GameplayEffect applied to each character caught — set to GE_HeavyStrike
    // in the Blueprint. Reuses the existing damage effect, no new formula.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Interact")
    TSubclassOf<UGameplayEffect> DamageEffect;

    // Rigged and waiting for prey. Set true by Arm(), cleared when it springs.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Interact")
    bool bArmed = false;

    // One-shot latch — a toppled shelf is spent, guards Detonate().
    UPROPERTY(BlueprintReadOnly, Category = "ANW|Interact")
    bool bTriggered = false;

    // Beat between tripping (telegraph) and the shelf actually crushing.
    UPROPERTY(EditAnywhere, Category = "ANW|Interact")
    float DetonationDelay = 0.8f;

    FTimerHandle DetonationTimerHandle;

    // Trip sequence: freeze the enemy, play the telegraph, then Detonate after the
    // delay. Called instead of Detonate() directly.
    void BeginDetonation(ABaseCharacter* Trigger);
};