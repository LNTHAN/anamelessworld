# A Nameless World — Design Rationale

A living record of design decisions and the reasoning behind them. Each entry captures **what** was decided, **why**, and **what was rejected**. This is a thinking artifact, not a feature spec — it will be folded into a formal Game Design Document once the game is more polished.

---

## 1. Core Concept & Narrative

### The protagonist knows he is in a game
**Decision:** Nameless is a Visual Novel character aware he exists inside fiction. When players grow bored and delete/discontinue his game, he gains the power to step through stories and destroy the worlds that dismissed him.
**Why:** A meta-premise gives every system a thematic reason to exist (the book as a portal, worlds as "stories," deletion as the inciting trauma). It's emotionally resonant and uncommon.
**Rejected:** A conventional "chosen hero" framing — generic, and it wouldn't justify the meta-UI choices below.

### Nameless is a scholar, not a warrior
**Decision:** His focus/weapon is a **book**; he is depicted as a cloaked hooded scholar, never an armored fighter. Face stays hidden until a gated reveal.
**Why:** His power is over *narrative itself*, not swordplay. A scholar who unmakes worlds is more unsettling than another fighter. Anonymity reinforces "Nameless."
**Rejected:** Staff/sword iconography — too generic-fantasy, undercuts the literary identity.

### Face reveal is a gated story reward
**Decision:** His face is only revealed if the player triggers the "correct" storyline branch.
**Why:** Turns identity into a *mechanical* reward for narrative exploration — players have a concrete reason to seek the right path. Reactive, choice-driven content (a Larian value).
**Rejected:** Showing his face in marketing/intro — spends the payoff for nothing.

---

## 2. Presentation & Tone

### Cold "World N … Finished" screen instead of a victory screen
**Decision:** After a battle, a dead-silent black screen with bottom-right corner text: "WORLD 1 … FINISHED." No fanfare, no "Victory!", no triumphant music.
**Why:** Victory screens signal triumph and reward. Nameless feels *neither* — he's indifferent. Subverting the expected beat conveys his detachment more powerfully than any dialogue.
**Rejected:** A traditional "Chapter Complete / Victory" stamp with screenshake — it carried triumphant energy that contradicts the character.

### Progression systems track the protagonist's psyche
**Decision:** Three recurring elements all shift per world, driven by one `CurrentWorldIndex`:
- **BGM tone** — dark/suspenseful (World 1) → lighter, more hopeful each world.
- **"Finished" text color** — red (rage/destruction, World 1) → paler each world.
- **"Finished" punctuation** — "Finished." (cold certainty) → "Finished…" (hesitation) → "Finished?" (genuine doubt).
**Why:** Multiple restrained, diegetic channels reinforcing the same arc = strong thematic cohesion. The player *feels* Nameless changing without being told. One source of truth keeps it consistent and cheap to author.
**Rejected:** Cutscene exposition stating his change — telling instead of showing.

### Meta intro that confronts the player directly
**Decision:** Opening slides show Nameless inside a monitor watching a bored player hover "Yes" on a "Delete this game?" dialog, and a CEO stamping his project "DISCONTINUED," before his vow to "destroy everything."
**Why:** Establishes the meta-premise wordlessly and implicates the *player* personally — the dismissal is something *you* did. Sets the confrontational tone immediately.
**Rejected:** A lore-dump narrator intro — slower, less personal, less memorable.

---

## 3. Visual Identity

### Per-world custom chapter cards (Kingdom Hearts style), not uniform FFT cards
**Decision:** Each chapter gets bespoke title-logo art expressing that world's theme, rather than one templated card with swapped text.
**Why:** The game travels world to world; each world needs an instant visual identity. KH world logos prove this reads as memorable and intentional.
**Rejected:** Uniform FFT-style line-and-text cards — elegant but generic across many distinct worlds.

### Recurring purple-gold book motif across all chapter logos
**Decision:** Every chapter logo embeds a small book glowing in Nameless's purple/gold, kept distinct from that world's palette.
**Why:** Ties all logos into a set AND visually states Nameless is the outsider intruding on each world — the foreign element that doesn't belong. The book is his signature.
**Rejected:** Fully independent per-world logos — loses the cohesive throughline.

### Palette as world-identity and genre commentary
**Decision:** World 1 (book world) = dark gold/purple, somber. World 2 (action world) = bright saturated summer-blockbuster, parodically over-the-top.
**Why:** Hard palette breaks make each world feel distinct on arrival. World 2's brightness is *deliberate genre parody* — it matches the bored soldier who's done this "ten thousand times," signaling "the action-movie cliché world."
**Rejected:** Grimdark realism for the warzone — would read as a serious war story and clash with the comedic tone.

