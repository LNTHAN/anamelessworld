#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UFloatingCombatText.generated.h"

// Base class for WBP_FloatingText — the transient "damage number / Miss! / Immune"
// popup that rises over a unit's head. C++ spawns one per hit and calls Init();
// the WBP implements Init visually (set the text + colour, play a rise/fade anim).
UCLASS(Abstract, BlueprintType, Blueprintable)
class ANAMELESSWORLD_API UFloatingCombatText : public UUserWidget
{
    GENERATED_BODY()

public:
    // C++ → BP handoff. BlueprintImplementableEvent = declared here, drawn in the WBP.
    UFUNCTION(BlueprintImplementableEvent, Category = "ANW|Feedback")
    void Init(const FText& Text, FLinearColor Color);
};