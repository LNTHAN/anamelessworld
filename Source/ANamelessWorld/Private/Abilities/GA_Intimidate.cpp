// GA_Intimidate.cpp
// PURPOSE: Applies State.Stunned to a target enemy, causing them to lose
//          their next turn. The contested INT vs WIS roll that determines
//          success or failure is wired in Session 13.

#include "Abilities/GA_Intimidate.h"
#include "AbilitySystemComponent.h"
#include "Characters/ABaseCharacter.h"
#include "Utilities/UCRPGCombatLibrary.h"
#include "Attributes/UCRPGAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"


// ════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ════════════════════════════════════════════════════════════════════════════

UGA_Intimidate::UGA_Intimidate()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// ════════════════════════════════════════════════════════════════════════════
// ACTIVATE ABILITY
// ════════════════════════════════════════════════════════════════════════════

void UGA_Intimidate::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("GA_Intimidate: CommitAbility failed."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // The fear radiates from Nameless.
    ABaseCharacter* Caster = Cast<ABaseCharacter>(GetAvatarActorFromActorInfo());
    UWorld* World = Caster ? Caster->GetWorld() : nullptr;
    if (!Caster || !World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Caster's INT modifier — how hard the fear is to resist. Read once.
    int32 INTModifier = 0;
    if (UAbilitySystemComponent* CasterASC = Caster->GetAbilitySystemComponent())
    {
        if (const UCRPGAttributeSet* CA = Cast<UCRPGAttributeSet>(
                CasterASC->GetAttributeSet(UCRPGAttributeSet::StaticClass())))
        {
            INTModifier = UCRPGCombatLibrary::GetModifier(CA->GetIntelligence());
        }
    }

    Caster->SetIsAttacking(true);

    // Every living enemy within IntimidateRadius of Nameless rolls to resist.
    const FVector CasterLoc = Caster->GetActorLocation();

    TArray<AActor*> Everyone;
    UGameplayStatics::GetAllActorsOfClass(World, ABaseCharacter::StaticClass(), Everyone);

    for (AActor* A : Everyone)
    {
        ABaseCharacter* Enemy = Cast<ABaseCharacter>(A);
        if (!Enemy || Enemy->IsPlayerCharacter() || !Enemy->IsAlive()) continue;
        if (FVector::Dist(CasterLoc, Enemy->GetActorLocation()) > IntimidateRadius) continue;

        // Status-immune units (the boss) shrug off fear — never displaced. This
        // skip is ONLY here in the "who gets flung" loop: a status-immune unit can
        // still TAKE collision damage from a mob flung into it (handled in
        // DisplaceEnemy), because that's damage, not a status.
        if (UAbilitySystemComponent* ImmuneCheckASC = Enemy->GetAbilitySystemComponent())
        {
            if (ImmuneCheckASC->HasMatchingGameplayTag(
                    FGameplayTag::RequestGameplayTag(FName("Immunity.Status"))))
            {
                UE_LOG(LogTemp, Log,
                    TEXT("GA_Intimidate: %s is status-immune — not displaced."), *Enemy->GetName());
                continue;
            }
        }

        UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent();
        const UCRPGAttributeSet* EA = EnemyASC ? Cast<UCRPGAttributeSet>(
            EnemyASC->GetAttributeSet(UCRPGAttributeSet::StaticClass())) : nullptr;
        if (!EA) continue;

        const int32 WisdomDC = UCRPGCombatLibrary::CalculateDC(
            UCRPGCombatLibrary::GetModifier(EA->GetWisdom()));
        const int32 RawRoll = UCRPGCombatLibrary::RollD20();
        const int32 FinalRoll = RawRoll + INTModifier;

        UE_LOG(LogTemp, Log, TEXT("GA_Intimidate: %s — %d + %d (INT) = %d vs DC %d."),
            *Enemy->GetName(), RawRoll, INTModifier, FinalRoll, WisdomDC);

        if (FinalRoll >= WisdomDC)
        {
            DisplaceEnemy(Enemy, Caster);   // failed the save → flung back
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("GA_Intimidate: %s held its ground."), *Enemy->GetName());
        }
    }

    // Delay end so the cast animation has time to play.
    FTimerHandle TimerHandle;
    FTimerDelegate Delegate;
    Delegate.BindLambda([this, Handle, ActorInfo, ActivationInfo, Caster]()
    {
        if (Caster) Caster->SetIsAttacking(false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    });
    World->GetTimerManager().SetTimer(TimerHandle, Delegate, 0.8f, false);
}

void UGA_Intimidate::DisplaceEnemy(ABaseCharacter* Enemy, AActor* Caster)
{
    UWorld* World = Enemy ? Enemy->GetWorld() : nullptr;
    if (!World || !Caster) return;

    // Straight away from Nameless, flattened to the ground plane.
    FVector Dir = Enemy->GetActorLocation() - Caster->GetActorLocation();
    Dir.Z = 0.f;
    Dir = Dir.GetSafeNormal();
    if (Dir.IsNearlyZero()) Dir = (-Enemy->GetActorForwardVector()).GetSafeNormal();

    const FVector Start = Enemy->GetActorLocation();
    const FVector End   = Start + Dir * Enemy->MoveRange;

    // Sweep the enemy's own width along the retreat line. ECC_Pawn is blocked by
    // walls, shelves, AND other enemy capsules. Ignore Nameless — he never stops
    // the shove or takes damage from it — and the enemy itself.
    const float Radius = Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Enemy);
    Params.AddIgnoredActor(Caster);

    FHitResult Hit;
    const bool bHit = World->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(Radius), Params);

    // Instant for now — the smooth panic-slide + VFX is the later juice pass.
    const FVector Destination = bHit ? Hit.Location : End;
    Enemy->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);

    if (bHit)
    {
        ApplyImpactDamage(Enemy);                                  // the flung enemy
        if (ABaseCharacter* Other = Cast<ABaseCharacter>(Hit.GetActor()))
        {
            if (!Other->IsPlayerCharacter() && Other->IsAlive())
                ApplyImpactDamage(Other);                          // mutual, if it hit an enemy
        }
        UE_LOG(LogTemp, Log, TEXT("GA_Intimidate: %s slammed into %s."),
            *Enemy->GetName(), *GetNameSafe(Hit.GetActor()));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GA_Intimidate: %s flung into open space."), *Enemy->GetName());
    }
}

void UGA_Intimidate::ApplyImpactDamage(ABaseCharacter* Victim)
{
    UAbilitySystemComponent* ASC = Victim ? Victim->GetAbilitySystemComponent() : nullptr;
    if (!ASC || !DamageEffect) return;

    FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
    Ctx.AddSourceObject(this);
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), Ctx);
    if (Spec.IsValid()) ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}