### Consistent rendering style, varying palette
**Decision:** All story slides share one gritty painterly rendering technique; only palette and lighting change per world.
**Why:** Keeps the game feeling like one coherent work while letting each world feel distinct.
**Rejected:** Different art styles per world — would feel like asset-flip inconsistency.

---

## 4. Audio

### One continuous track from menu through battle, not a separate battle theme
**Decision:** BGM starts on the main menu and persists (via GameInstance) through the intro cutscene and into the first battle.
**Why:** Preserves narrative momentum — the music keeps building from "I'm going to destroy everything" straight into combat. Continuity feels cinematic and intentional for a first battle.
**Rejected:** A dedicated battle theme — cutting tracks would break the built-up tension; reserved for later when battles become routine.

---

## 5. Systems Direction (Phase 1 — Chapter 1)

### Depth-first: build Chapter 1 to near-final quality before adding breadth
**Decision:** Phase 1 = one complete, polished, *fun* tactical chapter (movement + environment + the full manipulation kit + balance). Phase 2 = breadth (more worlds, abilities, companions).
**Why:** Building one chapter end-to-end exercises every system we'll reuse, and proves the combat is actually fun before we multiply content. Avoids the throwaway work of an abstract prototype that gets redesigned once movement lands.
**Rejected:** Ship a basic no-movement "first battle" for Phase 1 and add tactics later — creates rework, and we aren't time-constrained enough to justify it.

### Movement built before designing the Chapter 1 battle
**Decision:** Implement BG3-style free (radius-based) movement as the foundation of the Chapter 1 fight, not a later add-on.
**Why:** The manipulation kit (positioning who the "nearest creature" is, kiting, herding into hazards) only works on a real battlefield. Designing the battle *around* movement from the start avoids painful retrofitting.
**Rejected:** Author the battle on static positions first, add movement after — would require redesigning the encounter and the abilities.

