#include "Dialogue/UDialogueComponent.h"

void UDialogueComponent::StartDialogue(const TArray<FDialogueLine>& Lines)
{
    if (Lines.Num() == 0)
    {
        CurrentLines.Empty();
        CurrentIndex = -1;
        OnDialogueFinished.Broadcast();
        return;
    }

    CurrentLines = Lines;
    CurrentIndex = 0;
    OnLineShown.Broadcast(
        CurrentLines[0].SpeakerName,
        CurrentLines[0].LineText,
        CurrentLines[0].SpeakerPortrait);
}

void UDialogueComponent::AdvanceLine()
{
    if (!IsDialogueActive()) return;

    CurrentIndex++;

    if (CurrentIndex < CurrentLines.Num())
    {
        OnLineShown.Broadcast(
            CurrentLines[CurrentIndex].SpeakerName,
            CurrentLines[CurrentIndex].LineText,
            CurrentLines[CurrentIndex].SpeakerPortrait);
    }
    else
    {
        CurrentIndex = -1;
        CurrentLines.Empty();
        OnDialogueFinished.Broadcast();
    }
}

bool UDialogueComponent::IsDialogueActive() const
{
    return CurrentIndex >= 0;
}

int32 UDialogueComponent::GetCurrentLineIndex() const
{
    return CurrentIndex;
}

int32 UDialogueComponent::GetLineCount() const
{
    return CurrentLines.Num();
}
