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

## Delivery schedule — LOCKED to Aug 15 2026 (reprioritized 2026-07-21, VISUAL-FIRST)
Coding-school final due **Aug 15 2026**. **Reprioritization (2026-07-21):** the grader weights *visual
polish + a complete, finished-looking demo* over combat balance / systems depth. So **balance (rest of
Block J) and H2 progression are deferred to a new Block O** (may be cut), and effort shifts to **K (UI)
and M (environment art)**. Block J's *systems* (immunity, 3-mob encounter, data-driven damage/forecast,
Confuse range) are DONE — only its balance pass is deferred.
- **Jul 21–27 — Block K (UI/clarity polish):** range indicators (Confuse ring + valid targets, move zone),
  enemy-intent threat-lines, floating damage/Miss/Immune text, status icons. **PLUS two small tasks here:**
  (a) a 5-min mob-pacing fix (bump mob MoveRange / move mobs closer — pure demoability, not balance);
  (b) **L-minimal**: wire the win/lose screens now (the `OnCombatEnded_Event` hook already exists) so a
  *complete loop always exists* regardless of how M goes.
- **Jul 28–Aug 3 — Block L (full cinematic loop):** endings, "World N … Finished", Chapter 2 teaser (polish
  on top of L-minimal). Cheap — the `OnCombatEnded_Event` hook already exists.
- **Aug 4–8 — Block M (environment art / de-greybox):** sourced meshes+materials (Synty/Fab; source,
  don't create). **TIMEBOXED — the one real rabbit hole; deliberately AFTER L so it can't swallow the ending.**
  *(Pulled forward — part 1 done Jul 26, part 2 running Jul 27.)*

### REVISION 2026-07-27 — N cut, build work compressed to end of July
L landed a full week early, so the back half of the calendar is rewritten:
- **Jul 27–31 — M part 2 → UI theme pass → juice pass** (in that order):
  1. ~~**M part 2**~~ — **✓ DONE 2026-07-27** (prop materials, shelf dressing + attached tell-books,
     banner variance, lighting balance). Extra book dressing deliberately deferred to after UI+VFX.
  2. **Whole-game UI theme pass** (~2 sessions) — Block K's deferred item, previously unscheduled.
     Sequenced BEFORE juice: the UI is in every frame of the showcase video, a VFX burst is on screen for
     half a second. **Scopeable because the design language already exists** — `WBP_GameResult` has the
     locked requiem palette (Cinzel, muted crimson `0.60,0.18,0.16`, faded parchment, `M_Divider`); this
     pass *propagates* it into the battle UI, which currently doesn't match its own ending screens.
     Finish line: does the HUD look like it belongs to the same game as the result screen?
     - Palette + font onto `WBP_BattleHUD`, `WBP_CommandMenu`, `WBP_UnitCard`, `WBP_TurnOrderStrip`,
       `WBP_DialogueBox`; the deferred ~5px arrow/card height alignment; lose-screen button reposition.
     - **Status icons over units: CUT** — the yellow threat-line already reads as "confused", and a
       narrated demo says it out loud. Redundant.
  2b. **UI ornament pass — PULLED IN 2026-07-27** (was "stretch goal / Phase 2"). User's call: the game
     is presented to a class and the UI is on screen for every second of the demo. **Timeboxed to 2
     sessions with a fixed scope** — ornament on **three** things only: unit cards, command-menu buttons,
     turn-indicator backing. NOT every widget (dialogue box, cutscene and result screens already have
     their own look). **Ship the flat version if it stalls** — the flat theme already reads as finished;
     ornament is upside, not rescue.
     - Assets: **Kenney UI Pack RPG Expansion** (CC0, no attribution) — fantasy-styled panels that sit
       with a candlelit library. Applied via **9-slice** (Brush → Draw As "Box" + margins): one small
       frame texture stretches to any size without smearing corners.
     - **Ability icons on the command menu rank ABOVE borders** for return-per-minute — they make the
       menu readable at a glance in a video nobody can pause. `game-icons.net` (CC BY → needs a credits line).
     - Ordering rule that makes this safe: colour/type/hierarchy FIRST, ornament LAST. Ornament is
       applied on top of a settled layout; doing it first means redoing it when hierarchy changes.
     - Habits that prevent rework: set colour as brush **tint** never baked into the image; leave 10–20px
       padding slack (ornate borders eat edge); don't over-tune Rounded Box (it's a placeholder for frame art).
     - **Trade-off accepted:** this competes with the Aug 1–15 deliverables. If the calendar tightens,
       the **VFX count** in the juice pass is the trim lever, not the video or slides.
  3. **SFX/VFX juice pass** — *sourced* audio + Niagara wired into hooks that already fire
     (`OnDetonated`, `OnTelegraph`, `PlayHitReact`, `TriggerShake`). A silent demo reads as unfinished
     faster than anything else, and this is wiring, not authoring.
  **Target: Phase 1 build work COMPLETE by end of July** — ~7–9 sessions against ~10 available, so it
  fits with zero slack. A slip to Aug 1–2 is acceptable (the deliverable window has room); a rushed theme
  pass is not. **Trim levers in order:** status icons (already cut) → dressing scope → VFX count.
