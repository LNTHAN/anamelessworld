#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UDialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    // The name shown in the dialogue box, such as "Nameless" or "Narrator".
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Dialogue")
    FText SpeakerName;

    // The sentence currently shown to the player.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Dialogue")
    FText LineText;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnDialogueLine, FText, SpeakerName, FText, LineText);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ANAMELESSWORLD_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "ANW|Dialogue")
    FOnDialogueLine OnLineShown;

    UPROPERTY(BlueprintAssignable, Category = "ANW|Dialogue")
    FOnDialogueFinished OnDialogueFinished;

    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    void StartDialogue(const TArray<FDialogueLine>& Lines);

    UFUNCTION(BlueprintCallable, Category = "ANW|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintPure, Category = "ANW|Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintPure, Category = "ANW|Dialogue")
    int32 GetCurrentLineIndex() const;

    UFUNCTION(BlueprintPure, Category = "ANW|Dialogue")
    int32 GetLineCount() const;

private:
    TArray<FDialogueLine> CurrentLines;
    int32 CurrentIndex = -1;
};
