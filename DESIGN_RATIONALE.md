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

### Speed and Initiative kept separate (D&D 5e model)
**Decision:** Initiative (Dex + d20) sets turn order; Speed (flat stat) sets movement range. They are different stats.
**Why:** Avoids the Fire-Emblem problem where one Speed stat dominates everything. Keeps turn order and mobility as independent design levers.
**Rejected:** A single Speed stat governing both order and movement — too swingy/overpowered.

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
**Decision:** Re-theme Intimidate from "stun one mob" to **forced displacement + AoE fear** (scatter enemies away from Nameless).
**Why:** As a single-target disable it was redundant with Confuse, so players would always pick the stronger one. As displacement it owns a different job — survival, repositioning, and herding enemies into hazards (e.g. under a rigged bookshelf).
**Rejected:** Keep both as single-target disables — redundant, one strictly dominates.

### Embolden deferred to Chapter 2
**Decision:** Cut the ally-buff (Embolden/Advantage) from Chapter 1; reintroduce it when Nameless gains his first **companion** in Chapter 2.
**Why:** A buff with no ally to target is dead weight in a solo chapter. Holding it back also gives Chapter 2 a fresh mechanical "new toy" — good progression.
**Rejected:** Repurpose Embolden onto enemies in Ch1 — confusing semantics, and it competes with the cleaner kit.

### Interact — manipulating the world is a core verb
**Decision:** Add an **Interact** command that arms/triggers environmental objects, each with a unique effect per world. Chapter 1 = **rigged bookshelves** (arm one; it crushes enemies caught in its AoE).
**Why:** A manipulator who weaponizes the *world itself* is deeply on-theme (and the bookshelf is perfect VN-world flavor). It's also a second damage source that can reach the status-immune boss.
**Rejected:** Combat with no environmental interaction — wastes the manipulator fantasy and leaves the boss with too few counters.

### Movement ≈ equal — safety comes from control, never speed
**Decision:** Nameless's movement is roughly equal to enemies' (at most slightly higher). He survives via abilities + terrain + action economy (Move *or* Act tradeoffs), not by outrunning anyone.
**Why:** If the manipulator could simply kite faster than everyone, there'd be no lose condition. Tying survival to *control* (confuse a chaser so it turns on a neighbor, fear to make space, interact to wall a lane) keeps real risk and makes positioning the skill. The status-immune boss is the unkitable threat that punishes passive play.
**Rejected:** Give Nameless dominant movement speed — removes the failure state and trivializes the fight.