### Terrain and elevation that create tradeoffs, not decoration
**Decision:** Woods slow movement but grant dodge chance; elevation gates melee beyond a height threshold. Movement and Action are separate "stocks" per turn, usable in any order or skipped.
**Why:** Positioning must be *strategic*, not cosmetic. Meaningful tradeoffs (speed vs. defense, high ground vs. reach) are what make tactical combat deep — and what a studio like Larian looks for.
**Rejected:** Flat battlefield with movement as flavor — movement without terrain consequence is busywork.
**Chapter 1 scope note:** the book-stack elevation in the Ch1 library is **aesthetic/positional only for the first pass** — higher ground to stand on, no mechanical rule (no movement penalty, no accuracy modifier) yet. No ranged-attack system exists yet for elevation to meaningfully interact with (the boss's "fast/ranged" trait is still Block J, unbuilt), so gating melee-by-height now would be new code with nothing to play off. Revisit once Block J's ranged boss exists.

### Speed and Initiative kept separate (D&D 5e model)
**Decision:** Initiative (Dex + d20) sets turn order; Speed (flat stat) sets movement range. They are different stats.
**Why:** Avoids the Fire-Emblem problem where one Speed stat dominates everything. Keeps turn order and mobility as independent design levers.
**Rejected:** A single Speed stat governing both order and movement — too swingy/overpowered.

### Mid-battle XP/leveling, but MoveRange stays out of it (Block H2)
**Decision:** XP is awarded per successful action and can level Nameless up **mid-battle** (not deferred to chapter-end). Level-up raises raw D&D ability scores, which cascade into **CON → MaxHealth** and **INT → MaxMana** — but explicitly **not** DEX → MoveRange.
**Why:** CON→HP matches the AttributeSet's original Session-1 design and standard D&D 5e. Keeping MoveRange out of the stat-scaling chain protects the Speed/Initiative separation above — DEX already drives Initiative and AC, so routing Movement through it too would recreate the exact "one stat dominates everything" problem this section already rejected. MoveRange growth is deferred to **equipment**, in Phase 2.
**Rejected:** Tying MoveRange to DEX on level-up — briefly considered, reopens the Speed/Initiative decision above for no real benefit.

---

## 6. Combat Design — Nameless's Manipulation Kit

### Nameless deals zero direct damage — he is a pure manipulator
**Decision:** Remove Basic Attack / Heavy Strike. Nameless wins *only* by turning the world's "heroes" against each other and against the environment.
**Why:** It's the mechanical expression of his entire identity — a scholar who unmakes stories, not a fighter. A weak fallback attack would muddy the fantasy and let players ignore the real puzzle.
**Rejected:** Keep a chip-damage attack as a safety net — undercuts the identity and the design tension.

### Confuse (reworked Provoke), not classic taunt
**Decision:** The control ability makes a target attack the **nearest creature** (ally, boss, anyone) with a **damage buff but low accuracy** (Disadvantage). This is Confusion, not Provoke (which would force attacks onto the caster).
**Why:** Confusion is what turns enemies into Nameless's weapons. The damage-up / accuracy-down combo makes confused enemies reckless and dangerous but unreliable — a tool you aim, not a guarantee.
**Rejected:** Classic Provoke (force them to attack Nameless) — pulls aggro *onto* the fragile manipulator, the opposite of what we want.

### The boss is immune to status effects — and that's the encounter's spine
**Decision:** The boss cannot be Confused, Feared, or otherwise disabled. It can only be *damaged* — by confused mobs and environmental hazards, never by Nameless directly.
**Why:** Immunity (a) kills the degenerate "disable everything" strategy, (b) makes the boss an unkitable **clock** that forces urgency, and (c) reframes the win condition: you defeat the untouchable boss by aiming his own minions and his own world at him. The problem *is* the design.
**Rejected:** A boss that can be Confused — trivializes the fight and removes all pressure.

### Intimidate = displacement / AoE control, not a second disable
**Decision:** Re-theme Intimidate from "stun one mob" to **forced displacement + AoE fear** (scatter enemies away from Nameless). Concrete mechanic: on a successful cast, every affected enemy **instantly** retreats in a straight line away from the direction they're facing, for a distance equal to their own `MoveRange`. The retreat stops the instant it hits **anything solid** — another enemy, a wall, or a bookshelf — and the retreating enemy takes damage equal to their own normal-attack damage (reuses their existing `GE_DamageInstant`/`GE_HeavyStrike`, no new damage formula). If the solid thing hit was **another enemy** (never Nameless — see below), that enemy takes the same damage too (mutual); walls/bookshelves don't take damage back, just stop the retreat.
**Why:** As a single-target disable it was redundant with Confuse, so players would always pick the stronger one. As displacement it owns a different job — survival, repositioning, and herding enemies into each other or into hazards (e.g. under a rigged bookshelf). Instant (not delayed to their next turn, unlike Confuse) because fear is a startled reaction, not a persistent mental state — the payoff should be visible immediately. One unified "hits anything solid → damage" rule (rather than separate rules per collision type) keeps it simple to implement and to reason about.
**Nameless is excluded from the collision check** — a retreating enemy can never collide with (and damage) Nameless himself, only other enemies/the boss/the environment. Anything else would be backdoor chip damage through the "zero direct damage" rule (see below) via a side effect instead of an attack.
**Rejected:** Keep both as single-target disables — redundant, one strictly dominates. Also rejected: delayed (next-turn) retreat like Confuse — less dramatic payoff for what's fictionally a startle/panic reaction. Also rejected: different damage amounts for wall-vs-shelf-vs-enemy collisions — one flat rule is simpler and just as effective.

### Embolden deferred to Chapter 2
**Decision:** Cut the ally-buff (Embolden/Advantage) from Chapter 1; reintroduce it when Nameless gains his first **companion** in Chapter 2.
**Why:** A buff with no ally to target is dead weight in a solo chapter. Holding it back also gives Chapter 2 a fresh mechanical "new toy" — good progression.
**Rejected:** Repurpose Embolden onto enemies in Ch1 — confusing semantics, and it competes with the cleaner kit.

### Interact — manipulating the world is a core verb
**Decision:** Add an **Interact** command that arms environmental objects, each with a unique effect per world. Chapter 1 = **rigged bookshelves**, built as a **two-stage proximity trap**:
- **Stage 1 — Arm** (the Interact command, costs the player's Action; Nameless must be within interact range ~200 cm): rigs the shelf. It becomes *armed* and watches its AoE. Nameless can then walk away and spend later turns herding enemies toward it.
- **Stage 2 — Spring** (automatic, free): while armed, the instant a living **enemy** enters the AoE, the shelf topples and applies the crush damage (`GE_HeavyStrike`) to **every living character** inside the blast — enemies, allies, **and Nameless himself**. One-shot — a toppled shelf is spent.

**Trigger and damage are deliberately split.** *Trigger* is enemy-entry only: Nameless arms from ~200 cm but the AoE is ~300 cm, so at arm-time he stands inside his own blast — if his presence could spring it, arming would self-detonate. Fiction: he rigged it and steps over the tripwire; enemies don't. *Damage* is universal: a toppling shelf obeys physics, so if Nameless (or any ally) is still in the AoE when an enemy trips it, they're crushed too. He must arm it and **clear out** before herding enemies in. The boss *is* an enemy, so this still reaches the status-immune boss.
**Why:** A manipulator who weaponizes the *world itself* is deeply on-theme (and the bookshelf is perfect VN-world flavor). It's a second damage source that can reach the status-immune boss. **Two-stage + proximity** (rather than "interact = crush now") makes it the *cunning-trapper* fantasy — rig it, retreat, herd prey in with Confuse/Intimidate — instead of forcing Nameless to stand next to his own blast.
**Rejected:** *Interact = detonate immediately* — forces Nameless to crush from inside the blast, and loses the cunning rig-and-retreat fantasy. *Timed detonation (after N turns)* — can whiff on an empty tile and forces the player to predict enemy pathing turns ahead; proximity rewards herding and needs no round-counting. *Combat with no environmental interaction* — wastes the manipulator fantasy and leaves the boss with too few counters.

### Movement ≈ equal — safety comes from control, never speed
**Decision:** Nameless's movement is roughly equal to enemies' (at most slightly higher). He survives via abilities + terrain + action economy (Move *or* Act tradeoffs), not by outrunning anyone.
**Why:** If the manipulator could simply kite faster than everyone, there'd be no lose condition. Tying survival to *control* (confuse a chaser so it turns on a neighbor, fear to make space, interact to wall a lane) keeps real risk and makes positioning the skill. The status-immune boss is the unkitable threat that punishes passive play.
**Rejected:** Give Nameless dominant movement speed — removes the failure state and trivializes the fight.

---

## 7. Controls & Input

### Modal Left-Click (Idle = move, armed ability = target)
**Decision:** Left-click is context-sensitive. In **Idle** mode it moves the character; once an ability is **armed** (via command menu or hotkey) left-click confirms that ability's target, and RMB/Esc cancels back to Idle.
**Why:** One ability deals with one target type, but the player has several abilities plus movement — overloading them onto distinct buttons gets clunky fast. The mode model (XCOM/BG3 standard) keeps a single intuitive "click to do the obvious thing" while supporting many abilities. It also gives movement and abilities a shared gate ("only act when it's your turn / in the right mode").
**Rejected:** Fixed per-button mapping (LMB always move, RMB always target) — doesn't scale past one targeted ability and feels unintuitive.

### Ability selection: command-menu clicks + number hotkeys
**Decision:** Abilities can be armed by clicking the on-screen command menu *or* by pressing 1/2/3(/4). Both routes set the same pending-ability state.
**Why:** Menu clicks are discoverable for new players; hotkeys are faster for experienced ones. Low cost to support both.
**Rejected:** Hotkeys only (undiscoverable) or menu only (slow).

### Full tactical camera built now, not deferred to polish
**Decision:** Build rotate / zoom / pan camera control as part of the movement work (Block G), not later in the clarity/polish block.
**Why:** Positioning is the core of the game; you can't evaluate movement, ranges, or herding without freely inspecting the battlefield. Testing the movement system honestly *requires* the camera, so it belongs alongside it.
**Rejected:** Fixed camera now, tactical camera later — would make every movement/balance playtest misleading.

---

## 8. Data Architecture

### Spreadsheet-driven stats via DataTables (CSV), not per-asset Data Assets
**Decision:** Migrate character/ability/effect numbers (stats, MoveRange, cooldowns, ability params) into **DataTables** backed by `USTRUCT` row types and edited as **CSV** (Excel/Sheets → export → reimport). Keep the existing Data Asset approach only where a few complex hand-authored objects make sense. Set up the pipeline in a dedicated session *before* the balance pass (Block J).
**Why:** Tactical balancing means touching lots of numbers repeatedly; a spreadsheet/CRUD workflow (standard in production — e.g. Nexon's Fantasy War Tactics R) lets designers tune without code or per-asset clicking, version-controls as text, and scales to Phase 2's content explosion. Cheaper to establish now (3 characters) than after content multiplies.
**Rejected:** (a) One `UCRPGCharacterData` Data Asset per character forever — doesn't scale to bulk balancing. (b) Hardcoded magic numbers — the thing data-driven design exists to avoid. (c) Doing the migration in Phase 2 — more data to migrate, and it misses the Phase-1 balance pass where it's most useful.

---

## 9. Combat UX Conventions

### Fire Emblem colour language: cyan = movement, red = attack/threat
**Decision:** Movement range is shown in **cyan/blue**; ability/attack range and enemy threat range are shown in **red**. Same decal/zone tech, different colour.
**Why:** It's an established tactical convention (Fire Emblem: Three Houses, etc.) players already parse instantly — cyan "where I can go," red "where danger is." Free readability by matching genre expectation.
**Rejected:** A bespoke colour scheme — no upside, and it fights players' existing mental model.
**Note:** Our field is radius-based (not tiles), so these read as circular *zones* rather than tinted squares — same intent, different shape. Full visual polish of these indicators (flat-fill zones, edges, telegraphs) is Block K work, not now.
