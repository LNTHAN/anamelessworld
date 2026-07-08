// AEnemyAIController.h
// PURPOSE: Enemy AIController that uses Detour Crowd avoidance so agents steer
//          around each other (and other standing characters) during moves,
//          instead of jamming. It swaps the AIController's default
//          PathFollowingComponent for a UCrowdFollowingComponent.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AEnemyAIController.generated.h"

UCLASS()
class ANAMELESSWORLD_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController(const FObjectInitializer& ObjectInitializer);
};