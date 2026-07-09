// ATacticalCameraPawn.cpp

#include "Camera/ATacticalCameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

ATacticalCameraPawn::ATacticalCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root pivot — the point the camera orbits and the thing we move when panning.
    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    RootComponent = Pivot;

    // The boom. Pull the camera back and tilt it down for a tactical 3/4 view.
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Pivot);
    SpringArm->TargetArmLength = TargetZoom;
    SpringArm->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f)); // pitch down 50°
    SpringArm->bDoCollisionTest = true;   // auto pull-in past walls
    SpringArm->bEnableCameraLag = true;   // smooth follow when panning
    SpringArm->CameraLagSpeed = 10.f;
    // We drive the boom's rotation ourselves — don't inherit any control rotation.
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw   = false;
    SpringArm->bInheritRoll  = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    // This rig is a camera, not a character — no control-rotation coupling.
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ATacticalCameraPawn::BeginPlay()
{
    Super::BeginPlay();

    // Battlefield center is an authored reference point, not a hardcoded
    // coordinate or something derived from PlayerStart — decouples the
    // establishing shot from wherever combatants happen to be placed.
    FVector CenterLocation = GetActorLocation();
    if (ATargetPoint* Center = Cast<ATargetPoint>(
            UGameplayStatics::GetActorOfClass(this, ATargetPoint::StaticClass())))
    {
        CenterLocation = Center->GetActorLocation();
    }

    // Snap instantly (no easing) so there's no flash of the wrong framing
    // on the very first frame — Tick's easing only applies to LATER changes.
    SetActorLocation(CenterLocation);
    TargetLocation = CenterLocation;
    TargetZoom = WideZoom;
    SpringArm->TargetArmLength = WideZoom;
}

void ATacticalCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("CamPanForward", this, &ATacticalCameraPawn::PanForward);
    PlayerInputComponent->BindAxis("CamPanRight",   this, &ATacticalCameraPawn::PanRight);
    PlayerInputComponent->BindAxis("CamRotate",     this, &ATacticalCameraPawn::Rotate);
    PlayerInputComponent->BindAxis("CamZoom",       this, &ATacticalCameraPawn::Zoom);
}

// Move the pivot along the ground in a camera-relative direction.
void ATacticalCameraPawn::PanAlong(const FVector& WorldDir, float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
    AddActorWorldOffset(WorldDir * Value * PanSpeed * Dt, /*bSweep=*/false);
}

void ATacticalCameraPawn::PanForward(float Value)
{
    // Camera yaw only — flatten pitch so we slide across the floor, not into it.
    const FRotator YawOnly(0.f, SpringArm->GetComponentRotation().Yaw, 0.f);
    PanAlong(YawOnly.Vector(), Value); // forward vector of the yaw
}

void ATacticalCameraPawn::PanRight(float Value)
{
    const FRotator YawOnly(0.f, SpringArm->GetComponentRotation().Yaw, 0.f);
    const FVector Right = FRotationMatrix(YawOnly).GetScaledAxis(EAxis::Y);
    PanAlong(Right, Value);
}

void ATacticalCameraPawn::Rotate(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
    SpringArm->AddRelativeRotation(FRotator(0.f, Value * RotateSpeed * Dt, 0.f));
}

void ATacticalCameraPawn::Zoom(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    // Wheel up (+1) should zoom IN → shorten the boom.
    TargetZoom = FMath::Clamp(TargetZoom - Value * ZoomStep, MinZoom, MaxZoom);
}

void ATacticalCameraPawn::FocusOn(const FVector& WorldLocation)
{
    TargetLocation = WorldLocation;
    TargetZoom = FocusZoom;
    bIsFocusing = true;
}

void ATacticalCameraPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Ease the boom toward the desired length for smooth zoom.
    SpringArm->TargetArmLength =
        FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoom, DeltaSeconds, 10.f);

    // Ease toward the focus target ONLY while a transition is active. Once
    // it converges, stop touching location entirely — checking distance
    // instead of a flag would re-trigger every time manual panning moves you
    // away from the old target, fighting WASD forever instead of just once.
    if (bIsFocusing)
    {
        const FVector NewLocation =
            FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaSeconds, FocusPanSpeed);
        SetActorLocation(NewLocation);

        if (NewLocation.Equals(TargetLocation, 5.f))
        {
            bIsFocusing = false;
        }
    }

    // Camera shake — a short decaying wobble on the camera's own rotation. Done
    // here rather than via PlayerCameraManager shakes, which don't affect this rig.
    if (ShakeTimeRemaining > 0.f)
    {
        ShakeTimeRemaining = FMath::Max(0.f, ShakeTimeRemaining - DeltaSeconds);
        const float Decay   = ShakeTimeRemaining / ShakeDuration;   // 1 → 0
        const float Elapsed = ShakeDuration - ShakeTimeRemaining;
        const float Osc     = FMath::Sin(Elapsed * ShakeFrequency);
        const float Amp     = ShakeAmplitude * Decay;

        Camera->SetRelativeRotation(FRotator(
            Osc * Amp,          // pitch
            Osc * Amp * 0.6f,   // yaw
            Osc * Amp * 1.5f)); // roll — reads most on a top-down view

        if (ShakeTimeRemaining <= 0.f)
        {
            Camera->SetRelativeRotation(FRotator::ZeroRotator); // settle back level
        }
    }
}

void ATacticalCameraPawn::TriggerShake()
{
    ShakeTimeRemaining = ShakeDuration;   // Tick does the rest
}