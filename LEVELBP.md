# TestLevel — Level Blueprint Map

Structural map of `Content/TestLevel.umap`'s Level Blueprint: the integration hub where the
TurnManager, HUD widgets, dialogue, and combat start-up are wired together.

**Why this file exists:** Blueprint graphs are binary `.uasset` — unreadable outside the editor.
This is the text mirror, so the exec flow can be reasoned about (and reviewed) without opening UE.

**Maintenance:** update whenever the Level BP's *structure* changes (new events, reordered exec,
added/removed calls) — not for cosmetic node moves. Last verified: **2026-07-16**.

---

## Variables (Level BP)

| Variable | Type | Set in |
|---|---|---|
| `Turn Manager` | `UTurnManager` | BeginPlay (Construct Object, Outer = self) |
| `HUD` | `WBP_BattleHUD` | BeginPlay |
| `Dialogue Box Widget` | `WBP_DialogueBox` | BeginPlay |
| `Chapter Card Widget` | `WBP_ChapterCard` | BeginPlay |
| `Battle Commenced Widget` | `WBP_BattleCommenced` | OnDialogueFinished |
| `Command Menu` | `WBP_CommandMenu` | OnDialogueFinished |

Actor references come from the Persistent Level: `BP_Player`, `BP_Enemy`, `BP_Boss`
(the two added mobs need **no** references — see `Register World Combatants`).
`WBP_AbilityForecast` is created and added to viewport but **not** stored in a variable.

---

## Chain 1 — `Event BeginPlay`

```
Construct Turn Manager (Class=TurnManager, Outer=self) → SET Turn Manager
Register World Combatants (Target = Turn Manager)          ← auto-registers EVERY ABaseCharacter
Create WBP_BattleHUD → SET HUD
  SET HUD.Player Character = BP_Player
  SET HUD.Enemy Character  = BP_Enemy
  SET HUD.Boss Character   = BP_Boss
  SET HUD.Turn Manager     = Turn Manager
Add to Viewport (HUD) → SET HUD Visibility = Hidden
Create WBP_DialogueBox → Add to Viewport → Set Visibility = Collapsed → SET Dialogue Box Widget
Bind Event to On Line Shown       (BP_Player → Dialogue Comp) → OnDialogueStarted(SpeakerName, LineText)
Bind Event to On Dialogue Finished(BP_Player → Dialogue Comp) → OnDialogueFinished()
Create WBP_ChapterCard → SET Chapter Card Widget → Add to Viewport
Delay 3.0 → Remove from Parent (Chapter Card Widget)
Start Dialogue (Target = BP_Player Dialogue Component, Lines = Make Array [3 lines])
```

**Opening dialogue lines** (authored inline as `Make Dialogue Line` nodes):
0. *The Narrator* — "You were never meant to survive this page."
1. *Nameless* — "Then why am I still here?"
2. *The Narrator* — "An interesting question. Let's find out."

---

## Chain 2 — `OnDialogueStarted` (Custom Event: SpeakerName, LineText)

```
Dialogue Box Widget → Show Line (Speaker Name, Line Text)
```
Thin forwarder from the dialogue component to the dialogue widget.

---

## Chain 3 — `OnDialogueFinished` (Custom Event) — the battle-start sequence

```
Hide Dialogue (Dialogue Box Widget)
Create WBP_BattleCommenced → SET Battle Commenced Widget → Add to Viewport
Delay 2.0
Remove from Parent (Chapter Card Widget)
SET HUD Visibility = Visible
Bind to Turn Manager  [Target: Cast(GetPlayerController(0)) → TacticalPlayerController; arg: Turn Manager]
Start Combat (Turn Manager)
Refresh Strip  [Target: HUD → Turn Order Strip; arg: Get Turn Order (Turn Manager)]
Create WBP_AbilityForecast → Add to Viewport
Create WBP_CommandMenu → SET Command Menu
Setup Command Menu (In Player Character = BP_Player, In Tactical Controller = Cast → TacticalPlayerController)
Add to Viewport (Command Menu)
Bind Event to On Combat Ended (Turn Manager) → OnCombatEnded_Event
```

---

## Chain 4 — `OnCombatEnded_Event` (Custom Event: Player Won)

```
Branch (Player Won)
  True  → Print String "Player Won!"    [Development Only]
  False → Print String "Game Over..."   [Development Only]
```

---

## Ordering constraints (don't break these)

- **`Register World Combatants` must run before `Start Combat`.** Satisfied by construction:
  registration is in BeginPlay, `Start Combat` is in OnDialogueFinished (much later).
- **`Bind to Turn Manager` must precede `Start Combat`** — the controller has to be subscribed before
  the first `OnTurnStarted` fires, or the camera never focuses the opening turn.
- **`Setup Command Menu` runs AFTER `Start Combat`** — which is exactly why `WBP_CommandMenu` needs its
  internal *catch-up* call (evaluate `GetCurrentCombatant()` once on bind). It misses the first
  `OnTurnStarted` broadcast by design of this ordering. Don't "fix" it by reordering; the catch-up handles it.
- **HUD starts Hidden** (BeginPlay) and is revealed in OnDialogueFinished — all battle UI appears together.

## Known redundancy

- `Remove from Parent (Chapter Card Widget)` appears in **both** BeginPlay (after Delay 3.0) and
  OnDialogueFinished. The second is a harmless no-op. Left as-is; tidy only if touching that area.

## Future touch points

- **Block L** — `OnCombatEnded_Event` is the win/lose hook. Replace the two Print Strings with the real
  screens (win / lose → "World N … Finished" → Chapter 2 teaser). The plumbing already exists.
- **Block N** — onboarding will hook the dialogue chain, and needs the *scripted turn order* override
  (Nameless → mobs nearest-first → boss) as an optional mode on `UTurnManager`.
