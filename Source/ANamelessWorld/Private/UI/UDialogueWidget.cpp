#include "UI/UDialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UDialogueWidget::ShowLine(const FText& SpeakerName, const FText& LineText, UTexture2D* SpeakerPortrait)
{
    if (SpeakerNameText)
    {
        SpeakerNameText->SetText(SpeakerName);
    }

    if (DialogueLineText)
    {
        DialogueLineText->SetText(LineText);
    }

    if (SpeakerPortraitImage)
    {
        // Collapse rather than show an empty frame — a faceless speaker should
        // give the text the full width, not leave a hole where a portrait isn't.
        if (SpeakerPortrait)
        {
            SpeakerPortraitImage->SetBrushFromTexture(SpeakerPortrait);
            SpeakerPortraitImage->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            SpeakerPortraitImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UDialogueWidget::HideDialogue()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
