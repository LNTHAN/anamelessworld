# A Nameless World — Roadmap

## Philosophy: Depth-first
**Phase 1 = build World 1 / Chapter 1 to near-final quality** — real movement, environment, the full manipulation kit, balanced and *fun*. **Phase 2 = breadth** — more worlds, abilities, companions. Building one chapter end-to-end exercises every system we'll reuse and avoids throwaway prototypes.

## Flow (Chapter 1)
Menu → Intro Slides → Chapter Card → Pre-battle Dialogue → Battle Commenced → **Tactical Battle** → Ending Slides → World Finished → Chapter 2 Teaser

---

## Sessions Completed (1–22)
| # | Topic | Status |
|---|---|---|
| 1–6 | AttributeSet, ABaseCharacter+ASC, Inventory, TurnManager, GA_BasicAttack, end-to-end damage | ✓ |
| 7–10 | Player/Enemy subclasses, win/lose + HUD, enemy AI + Boss, command menu + cycle target | ✓ |
| 11 | Data Assets (UCRPGCharacterData, per-character DAs) | ✓ |
| 12–13 | Protagonist kit (Embolden/Intimidate/Provoke), d20 dice system | ✓ |
| 14–17 | Character models + anim wiring, battle log, turn-order strip, turn sequencing fix | ✓ |
| 18–19 | Dialogue system + FFT-style styling | ✓ |
| 20 | Main menu, intro cutscene, BGM via GameInstance | ✓ |
| 22 | Chapter-loop assets + screen widgets, Cinzel font, reusable cutscene widget, battle-start wiring | ✓ |

> Note: the original Phase-1 "complete a basic first battle" goal is superseded by the depth-first plan below. Sessions 1–22 built the foundation; the combat is now being redesigned around tactical movement + manipulation (see DESIGN_RATIONALE §6).

---

## Phase 1 — World 1 / Chapter 1 (to near-final quality)

### Block G — Movement foundation
Radius-based (BG3-style) free movement. **Action economy = Move + one Action per turn** (separate stocks, any order, skippable). Movement-range preview. Turn-manager integration. Enemy movement. *The biggest new system — several sessions.*

### Block H — Manipulation kit (final Chapter 1 design)
- Retire Nameless's direct damage (remove Basic / Heavy Strike).
- **Confuse** (rework Provoke): target attacks the **nearest creature** (incl. boss); damage-buff + low accuracy (Disadvantage).
- **Intimidate** (rework): **displacement + AoE fear** — control/repositioning, not a second disable.
- Command menu → **Move / Confuse / Intimidate / Interact**.
- Embolden deferred to Chapter 2.

### Block I — Environment & Interact
Interactable-object framework + **Interact** command. Chapter 1 = rigged **bookshelf** (arm → crushes enemies in its AoE). A second damage source able to hit the status-immune boss.

### Block I2 — Data architecture (spreadsheet-driven)
Define `USTRUCT` row types; build **DataTables** (Characters, Abilities) imported from **CSV** (Excel/Sheets workflow). Migrate stats/MoveRange/cooldowns/ability params off Data Assets so balancing is CRUD-in-a-sheet. Lands **before** Block J so the balance pass uses it. *(See DESIGN_RATIONALE §8.)*

### Block J — AI, boss & encounter
Tactical enemy AI (move-to-target, confused targeting, boss pursuit). **Boss = fast/ranged, immune to status** — the unkitable clock. Encounter layout, mob count, and the balance pass → tight, winnable, fun.

### Block K — Clarity & feedback (the "fun" polish)
Movement-range & ability-range/AoE indicators, threat telegraphs, status icons, hit/miss/damage feedback, dice-roll visualization, ability VFX, confusion + bookshelf-crash visuals.

### Block L — Cinematic chapter loop
Chapter card + Battle Commenced (done). Ending slides → World Finished → Chapter 2 teaser, reusing the configurable cutscene widget.

### Block M — Onboarding
Teach the manipulation loop. **Last** — only once mechanics are frozen.

---

## Phase 2 — Breadth
- More worlds (each with signature interactables, hazards, and palette)
- New abilities; **companions with unique kits** (Embolden returns here)
- Branching dialogue + choices
- Multiple battles per chapter

---

## Locked Combat Design (see DESIGN_RATIONALE §6)
- Nameless = **pure manipulation**, zero direct damage
- **Confuse** (nearest-creature, dmg-buff + low accuracy) replaces Provoke
- **Boss immune to status** → killed via confused mobs + environment, never directly
- **Intimidate** = displacement / AoE control
- **Embolden** → Chapter 2 (needs a companion)
- **Interact** = core verb (Ch1 = bookshelves)
- **Movement** ≈ equal; safety from control + terrain + action economy, never raw speed

---

## Asset Plan
- **Character models + animations:** Mixamo (free) — import FBX into UE5, retarget to UE5 mannequin skeleton
- **BGM:** Suno AI generated, modern fantasy cinematic, per-chapter tracks that get lighter as protagonist changes
- **UI art:** Custom AI-generated assets (button, logo, background)
