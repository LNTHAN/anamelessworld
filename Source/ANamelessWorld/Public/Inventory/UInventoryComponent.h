// UInventoryComponent.h
// PROJECT: A Nameless World
// PURPOSE: Manages the backpack for any ABaseCharacter.
//          Stores a TArray<FInventoryItem> and exposes Add/Remove/Query functions.
//
// DEPENDENCIES: InventoryTypes.h (FInventoryItem, EItemType)
// DEPENDENTS:   ABaseCharacter.cpp — calls CreateDefaultSubobject<UInventoryComponent>
//               Future: UI widget, save/load system (UCRPGSaveGame)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
// UActorComponent — the base class for all components that attach to actors.
// We inherit from this (not from UObject directly) because:
//   • It gives us BeginPlay, EndPlay lifecycle hooks
//   • It can be registered with an actor via CreateDefaultSubobject
//   • The editor can show it in the Details panel's component list

#include "Inventory/InventoryTypes.h"
// Full include because we STORE FInventoryItem by VALUE in TArray.
// (We can't just forward declare FInventoryItem because we need its full layout
//  to know how much memory the TArray needs.)

#include "UInventoryComponent.generated.h"


// ── Class Declaration ─────────────────────────────────────────────────────────

UCLASS(ClassGroup = (ANW), meta = (BlueprintSpawnableComponent))
// ClassGroup = (ANW): groups this component under "ANW" in the editor's
//   "Add Component" dropdown. Keeps things tidy as the project grows.
// BlueprintSpawnableComponent: allows Blueprints to add this component
//   to any actor via the editor UI (drag and drop onto the character blueprint).

class ANAMELESSWORLD_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    // ── Constructor ──────────────────────────────────────────────────────────
    UInventoryComponent();
    // Sets PrimaryComponentTick.bCanEverTick = false.
    // Inventory updates are EVENT-DRIVEN (add/remove on player action),
    // not per-frame — so we don't need Tick() running every frame.


    // ── Core Operations ──────────────────────────────────────────────────────

    // Add an item to the inventory.
    //   • Checks weight: rejects if adding this item would exceed MaxWeight.
    //   • Handles stacking: if ItemID already exists AND has space, increments
    //     the existing slot's Quantity. Otherwise adds a new slot.
    //   • Returns true if the item was successfully added (or stacked).
    //   • Returns false if the inventory is full (over weight) or ID is invalid.
    UFUNCTION(BlueprintCallable, Category = "ANW|Inventory")
    bool AddItem(FInventoryItem NewItem);

    // Remove a quantity of an item by ID.
    //   • Finds the slot by ItemID, decreases Quantity by QuantityToRemove.
    //   • If Quantity reaches 0 or below, the slot is removed from Items entirely.
    //   • Returns true if the item was found and removed.
    //   • Returns false if ItemID was not in the inventory.
    UFUNCTION(BlueprintCallable, Category = "ANW|Inventory")
    bool RemoveItem(FName ItemID, int32 QuantityToRemove = 1);
    // QuantityToRemove = 1  →  default parameter. Calling RemoveItem("ItemID")
    // without a second argument removes exactly 1. You can pass RemoveItem("ItemID", 5)
    // to remove 5 at once.

    // Returns true if the inventory contains at least 1 of this item.
    // const — this function is read-only, it does not modify any member variables.
    UFUNCTION(BlueprintCallable, Category = "ANW|Inventory")
    bool HasItem(FName ItemID) const;

    // Returns the combined weight of all items currently in the inventory.
    // Formula: sum of (item.Weight * item.Quantity) across all slots.
    // Used to check whether a new item can be added.
    UFUNCTION(BlueprintCallable, Category = "ANW|Inventory")
    float GetTotalWeight() const;

    // Returns a read-only reference to the full item array.
    // UI widgets call this to display the inventory contents.
    // const TArray<>& — reference avoids copying the array; const prevents modification.
    UFUNCTION(BlueprintCallable, Category = "ANW|Inventory")
    const TArray<FInventoryItem>& GetItems() const { return Items; }


protected:

    // ── Data ─────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ANW|Inventory")
    TArray<FInventoryItem> Items;
    // The backpack — a dynamic list of item slots.
    // TArray grows automatically when you call Items.Add().
    // VisibleAnywhere: you can see the items in the editor while Play-testing.
    // BlueprintReadOnly: Blueprint UI can iterate and display these items,
    //                    but cannot add/remove directly (use AddItem/RemoveItem).

    UPROPERTY(EditDefaultsOnly, Category = "ANW|Inventory")
    float MaxWeight = 50.0f;
    // Total carrying capacity in kilograms (or your game's weight unit).
    // Default 50.0f — roughly 10 average items.
    // EditDefaultsOnly: designers can change this per character Blueprint
    //                   (a heavily-armored fighter carries more than a rogue).
    // Later session: this will scale with the STR modifier from UCRPGAttributeSet.


private:

    // ── Private Helpers ───────────────────────────────────────────────────────

    // Searches Items for a slot matching ItemID.
    // Returns the ARRAY INDEX (0, 1, 2...) if found, or -1 if not found.
    //
    // WHY PRIVATE?
    //   This is an IMPLEMENTATION DETAIL. External code should use HasItem()
    //   or AddItem() — they don't need to know that we search by index internally.
    //   Hiding it prevents other classes from depending on how we search.
    //   If we ever switch to a TMap for faster lookup, only this function changes.
    //
    // WHY RETURN INDEX and not a pointer to FInventoryItem?
    //   Because Items is a TArray — if we add/remove items AFTER getting the pointer,
    //   the array might reallocate its memory and the pointer goes invalid.
    //   An index is always safe: Items[index] looks it up fresh each time.
    int32 FindItemIndex(FName ItemID) const;
};
