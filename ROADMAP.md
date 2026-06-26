# ANamelessWorld — Phase 1 Roadmap
# Goal: Complete First Battle (Final Fantasy Tactics style)

## Flow
Menu → Story Slideshow → Pre-battle Dialogue → Battle → Post-battle Dialogue → Chapter Complete → To Be Continued

---

## Sessions Completed
| # | Topic | Status |
|---|---|---|
| 1 | UCRPGAttributeSet (Health, Mana, D&D stats) | ✓ |
| 2 | ABaseCharacter + AbilitySystemComponent | ✓ |
| 3 | UInventoryComponent | ✓ |
| 4 | UTurnManager | ✓ |
| 5 | GA_BasicAttack C++ | ✓ |
| 6 | GE_DamageInstant + end-to-end damage test | ✓ |
| 7 | APlayerCharacter + AEnemyCharacter subclasses, key-bound attack, UTurnManager wiring | ✓ |
| 8 | Win/lose detection, WBP_BattleHUD with health bars and turn indicator | ✓ |
| 9 | Enemy AI (auto-select move + target), ABossCharacter | ✓ |
| 10 | Command menu, Cycle Target, Boss HP bar | ✓ |
| 11 | Data Assets — UCRPGCharacterData, per-character DA assignments | ✓ |

---

## Block A — Battle System Core (Sessions 12–14)
Goal: Protagonist's real combat identity, d20 resolution, and character animations

| # | Topic |
|---|---|
| 12 | Protagonist's real kit — Lift Up (buff), Silence (debuff/stun), Provoke (enrage/redirect) |
| 13 | d20 dice system — roll + modifier vs DC, Advantage/Disadvantage, Natural 20/1 |
| 14 | Character models + animations — Mixamo free assets, attack/hurt/death state machine |

---

## Block B — Battle UI (Sessions 15–16)
Goal: Player can see what's happening and what's coming

| # | Topic |
|---|---|
| 15 | Battle log — in-game text feed showing damage, misses, stuns, self-damage per action |
| 16 | Turn order indicator — UI strip showing upcoming turn order (like FF Tactics) |

---

## Block C — Dialogue + Story (Sessions 16–19)
Goal: Story wraps around the battle

| # | Topic |
|---|---|
| 16 | Dialogue system — text box, character name, advance on input |
| 17 | Pre-battle dialogue — conversation triggers when battle field loads |
| 18 | Post-battle dialogue — triggers when all enemies die |
| 19 | Story slideshow — opening sequence (the girl scene, Visual Novel world dies) |

---

## Block D — Menu + Audio (Sessions 20–21) ✓ COMPLETE
Goal: Game has a proper start and music

| # | Topic | Status |
|---|---|---|
| 20 | Main menu, intro cutscene (6 slides), BGM via GameInstance | ✓ Done |
| 21 | (merged into Session 20) | ✓ Done |

---

## Block E — Chapter Loop (Session 22)
Goal: Chapter 1 ends, Chapter 2 begins — full cinematic loop

| # | Topic | Status |
|---|---|---|
| 22 (pt 1) | Assets + screen widgets (ChapterCard, BattleCommenced, WorldFinished), Cinzel font, DESIGN_RATIONALE.md | ✓ Done |
| 22 (pt 2) | Integration: wire chapter loop into TestLevel Level BP (battle-start card/commenced, battle-end ending sequence), make WBP_CutsceneScreen reusable, BGM fade on WorldFinished | ☐ Next |

---

## Block F — Gameplay Arc (after chapter loop)
Goal: Make the combat actually good. Ordered by dependency — design before polish, polish before tutorial.

| # | Topic | Notes |
|---|---|---|
| F1 | **Win-condition design** (PRIORITY) | How does Nameless win with no direct damage? Decide: damage-dealing ally vs. Enrage/self-destruction. Verify the fight is actually winnable. Core design, must come before any polish. |
| F2 | Battle UI clarity | Miss reasons, ability effect feedback, turn-logic readability. Clarity before flash. |
| F3 | Dice-roll animation | Visualize the d20 roll + modifier vs DC. |
| F4 | Ability VFX | Embolden / Intimidate / Provoke visual effects, damage numbers. |
| F5 | Onboarding / tutorial | LAST — never build a tutorial until mechanics are final. |

---

## Phase 1 Complete Flow
Menu → Intro Slides → Battle → Ending Slides → World Finished → Chapter 2 Title → Intro Slides → (Chapter 2 battle = Phase 2)

**Chapter names:**
- Chapter 1: A Novel Origin
- Chapter 2: Here Comes the Action

**Deadline: End of July 2026**

---

## Phase 2 Plan
- Free movement on field (BG3-style, radius-based, not tile-based)
- Environmental effects
- Dialogue choices + branching paths
- New characters + new abilities
- Multiple battles per chapter

---

## Asset Plan
- **Character models + animations:** Mixamo (free) — import FBX into UE5, retarget to UE5 mannequin skeleton
- **BGM:** Suno AI generated, modern fantasy cinematic, per-chapter tracks that get lighter as protagonist changes
- **UI art:** Custom AI-generated assets (button, logo, background)
