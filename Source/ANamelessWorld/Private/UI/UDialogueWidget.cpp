#include "UI/UDialogueWidget.h"

#include "Components/TextBlock.h"

void UDialogueWidget::ShowLine(const FText& SpeakerName, const FText& LineText)
{
    if (SpeakerNameText)
    {
        SpeakerNameText->SetText(SpeakerName);
    }

    if (DialogueLineText)
    {
        DialogueLineText->SetText(LineText);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UDialogueWidget::HideDialogue()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
