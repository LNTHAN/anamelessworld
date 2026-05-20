// InventoryTypes.h
// PROJECT: A Nameless World
// PURPOSE: Declares FInventoryItem (one item slot in the backpack) and
//          EItemType (the category of an item: Weapon, Armor, Consumable, etc.)
//
// WHY IS THIS A SEPARATE FILE?
//   Both UInventoryComponent AND future UI classes (like the inventory screen
//   widget) need FInventoryItem. If it lived inside UInventoryComponent.h,
//   those UI classes would have to #include the whole component just to read
//   one struct. Separating it means any class can include just this file.
//
// CODING ORDER NOTE:
//   This is included by UInventoryComponent.h.
//   It has no dependencies on any other class in this project.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"
// .generated.h is required even for structs and enums because USTRUCT and
// UENUM use UPROPERTY and reflection — UHT must process them.


// ── Item Category Enum ────────────────────────────────────────────────────────

UENUM(BlueprintType)
// BlueprintType: allows Blueprint code (UI widgets, etc.) to read this enum.
enum class EItemType : uint8
// : uint8  — stores the enum as a single byte instead of a 4-byte int.
//            Required by UE5 for any UENUM used with Blueprint.
{
    Weapon      UMETA(DisplayName = "Weapon"),
    // Physical or magical weapon. Usually MaxStackSize = 1 (you can't stack two swords).

    Armor       UMETA(DisplayName = "Armor"),
    // Equippable protection. MaxStackSize = 1.

    Consumable  UMETA(DisplayName = "Consumable"),
    // Potions, food, scrolls. Can stack (MaxStackSize = 10 or higher).

    QuestItem   UMETA(DisplayName = "Quest Item"),
    // Key items that cannot be dropped or destroyed.

    Misc        UMETA(DisplayName = "Miscellaneous")
    // Crafting materials, vendor trash. Default type.
    // UMETA(DisplayName = ...) sets the human-readable name shown in the editor.
};


// ── Item Struct ───────────────────────────────────────────────────────────────
//
// WHY A STRUCT (F prefix) and not a UCLASS (U prefix)?
//
//   FInventoryItem is PURE DATA — it has no methods, no lifecycle, no GC needs.
//   Storing it as a USTRUCT means:
//     • It lives INSIDE TArray<FInventoryItem> without extra heap allocations
//     • No Garbage Collector overhead — it's plain memory, like a C struct
//     • Copying it is fast (just copy the bytes)
//
//   If this were a UObject it would have to be:
//     • Heap-allocated (new UInventoryItem())
//     • Tracked by the GC (UPROPERTY required to avoid being deleted)
//     • Stored as TArray<UInventoryItem*> — an array of POINTERS, not values
//
//   Rule of thumb: if it's just data with no UE5 features needed → STRUCT.

USTRUCT(BlueprintType)
// BlueprintType: Blueprint code can create and read this struct (e.g. in UI widgets).
struct ANAMELESSWORLD_API FInventoryItem
{
    GENERATED_BODY()
    // Required inside every USTRUCT, just like UCLASS.
    // Pastes in the reflection boilerplate that UPROPERTY needs.

    // ── Identity ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    FName ItemID = NAME_None;
    // FName is a hashed string — faster to compare than FString.
    // This ID matches a row name in a DataTable (set up in a later session).
    // NAME_None is the "empty/unset" value for FName (like nullptr for pointers).
    // WHY FName NOT FString? We compare ItemIDs constantly (HasItem, FindItem).
    //   FName comparison is O(1) — two integer compares.
    //   FString comparison is O(n) — walks every character. FName wins for IDs.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    FString DisplayName = TEXT("Unknown Item");
    // Human-readable name shown in the inventory UI. FString is fine here
    // because we display it rarely (only on hover/select), not compare it.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    FString Description = TEXT("");
    // Flavour text shown in the tooltip or item detail panel.


    // ── Classification ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    EItemType ItemType = EItemType::Misc;
    // Default to Misc so new items don't break the inventory before being set up.


    // ── Stack ─────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    int32 Quantity = 1;
    // How many of this item are in this slot.
    // One slot can hold multiple copies IF Quantity < MaxStackSize.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    int32 MaxStackSize = 1;
    // How many can stack in one slot.
    // Weapons/Armor = 1 (can't stack two swords in one slot).
    // Consumables = 10 (one slot can hold 10 health potions).


    // ── Weight ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ANW|Inventory")
    float Weight = 0.5f;
    // Weight per SINGLE item. Total weight of this slot = Weight * Quantity.
    // Carrying capacity is checked against STR modifier in UInventoryComponent.
    // f suffix on 0.5 makes it a float literal (not a double).


    // ── State ─────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "ANW|Inventory")
    bool bIsEquipped = false;
    // Only one weapon and one armor can be equipped at a time (enforced by
    // UInventoryComponent::EquipItem in a later session).
    // BlueprintReadOnly — the UI can show whether this is equipped,
    // but cannot set it directly; that must go through the component function.
};
