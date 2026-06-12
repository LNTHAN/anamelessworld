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

## Block D — Menu + Audio (Sessions 20–21)
Goal: Game has a proper start and music

| # | Topic |
|---|---|
| 20 | Main menu — Start Game button, transitions to story slideshow |
| 21 | BGM — 1 track starts at menu, persists through slideshow and battle, stops at end |

---

## Block E — Ending Sequence (Sessions 22–24)
Goal: Battle ends with a proper cinematic finish

| # | Topic |
|---|---|
| 22 | Chapter complete screen — fade to black after post-battle dialogue |
| 23 | "To Be Continued..." screen — final card, then loop to menu |
| 24 | Buffer — polish, bugfix, full playthrough pass |

---

## Total
- Sessions done: 11
- Sessions remaining: ~13
- At 2 sessions/day: **~7 days**
- Realistic (some sessions expand): **8–9 days**

---

## Revised Calendar (from Jun 08)
| Day | Date | Session A | Session B |
|---|---|---|---|
| 1 | Jun 08 | 12 — Protagonist real kit (Lift Up, Silence, Provoke) | 13 — d20 dice system |
| 2 | Jun 09 | 14 — Character models + animations (Mixamo) | 15 — Turn order indicator UI |
| 3 | Jun 10 | 16 — Dialogue system | 17 — Pre-battle dialogue |
| 4 | Jun 11 | 18 — Post-battle dialogue | 19 — Story slideshow |
| 5 | Jun 12 | 20 — Main menu | 21 — BGM |
| 6 | Jun 13 | 22 — Chapter complete screen | 23 — "To Be Continued..." screen |
| 7 | Jun 14 | 24 — Polish + full playthrough pass | Buffer |

**Target completion: ~2026-06-14**
Hard deadline with buffer: **2026-06-19**

---

## Asset Plan
- **Character models + animations:** Mixamo (free) — import FBX into UE5, retarget to UE5 mannequin skeleton
- **BGM:** Free/royalty-free track (itch.io or freemusicarchive.org)
- **UI art:** Placeholder colored panels for MVP; replace post-deadline if desired
