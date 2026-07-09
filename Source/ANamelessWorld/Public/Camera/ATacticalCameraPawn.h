// ATacticalCameraPawn.h
// PURPOSE: A free-flying tactical camera rig. The PlayerController possesses
//          THIS (not the character), so the camera pans/rotates/zooms over the
//          whole battlefield independently of where Nameless stands.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ATacticalCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class ANAMELESSWORLD_API ATacticalCameraPawn : public APawn
{
    GENERATED_BODY()

public:
    ATacticalCameraPawn();

    virtual void BeginPlay() override;
    
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;

    // --- Components ---------------------------------------------------------

    // Root. The point we orbit and pan around. Moving this Pawn moves the pivot.
    UPROPERTY(VisibleAnywhere, Category = "ANW|Camera")
    USceneComponent* Pivot;

    // The boom. Its rotation = orbit; TargetArmLength = zoom. Handles collision.
    UPROPERTY(VisibleAnywhere, Category = "ANW|Camera")
    USpringArmComponent* SpringArm;

    // The actual camera on the end of the boom.
    UPROPERTY(VisibleAnywhere, Category = "ANW|Camera")
    UCameraComponent* Camera;

    // --- Tunables (edit in-editor, no recompile) ---------------------------

    // Ground-plane pan speed in unreal units / second.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float PanSpeed = 2000.f;

    // Orbit speed in degrees / second.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float RotateSpeed = 120.f;

    // How much one mouse-wheel notch changes the boom length.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float ZoomStep = 200.f;

    // How far in/out the boom may travel.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float MinZoom = 600.f;

    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float MaxZoom = 2800.f;

    // Zoom used for the establishing shot (BeginPlay) and whenever nothing
    // specific needs focus. Wide enough to see the whole battlefield.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float WideZoom = 2200.f;

    // Zoom used when focusing on the active combatant each turn — tighter
    // than WideZoom so you can actually tell who's doing what.
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float FocusZoom = 1200.f;

    // How fast the camera eases toward TargetLocation when focusing on a
    // combatant. Separate from PanSpeed (that's manual WASD panning).
    UPROPERTY(EditAnywhere, Category = "ANW|Camera|Tuning")
    float FocusPanSpeed = 4.f;

    // Smoothly re-centers the camera on a world location and switches to
    // FocusZoom. Called by the controller whenever a combatant's turn starts.
    void FocusOn(const FVector& WorldLocation);

    // Kicks off a short camera shake. Called from Blueprints (the rigged shelf's
    // OnDetonated) for impact. Rotates our own camera component directly — the
    // PlayerCameraManager shake system doesn't apply to this decoupled rig.
    UFUNCTION(BlueprintCallable, Category = "ANW|Camera")
    void TriggerShake();

private:
    // --- Input handlers -----------------------------------------------------
    void PanForward(float Value); // W/S — into/out of the screen
    void PanRight(float Value);   // A/D — screen-left/right
    void Rotate(float Value);     // Q/E — orbit the field
    void Zoom(float Value);       // mouse wheel

    // Where the pivot is easing toward. Set by FocusOn(); nothing else
    // touches it, so manual WASD panning during a turn is never fought.
    FVector TargetLocation;

    // True only while actively easing toward TargetLocation after a FocusOn()
    // call. Set false once the transition completes — after that, Tick never
    // touches location again until the next FocusOn(), so manual panning is
    // permanently free in between.
    bool bIsFocusing = false;

    // Desired boom length; Zoom() nudges this, Tick eases toward it.
    // Starts at MinZoom (not some arbitrary larger value) so spawning near a
    // wall/corner never causes the spring-arm's collision pull-in to yank the
    // camera into geometry — the player can freely scroll out from here.
    float TargetZoom = 600.f;

    // Helper: move the pivot along the ground, relative to current camera yaw.
    void PanAlong(const FVector& WorldDir, float Value);

    UPROPERTY(EditAnywhere, Category = "ANW|Camera")
    float ShakeDuration = 0.35f;

    UPROPERTY(EditAnywhere, Category = "ANW|Camera")
    float ShakeAmplitude = 3.0f;    // peak wobble in degrees

    UPROPERTY(EditAnywhere, Category = "ANW|Camera")
    float ShakeFrequency = 30.0f;

    float ShakeTimeRemaining = 0.0f;

public:
    virtual void Tick(float DeltaSeconds) override;
};