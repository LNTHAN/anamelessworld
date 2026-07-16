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

## Delivery schedule — LOCKED to Aug 15 2026 (set 2026-07-16)
Coding-school final; passable demonstrable build due **Aug 15 2026**. Critical path ≈ 11–13 sessions, so there is margin — the real risk is scope creep in polish (K) and art (M), not time.
- **Wk1 (Jul 16–22) — Block J:** status immunity + 3 mobs + layout + balance → winnable boss fight.
- **Wk2 (Jul 23–29) — Block L finish:** win/lose + "World Finished" + endings + Ch2 teaser → **complete playable loop (passable-build milestone).**
- **Wk3 (Jul 30–Aug 5):** Block K legibility (enemy-intent indicators + damage numbers) + start Block N onboarding.
- **Wk4 (Aug 6–12):** finish onboarding + **light** Block M art (floor material + one real shelf) + H2 if time.
- **Aug 13–15:** buffer — playtest / balance / bug-fix, no new features.
- **Cut order if slipping:** H2 → full M art (keep light) → Tier-2 danger overlays.

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
**Phase 1 — level geometry first:** block out Chapter 1's actual room (walls, floor shape, furniture placement including a bookshelf mesh) — level-design/editor work, not code. Both this block's own Interact framework and Block H part 2 (Intimidate's displacement/AoE) need real terrain/hazards to be testable against anything more than an empty rectangle; building either against the bare TestLevel floor would mean redoing it once the map exists.
**Phase 2 — the systems:** interactable-object framework + **Interact** command. Chapter 1 = rigged **bookshelf** (arm → crushes enemies in its AoE) — a second damage source able to hit the status-immune boss.

### Block I2 — Data architecture (spreadsheet-driven) — ✓ DONE (Characters table)
Define `USTRUCT` row types; build **DataTables** (Characters, Abilities) imported from **CSV** (Excel/Sheets workflow). *Status: `FCRPGCharacterRow` + `DT_Characters` (3 rows) done, read at spawn via `InitStatsFromRow`; CSV import deferred (editor grid is enough for now); an Abilities table is not yet needed. `bStatusImmune` staged for J.* Migrate stats/MoveRange/cooldowns/ability params off Data Assets so balancing is CRUD-in-a-sheet. Include a **`bStatusImmune` capability flag** (any unit can carry it; prefer this over an archetype label literally named "Boss") so Block J's boss immunity is assigned from data, not a hardcoded per-class tag. Lands **before** Block J so the immunity flag + balance pass both use it. *(See DESIGN_RATIONALE §8.)*

### Block H2 — Character progression (XP, leveling, Mana economy)
**Moved here (was drafted right after Block H) so it's built once, on the final kit, in the final data format** — Block H's rework (Confuse, displacement Intimidate) and Block I's Interact would otherwise force a second pass. Built **data-driven from I2's DataTables**, not hardcoded per-ability GameplayEffect assets. EXP awarded **per successful action** during battle (amount varies by action type — exact values tuned in the Block J balance pass). Leveling triggers **mid-battle**, immediately on threshold crossing — not deferred to chapter-end. Level-up raises **raw D&D ability scores**, which cascade into derived resources: **CON → MaxHealth** (LevelUp adds CON modifier × 5), **INT → MaxMana** (mirrors the CON→HP pattern). **MoveRange stays flat/independent** — explicitly NOT stat-derived, to preserve the locked §5 decision that Speed/Movement stays separate from DEX (which already drives Initiative + AC). MoveRange growth becomes an **equipment** concern in Phase 2. Every ability needs a **Mana cost** assigned (none currently deduct Mana). Infra already scaffolded since Session 1 and unused: `XP`, `CharacterLevel`, `Mana`, `MaxMana`, all 6 D&D core stats already exist on `UCRPGAttributeSet` with Init values + clamping — this block is GameplayEffects/logic (GE_GainXP, LevelUp trigger, mana-cost commits), not new attributes. **Deferred to Phase 2:** unlocking new abilities, skill/ability-upgrade points, equipment.

### Block J — AI, boss & encounter
Tactical enemy AI (move-to-target, confused targeting, boss pursuit). **Boss = fast/ranged, immune to status** — the unkitable clock. Encounter layout, mob count, and the balance pass → tight, winnable, fun.

### Block K — Clarity & feedback (the "fun" polish)
Movement-range & ability-range/AoE indicators, threat telegraphs, status icons, hit/miss/damage feedback, dice-roll visualization, ability VFX, confusion + bookshelf-crash visuals.

### Block L — Cinematic chapter loop
Chapter card + Battle Commenced (done). Ending slides → World Finished → Chapter 2 teaser, reusing the configurable cutscene widget.

### Block M — Environment art pass (de-greybox)
Swap the greybox floor / walls / shelves for **sourced** meshes + materials (Synty/Fab — *source, don't create*; the user is a coder). Near-zero code: wires into existing actors — the rigged shelf's base-hinge `Pivot` + auto-sized blast rectangle already carry a real bookshelf model, and there's a standing deferred note to swap the greybox cube. **Time-boxed, downstream of the boss fight (J) + the full loop (L)** — environment art is a classic rabbit hole and this is a *coding* final. A **light** version (a decent floor material + one real shelf mesh) may be pulled forward for a demo/morale boost without opening the full pass.

### Block N — Onboarding
Teach the manipulation loop. **Last** — only once mechanics are frozen.

---

## Phase 2 — Breadth
- More worlds (each with signature interactables, hazards, and palette)
- New abilities; **companions with unique kits** (Embolden returns here)
- Ability-upgrade points spent on the Block H2 progression kit (unlocking/upgrading abilities)
- Equipment (including MoveRange growth — see Block H2)
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
