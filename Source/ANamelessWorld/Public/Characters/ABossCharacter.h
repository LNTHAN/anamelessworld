// ABossCharacter.h
// PURPOSE: The boss enemy — higher stats, uses Heavy Strike more often.
//          Inherits all AI logic from AEnemyCharacter.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AEnemyCharacter.h"
#include "ABossCharacter.generated.h"

UCLASS()
class ANAMELESSWORLD_API ABossCharacter : public AEnemyCharacter
{
    GENERATED_BODY()

public:
    ABossCharacter();
};