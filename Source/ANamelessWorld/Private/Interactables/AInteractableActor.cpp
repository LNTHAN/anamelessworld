// AInteractableActor.cpp

#include "Interactables/AInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/ABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AIController.h"
#include "Engine/StaticMesh.h"

AInteractableActor::AInteractableActor()
{
    // A hinge at the base. We rotate THIS to topple, so the shelf swings around
    // its bottom edge instead of spinning about its own center.
    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    SetRootComponent(Pivot);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Pivot);

    // The AoE + trip-wire. Query-only (no physics push), and set to OVERLAP the
    // Pawn channel so characters walking in fire begin-overlap events. Radius is
    // finalised in BeginPlay from TriggerRadius.
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(Pivot);

    TriggerSphere->SetUsingAbsoluteScale(true); // ignore mesh scale, keep the AoE radius constant
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    // GetOverlappingActors and begin-overlap only work if this is on (it isn't
    // by default). Needed for the future "enemy walks into the AoE" auto-trigger.
    TriggerSphere->SetGenerateOverlapEvents(true);
}

float AInteractableActor::GetDistanceToBody(const FVector& FromLocation) const
{
    if (Mesh)
    {
        FVector ClosestPoint;
        // >=0 : distance to the mesh surface (0 = inside). <0 : no simple
        // collision to test against — fall back to the origin below.
        const float Dist = Mesh->GetClosestPointOnCollision(FromLocation, ClosestPoint);
        if (Dist >= 0.f) return Dist;
    }
    return FVector::Dist(FromLocation, GetActorLocation());
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();

    // One number (TriggerRadius) drives both the visual AoE and the detector.
    TriggerSphere->SetSphereRadius(TriggerRadius);

    // Start listening for anything stepping into the AoE.
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AInteractableActor::OnSphereBeginOverlap);
}

void AInteractableActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Lift the mesh so its base sits on the Pivot. Done here (not BeginPlay) so it
    // shows in the EDITOR too — placement becomes WYSIWYG.
    if (Mesh && Mesh->GetStaticMesh())
    {
        const float HalfHeight =
            Mesh->GetStaticMesh()->GetBounds().BoxExtent.Z * Mesh->GetComponentScale().Z;
        Mesh->SetRelativeLocation(FVector(0.f, 0.f, HalfHeight));
    }
}

void AInteractableActor::Arm()
{
    if (bTriggered || bArmed) return;

    bArmed = true;
    UE_LOG(LogTemp, Log, TEXT("AInteractableActor: %s ARMED."), *GetName());

    // Detonate now if an enemy is already in range; otherwise poll for one to
    // walk in. A light repeating distance check (the overlap-event path was
    // unreliable) — it stops itself the moment the trap springs.
    CheckProximity();
    if (bArmed && !bTriggered)
    {
        GetWorldTimerManager().SetTimer(ProximityTimerHandle, this,
            &AInteractableActor::CheckProximity, 0.15f, true);
    }
}

void AInteractableActor::CheckProximity()
{
    if (!bArmed || bTriggered) return;

    TArray<AActor*> Everyone;
    UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Everyone);
    for (AActor* A : Everyone)
    {
        ABaseCharacter* Char = Cast<ABaseCharacter>(A);
        if (Char && !Char->IsPlayerCharacter() && Char->IsAlive()
            && IsInBlastZone(Char->GetActorLocation()))
        {
            BeginDetonation(Char); 
            return;
        }
    }
}

void AInteractableActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Only an armed, unspent shelf reacts.
    if (!bArmed || bTriggered) return;

    ABaseCharacter* Char = Cast<ABaseCharacter>(OtherActor);
    if (!Char) return;                     // not a combatant (e.g. the camera rig)

    // Nameless doesn't spring his own trap (he'd self-detonate on arm otherwise);
    // only a living enemy trips it. The DAMAGE, in Detonate(), still hits everyone.
    if (Char->IsPlayerCharacter() || !Char->IsAlive()) return;

    Detonate();
}

FVector AInteractableActor::GetBlastHalfExtents() const
{
    // Shelf's own half-size (accounting for its scale).
    FVector Half = (Mesh && Mesh->GetStaticMesh())
        ? Mesh->GetStaticMesh()->GetBounds().BoxExtent * Mesh->GetComponentScale()
        : FVector(50.f, 50.f, 100.f);

    // Expand the SHORTER horizontal axis (the front/back topple direction).
    if (Half.X <= Half.Y) Half.X += ToppleReach;
    else                  Half.Y += ToppleReach;

    Half.Z = 100.f;   // a fixed vertical band; height doesn't matter here
    return Half;
}

bool AInteractableActor::IsInBlastZone(const FVector& WorldLoc) const
{
    const FVector Rel  = WorldLoc - GetActorLocation();
    const FVector Half = GetBlastHalfExtents();
    const float AlongX = FMath::Abs(FVector::DotProduct(Rel, GetActorForwardVector()));
    const float AlongY = FMath::Abs(FVector::DotProduct(Rel, GetActorRightVector()));
    return AlongX <= Half.X && AlongY <= Half.Y;
}

void AInteractableActor::Detonate()
{
    if (bTriggered) return;                // one-shot guard
    bTriggered = true;
    bArmed = false;

    GetWorldTimerManager().ClearTimer(ProximityTimerHandle);

    // Stop detecting further entries — the shelf is down.
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Everyone within blast range of the shelf's BODY — enemies, allies, Nameless.
    TArray<AActor*> Everyone;
    UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Everyone);

    for (AActor* A : Everyone)
    {
        ABaseCharacter* Victim = Cast<ABaseCharacter>(A);
        if (!Victim || !Victim->IsAlive()) continue;
        if (!IsInBlastZone(Victim->GetActorLocation())) continue;

        UAbilitySystemComponent* VictimASC = Victim->GetAbilitySystemComponent();
        if (!VictimASC || !DamageEffect) continue;

        // No ASC of our own — build the spec from the victim's ASC and apply it
        // to that same victim. Source object = this shelf, for logging/attribution.
        FGameplayEffectContextHandle Context = VictimASC->MakeEffectContext();
        Context.AddSourceObject(this);

        FGameplayEffectSpecHandle Spec =
            VictimASC->MakeOutgoingSpec(DamageEffect, 1.f, Context);

        if (Spec.IsValid())
        {
            VictimASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            UE_LOG(LogTemp, Log, TEXT("AInteractableActor: %s crushed %s."),
                *GetName(), *Victim->GetName());
        }
    }

    // Presentation hook — the Blueprint plays the shake / VFX / sound / topple.
    OnDetonated();
}

void AInteractableActor::BeginDetonation(ABaseCharacter* Trigger)
{
    if (bTriggered) return;

    // Stop re-checking / being armed so nothing re-trips during the sequence.
    bArmed = false;
    GetWorldTimerManager().ClearTimer(ProximityTimerHandle);

    // Freeze the enemy that tripped it — it stops IN the blast rather than the
    // shelf detonating mid-stride. This is your "enemy stops in range" beat.
    if (Trigger)
    {
        if (AAIController* AI = Cast<AAIController>(Trigger->GetController()))
        {
            AI->StopMovement();
        }
    }

    // Telegraph now (Blueprint plays the "!!"/scared cue); crush after the beat.
    OnTelegraph();
    GetWorldTimerManager().SetTimer(DetonationTimerHandle, this,
        &AInteractableActor::Detonate, DetonationDelay, false);
}