// AInteractableActor.cpp

#include "Interactables/AInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/ABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableActor::AInteractableActor()
{
    // No per-frame work — the trap reacts to overlap events, never Tick.
    PrimaryActorTick.bCanEverTick = false;

    // The mesh is the root: everything (and the AoE sphere) hangs off the body.
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    // The AoE + trip-wire. Query-only (no physics push), and set to OVERLAP the
    // Pawn channel so characters walking in fire begin-overlap events. Radius is
    // finalised in BeginPlay from TriggerRadius.
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(Mesh);
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

void AInteractableActor::Arm()
{
    if (bTriggered || bArmed) return;

    bArmed = true;
    UE_LOG(LogTemp, Log, TEXT("AInteractableActor: %s ARMED."), *GetName());

    // Begin-overlap only fires on ENTRY, so an enemy already standing in range
    // at arm-time wouldn't trip it. Check current occupants (by body-distance,
    // not the origin sphere) and spring immediately if a living enemy is inside.
    TArray<AActor*> Everyone;
    UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Everyone);
    for (AActor* A : Everyone)
    {
        ABaseCharacter* Char = Cast<ABaseCharacter>(A);
        if (Char && !Char->IsPlayerCharacter() && Char->IsAlive()
            && GetDistanceToBody(Char->GetActorLocation()) <= TriggerRadius)
        {
            Detonate();
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

void AInteractableActor::Detonate()
{
    if (bTriggered) return;                // one-shot guard
    bTriggered = true;
    bArmed = false;

    // Stop detecting further entries — the shelf is down.
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Everyone within blast range of the shelf's BODY — enemies, allies, Nameless.
    TArray<AActor*> Everyone;
    UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Everyone);

    for (AActor* A : Everyone)
    {
        ABaseCharacter* Victim = Cast<ABaseCharacter>(A);
        if (!Victim || !Victim->IsAlive()) continue;
        if (GetDistanceToBody(Victim->GetActorLocation()) > TriggerRadius) continue;

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
}