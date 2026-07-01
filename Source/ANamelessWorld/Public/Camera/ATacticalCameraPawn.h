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

private:
    // --- Input handlers -----------------------------------------------------
    void PanForward(float Value); // W/S — into/out of the screen
    void PanRight(float Value);   // A/D — screen-left/right
    void Rotate(float Value);     // Q/E — orbit the field
    void Zoom(float Value);       // mouse wheel

    // Desired boom length; Zoom() nudges this, Tick eases toward it.
    float TargetZoom = 1400.f;

    // Helper: move the pivot along the ground, relative to current camera yaw.
    void PanAlong(const FVector& WorldDir, float Value);

public:
    virtual void Tick(float DeltaSeconds) override;
};