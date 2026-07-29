#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UDialogueComponent.generated.h"

class UTexture2D;

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

    // Optional portrait for whoever is speaking. Left empty for speakers with no
    // face — a voice, a narrator, a book. Set per line rather than looked up by
    // name, so renaming a speaker can never silently drop their portrait.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Dialogue")
    UTexture2D* SpeakerPortrait = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDialogueLine, FText, SpeakerName, FText, LineText, UTexture2D*, SpeakerPortrait);

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
