#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDialogueWidget.generated.h"

class UTextBlock;

UCLASS(Abstract, BlueprintType, Blueprintable)
class ANAMELESSWORLD_API UDialogueWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    void ShowLine(const FText& SpeakerName, const FText& LineText);

    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    void HideDialogue();

protected:
    // In WBP_DialogueBox, create a TextBlock with this exact name.
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* SpeakerNameText;

    // In WBP_DialogueBox, create a TextBlock with this exact name.
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* DialogueLineText;
};