- **Block N (onboarding) — CUT.** The final is a **narrated live demo** — the grader is walked through
  the manipulation loop in person, never handed the controller — so in-game teaching has no audience.
  The scripted turn-order override on `UTurnManager` is cut with it. Revisit in Phase 2, when the game
  is actually put in a player's hands.
- **Aug 1–15 — deliverables + buffer:** showcase video, written docs, presentation slides. Then
  balance-lite (Block O, optional) and playtest/bug-fix. **No new features.**
  - **Capture order matters:** the Level-BP debug keys (**K** = instant win, **L** = instant lose) stay
    in until the video is shot — they're how you film the ending chain and both result screens without
    replaying a battle per take. **Strip them after capture, before the final build.**
- **Block O (deferred): balance pass + H2 progression** — only if buffer allows; else cut (grader doesn't
  weight balance). See CONTEXT for the concrete balance targets already captured. H2 progression is the
  first thing to drop: XP/leveling is invisible work in a single-battle demo.
- **Cut order if slipping:** Block O (already deferred) → full M art (keep a light version) → Tier-2 danger overlays.
- **Order note:** L precedes M (the complete-loop signal is secured before the art rabbit hole); L-minimal is
  still wired early in the K week as insurance.

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
Make the board *readable* so the manipulation puzzle can be played on purpose. §9 colour language: **cyan = movement, red = attack/threat** (radius-based → circular zones, not tiles).

