// UInventoryComponent.cpp
// PROJECT: A Nameless World
// PURPOSE: Implements the four core inventory operations declared in the header.
//
// CODING NOTE:
//   The .h file is the WHAT (function signatures, member variables).
//   This .cpp file is the HOW (the actual logic).
//   If you change how FindItemIndex works internally, you only touch this file —
//   nothing that includes the .h needs to recompile.

#include "Inventory/UInventoryComponent.h"
// Always include the matching header first (its own .h).

// ── Constructor ───────────────────────────────────────────────────────────────

UInventoryComponent::UInventoryComponent()
{
    // Turn off per-frame tick — inventory is event-driven, not per-frame.
    // Every component has tick ON by default. Turning it off saves CPU.
    // We'll use Add/RemoveItem functions instead of updating every frame.
    PrimaryComponentTick.bCanEverTick = false;
}


// ── AddItem ───────────────────────────────────────────────────────────────────

bool UInventoryComponent::AddItem(FInventoryItem NewItem)
{
    // ── Guard: reject items with no ID ───────────────────────────────────────
    // NAME_None is UE5's "empty FName" — like nullptr for pointers.
    // An item with no ID can't be tracked or removed later.
    if (NewItem.ItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UInventoryComponent::AddItem — tried to add item with no ItemID. Rejected."));
        // UE_LOG prints to the editor's Output Log. Format:
        //   LogTemp    — log category (built-in general purpose)
        //   Warning    — severity: shows in yellow in the Output Log
        //   TEXT(...)  — L"..." wide string. Always use TEXT() in UE5 for string literals.
        return false;
    }

    // ── Guard: weight check ───────────────────────────────────────────────────
    // Calculate how much this new item (all its units) would add.
    float IncomingWeight = NewItem.Weight * NewItem.Quantity;

    if (GetTotalWeight() + IncomingWeight > MaxWeight)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UInventoryComponent::AddItem — inventory full (weight limit %.1f). Item: %s"),
            MaxWeight, *NewItem.ItemID.ToString());
        // *NewItem.ItemID.ToString()
        //   .ToString() converts FName → FString
        //   * dereferences the FString to get a raw C-string (const TCHAR*)
        //   UE_LOG's %s format expects a raw C-string, so we need both steps.
        return false;
    }

    // ── Try to stack onto an existing slot ───────────────────────────────────
    // FindItemIndex returns -1 if the item is NOT already in the inventory.
    int32 ExistingIndex = FindItemIndex(NewItem.ItemID);

    if (ExistingIndex != -1)
    {
        // Item already exists — check if there's room in the stack.
        int32 SpaceInStack = Items[ExistingIndex].MaxStackSize - Items[ExistingIndex].Quantity;

        if (SpaceInStack > 0)
        {
            // Add as many as fit. FMath::Min picks the smaller of the two numbers,
            // so we never exceed MaxStackSize even if NewItem.Quantity is large.
            Items[ExistingIndex].Quantity += FMath::Min(NewItem.Quantity, SpaceInStack);
            return true;
            // Example: slot has 7/10 potions. NewItem.Quantity = 5.
            //   SpaceInStack = 10 - 7 = 3
            //   FMath::Min(5, 3) = 3
            //   Result: slot now has 10/10 potions. The remaining 2 are lost.
            //   (Full overflow handling — splitting into two slots — is a V2 feature.)
        }
        // Stack is full — fall through to add a new slot below.
    }

    // ── Add as a new slot ─────────────────────────────────────────────────────
    Items.Add(NewItem);
    // TArray::Add appends to the end. The array grows automatically if needed.
    return true;
}


// ── RemoveItem ────────────────────────────────────────────────────────────────

bool UInventoryComponent::RemoveItem(FName ItemID, int32 QuantityToRemove)
{
    int32 Index = FindItemIndex(ItemID);

    if (Index == -1)
    {
        // Item not found — nothing to remove. Log a warning and return false.
        UE_LOG(LogTemp, Warning,
            TEXT("UInventoryComponent::RemoveItem — item not found: %s"),
            *ItemID.ToString());
        return false;
    }

    // Decrease the quantity in the slot.
    Items[Index].Quantity -= QuantityToRemove;

    if (Items[Index].Quantity <= 0)
    {
        // The slot is empty — remove it from the array entirely.
        // RemoveAt(index) removes the element at that position and shifts
        // all elements after it one step left. The array shrinks by 1.
        Items.RemoveAt(Index);
        // WHY RemoveAt NOT just setting Quantity to 0?
        //   An empty slot with Quantity = 0 would still appear in the inventory
        //   and confuse GetTotalWeight(), HasItem(), and the UI. Remove it clean.
    }

    return true;
}


// ── HasItem ───────────────────────────────────────────────────────────────────

bool UInventoryComponent::HasItem(FName ItemID) const
{
    // FindItemIndex returns -1 if not found, or the index if found.
    // We only care about "found or not" — comparing to -1 gives us that bool.
    return FindItemIndex(ItemID) != -1;
}


// ── GetTotalWeight ────────────────────────────────────────────────────────────

float UInventoryComponent::GetTotalWeight() const
{
    float Total = 0.0f;

    // Range-based for loop — reads every FInventoryItem in the Items array.
    // const FInventoryItem& Item:
    //   const    — we promise not to modify the item (this is a read-only function)
    //   &        — reference: avoids copying each struct (important if structs are large)
    for (const FInventoryItem& Item : Items)
    {
        Total += Item.Weight * Item.Quantity;
        // Slot weight = weight per unit × number of units in this slot.
        // A stack of 5 health potions (0.2f each) adds 1.0f to the total.
    }

    return Total;
}


// ── FindItemIndex (private helper) ────────────────────────────────────────────

int32 UInventoryComponent::FindItemIndex(FName ItemID) const
{
    // Linear search through Items.
    // Items.Num() returns the current number of elements (like .length in other languages).
    // i starts at 0 because arrays in C++ (and UE5) are zero-indexed.
    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (Items[i].ItemID == ItemID)
        {
            return i;
            // Found — return the index immediately (no need to keep searching).
        }
    }

    return -1;
    // -1 is the convention for "not found" with index-based searches.
    // We use -1 because 0 is a valid index (the first slot).
    // Callers always check: if (FindItemIndex(...) != -1) { /* found */ }
    //
    // WHY LINEAR SEARCH and not a TMap?
    //   For a small backpack (< 30 items), linear search is fast enough.
    //   A TMap would give O(1) lookup but requires extra memory and maintenance.
    //   We follow the YAGNI principle: "You Aren't Gonna Need It" — don't
    //   optimise until you have a measured performance problem.
    //   If the inventory grows to hundreds of items, swap to TMap<FName, int32>
    //   and only this one function changes.
}
