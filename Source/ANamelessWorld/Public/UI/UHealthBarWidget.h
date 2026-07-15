#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UHealthBarWidget.generated.h"

class ABaseCharacter;

// Base class for WBP_HealthBar. Holds a pointer to the character whose HP the
// bar displays. ABaseCharacter sets OwnerCharacter right after it spawns the
// widget; the UMG Percent binding reads GetHealth()/GetMaxHealth() off it.
UCLASS(Abstract, BlueprintType, Blueprintable)
class ANAMELESSWORLD_API UHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Whose health this bar shows. Set from C++ (ABaseCharacter::BeginPlay).
    // BlueprintReadOnly so the WBP_HealthBar Percent binding can read it.
    UPROPERTY(BlueprintReadOnly, Category = "ANW|UI")
    ABaseCharacter* OwnerCharacter = nullptr;
};