**Range/AoE indicators (each ability now HAS a range that's currently invisible):**
- **Movement range** — cyan zone around the active unit, shown on the player's turn (drive size from `MoveRange`; the Block-G `M_MoveRange` decal is the starting point).
- **Confuse cast range** — ring around Nameless during Targeting + highlight valid (in-range, living-enemy) targets. Currently `ConfuseCastRange` (600) is enforced but you click blind.
- **Intimidate AoE** + **Interact/shelf blast zone** — show the affected area before committing.

**Enemy-intent telegraphs (the highest-leverage legibility work — a manipulation game is only fun if you can see what you're manipulating). Two tiers:**
- **Tier 1 (cheap, THIS block):** during the player's turn, a curved **threat line** from each enemy to its intended target (who it will attack — the exact datum Confuse manipulates) + that enemy's move/attack range on hover. Event-driven refresh (recompute after the player moves/confuses, NOT per-frame). Prereq already noted for Block J: keep each enemy's current-target decision cheap to query outside its turn.
- **Tier 2 (expensive, deferred — Phase 2):** live FE3H-style danger zones that recompute as you drag a hypothetical move. Don't let its cost scare off Tier 1.

**Combat feedback:** floating combat text — "Miss!" / "Resisted!" / **"Immune"** (Confuse on the boss) / damage numbers; roll detail (`d20: 17+3 vs AC 14`) to the battle log. Forecast panel shows Immune instead of an inflict %. **Dice-roll visuals stay COMBAT-COMPACT** (no cinematic 3D dice — that's reserved for narrative action checks, which don't exist until Phase 2).

**Juice:** status icons over units, ability VFX, the confusion + bookshelf-crash visuals, more screenshake. Also the deferred Intimidate polish (smooth panic-slide + fall-over, replacing the teleport) and the toppled-shelf linger/fade.

**Animation strand (added 2026-07-21) — a parallel track through Block K.** Make the body show state, not just the HUD. Uses the existing FProperty-reflection pattern (`SetIsAttacking`/`SetIsDead`) + a name-driven montage helper. Assets: TinyHero (enemy) set is rich; BattleWizard (Nameless) covers his kit; RPGHero (boss) is sparse but he's status-immune so he never needs dizzy/backpedal — attack + hit-react + victory is enough.
- **Per-ability attack anims** — distinct clips for Basic vs Heavy vs Confuse vs Intimidate (currently all share one `bIsAttacking`).
- **Getting-hit reaction** — fires on damage received; pairs with item #1's floating combat text (same moment).
- **Dizzy** — loops while `State.Confused` is on the unit.
- **Backpedal / stagger-back** — for the Intimidated/displaced beat (pairs with item #3 threat-line legibility).
- **Victory / defeat** poses — play on `OnCombatEnded` (win vs lose), and defeat pairs with L-minimal's screens.

**Confirmed build order (locked 2026-07-21):** (1) floating combat text → (2) range indicators (cyan move zone + red Confuse ring) → (3) enemy-intent threat lines → (4) warm-up side-tasks (mob-pacing fix + L-minimal win/lose). Animation strand woven in where it pairs (hit-react with #1, backpedal/dizzy with #3, victory/defeat with #4/L).

**STATUS (2026-07-22): items #1–#3 DONE** (Session 41 — full detail in CONTEXT.md "Block K core" section). Item #2's move zone became **terrain-exact tiles** (user overrode the circular-zone default) + gained **red/grey target auras**. The two **click-to-inspect optionals** (pinned enemy threat zones + character cards on click) are **DONE** — built as **one unified 3-slot bottom HUD** (NameCard / forecast arrow / Inspect-Target card), cards corner-tucked + mirrored (full detail in CONTEXT.md "Block K optionals — click-to-inspect"). Still open: the #4 side-tasks (mob-pacing, L-minimal), then dizzy/backpedal/victory/defeat anims as time allows.

### Block L — Cinematic chapter loop
Chapter card + Battle Commenced (done). Ending slides → World Finished → Chapter 2 teaser, reusing the configurable cutscene widget.

### Block M — Environment art pass (de-greybox)
Swap the greybox floor / walls / shelves for **sourced** meshes + materials (Synty/Fab — *source, don't create*; the user is a coder). Near-zero code: wires into existing actors — the rigged shelf's base-hinge `Pivot` + auto-sized blast rectangle already carry a real bookshelf model, and there's a standing deferred note to swap the greybox cube. **Time-boxed, downstream of the boss fight (J) + the full loop (L)** — environment art is a classic rabbit hole and this is a *coding* final. A **light** version (a decent floor material + one real shelf mesh) may be pulled forward for a demo/morale boost without opening the full pass.

### Block O — Balance pass + progression (DEFERRED, may be cut)
Moved out of Block J and after N on 2026-07-21 — the grader weights visuals over balance, so this is the
first thing cut if the buffer is tight. Two parts: **(1) the Block J balance pass** — real per-unit damage
values, MoveRanges, `ConfuseCastRange`, mob pacing, and the flat-vs-data env-damage decision (concrete
targets in CONTEXT); **(2) H2 progression** — XP/mid-battle leveling/Mana (see the H2 detail block above).
Do only if time remains after N; a rough-but-complete demo beats a balanced-but-unfinished one for the grade.

### Block N — Onboarding — ✗ CUT (2026-07-27), deferred to Phase 2
Cut because the coding-final is delivered as a **narrated live demo**: the grader watches and is talked
through the manipulation loop, and never plays it themselves. In-game teaching therefore has no audience
to teach. Nothing here is lost work — it's unstarted, and it becomes necessary the moment the game goes
in front of a player who has no narrator (Phase 2, or any public build).
- **Scripted turn order (tutorial override) — cut with the block:** a *fixed* order (Nameless → the 3 mobs, nearest-first → boss last) so onboarding could walk Movement → Confuse → Intimidate as a controlled beat. Design intent, if rebuilt: an optional "scripted order" mode on `UTurnManager` bypassing the DEX+d20 roll for that encounter only; **initiative stays the default for all real battles.** The layout is already pre-positioned for it (2 mobs mid-field near Nameless, 1 by the boss), so the groundwork survives the cut.

---

## Phase 2 — Breadth
- More worlds (each with signature interactables, hazards, and palette)
- New abilities; **companions with unique kits** (Embolden returns here)
- Ability-upgrade points spent on the Block H2 progression kit (unlocking/upgrading abilities)
- Equipment (including MoveRange growth — see Block H2)
- Branching dialogue + choices
- Multiple battles per chapter

### Phase 2 — data-driven ability system (designed 2026-07-16, deliberately deferred)
Composition-style ability data, so new skills are authored as rows instead of code:
- **`DT_AbilityEffect`** — one row per *effect primitive* (damage, status application, displacement…), holding its numbers.
- **`DT_Ability`** — one row per skill, referencing one or more AbilityEffect IDs → new skills = new combinations, no new C++.
- **Ability-ID column on `DT_Characters`** — which kit a unit carries.
- A resolution layer reading these, replacing per-ability hardcoded numbers; unifies ALL damage sources (attacks, Intimidate collision, shelf crush) into one tunable place.

**Why deferred (not rejected):** (a) **GAS already provides the effect layer** — GameplayEffects do application, duration, stacking, immunity gating; a DT_AbilityEffect table partly re-implements it. What GEs actually lacked was *data-driven numbers*, and **SetByCaller + DT_Characters columns already delivers that** at a fraction of the cost. (b) **Zero new abilities ship before Aug 15** — the Ch1 kit is frozen (Confuse/Intimidate/Interact; Embolden is Ch2), so a system whose payoff is "new abilities are cheap" returns *after* the deadline while its cost lands on the critical path. Revisit when actually authoring abilities.
**Rejected sub-idea:** making the rigged shelf an `ABaseCharacter` to fit the schema — it isn't a combatant and would inherit turn order, AI, action economy and an HP bar it never uses. Give env objects their own rows/floats instead; don't let the schema dictate what things *are*.

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
