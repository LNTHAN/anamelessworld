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

---

## Block A — Battle System Core (Sessions 7–13)
Goal: 1 player vs 3 enemies taking turns, abilities fire, someone dies

| # | Topic |
|---|---|
| 7 | APlayerCharacter + AEnemyCharacter subclasses, key-bound attack, UTurnManager wiring |
| 8 | Turn order queue — initiative rolls determine who goes when, shown in order |
| 9 | Enemy AI — enemies auto-select a move and target on their turn |
| 10 | ABossCharacter — higher stats, different moveset |
| 11 | Data Assets — each character loads stats + abilities from a data asset (no hardcoding) |
| 12 | Multiple abilities — at least 2 per character (e.g. basic attack + special) |
| 13 | Win/lose detection — all enemies dead = battle over; player dead = game over |

---

## Block B — Battle UI (Sessions 14–17)
Goal: Player can see HP, whose turn it is, and pick commands from a menu

| # | Topic |
|---|---|
| 14 | Health bars — UMG widget showing HP for all characters |
| 15 | Turn order indicator — UI strip showing upcoming turn order (like FF Tactics) |
| 16 | Command menu — player selects ability from a list on their turn |
| 17 | Ability animations — characters play attack/hurt/death animations (no VFX/SFX) |

---

## Block C — Dialogue + Story (Sessions 18–21)
Goal: Story wraps around the battle

| # | Topic |
|---|---|
| 18 | Dialogue system — text box, character name, advance on input |
| 19 | Pre-battle dialogue — conversation triggers when battle field loads |
| 20 | Post-battle dialogue — triggers when all enemies die |
| 21 | Story slideshow — sequence of images with text (intro before battle) |

---

## Block D — Menu + Audio (Sessions 22–23)
Goal: Game has a proper start and music

| # | Topic |
|---|---|
| 22 | Main menu — Start Game button, transitions to story slideshow |
| 23 | BGM — 1 track starts at menu, persists through slideshow and battle, stops at end |

---

## Block E — Ending Sequence (Sessions 24–25)
Goal: Battle ends with a proper cinematic finish

| # | Topic |
|---|---|
| 24 | Chapter complete screen — fade to black after post-battle dialogue |
| 25 | "To Be Continued..." screen — final card, then credits or loop to menu |

---

## Total
- Sessions done: 6
- Sessions remaining: ~19
- At 2 sessions/day: **~10 days**
- Realistic (some sessions expand): **2–3 weeks**

---

## 2-Sessions/Day Calendar
Target start: 2026-06-03 (Session 7)

| Day | Date | Session A | Session B |
|---|---|---|---|
| 1 | Jun 03 | 7 — APlayerCharacter + AEnemyCharacter subclasses, key-bound attack | 8 — Turn order queue, initiative rolls, turn order UI |
| 2 | Jun 04 | 9 — Enemy AI (auto-select move + target) | 10 — ABossCharacter (higher stats, different moveset) |
| 3 | Jun 05 | 11 — Data Assets (load stats + abilities per character) | 12 — Multiple abilities (2 per character) |
| 4 | Jun 06 | 13 — Win/lose detection (all enemies dead / player dead) | 14 — Health bars UMG widget |
| 5 | Jun 07 | 15 — Turn order indicator UI strip | 16 — Command menu (player picks ability on their turn) |
| 6 | Jun 08 | 17 — Ability animations (attack / hurt / death, no VFX) | 18 — Dialogue system (text box, name, advance on input) |
| 7 | Jun 09 | 19 — Pre-battle dialogue trigger | 20 — Post-battle dialogue trigger |
| 8 | Jun 10 | 21 — Story slideshow (images + text sequence) | 22 — Main menu (Start Game → slideshow) |
| 9 | Jun 11 | 23 — BGM (1 track, persists through slideshow + battle) | 24 — Chapter complete screen + fade to black |
| 10 | Jun 12 | 25 — "To Be Continued..." screen | Buffer / polish + bugfix |

**Target completion: ~2026-06-12**
Buffer week for overscoping: **2026-06-19 hard deadline**
