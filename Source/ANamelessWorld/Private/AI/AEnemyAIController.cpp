#include "AI/AEnemyAIController.h"
#include "Navigation/CrowdFollowingComponent.h"

// Override the inherited "PathFollowingComponent" subobject with the crowd-aware
// version. The engine's CrowdManager then coordinates avoidance among all
// controllers that use one of these.
AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(
        TEXT("PathFollowingComponent")))
{
}