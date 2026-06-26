# ANamelessWorld — Session Context

## Last Session
**Session 22 (part 1)** — in progress, assets + screen widgets + design doc done 2026-06-26
(Session 21 BGM was merged into Session 20.)

## Last Completed Task
Main menu and intro cutscene:
- MainMenuLevel: empty level set as Game Default Map
- WBP_MainMenu: full-screen background (ANW_Background), logo image (ANW_Logo), custom stone/rune button (ANW_Button) with "New Game" text, anchored top-left
- WBP_CutsceneScreen: 6-slide auto-advancing intro cutscene with FadeIn/FadeOut widget animations, Skip button, custom events ShowNextSlide/LoadGame/NextSlide
- Slides: ANW_Intro-1 "Too boring for you huh?" → ANW_Intro-2 "Our life stories are just trashes to you eh?" → 3x black screens with Nameless's monologue → ANW_Background "starting from my world."
- BP_GameInstance: PlayBGM function using Spawn Sound 2D with Persist Across Level Transition, called from MainMenuLevel Level Blueprint after widget Add to Viewport via Cast to BP_GameInstance
- BGM-1: modern fantasy cinematic score, looping, persists from main menu through battle

## Next Task
**Session 22 (part 2 / Session 23) — Integration.** Wire the full chapter loop into the **TestLevel Level Blueprint** (where the TurnManager is created/driven and `OnCombatEnded` is bound — currently `OnCombatEnded_Event → Branch(PlayerWon) → Print "Player Won"/"Game Over"`).
- **Battle start:** ChapterCard → (in-battle dialogue, later) → BattleCommenced → then StartCombat (don't start turns until intro sequence finishes; characters idle meanwhile, no engine pause needed).
- **Battle end (on OnCombatEnded PlayerWon=true):** ending slides (4) → WBP_WorldFinished → Chapter 2 card → Chapter 2 opening slide (1 slide for now).
- **Make WBP_CutsceneScreen reusable:** expose SlideImages/SlideTexts (Instance Editable / Expose on Spawn) + replace hardcoded `Open Level` with an **OnFinished event dispatcher** so caller decides next step. Reuse it for intro / ending / ch2 slides instead of duplicating.
- **BGM:** fade out the persistent GameInstance BGM when WBP_WorldFinished shows (dead silence). Per-world color/punctuation + BGM driven by `CurrentWorldIndex` later — World 1 hardcodes red "FINISHED.".

### Session 22 assets/widgets already built
- Art (in Content/UI): Battle Commenced graphic, Chapter 1 logo "A Novel Origin", Chapter 2 logo "Here Comes the Action", ending slides ×4 (portal open / foe reaching / mouth close-up / walking through dissolving), Chapter 2 opening slide ×1 (steps out into modern warzone). Each chapter logo carries a small purple-gold book motif (Nameless = outsider).
- Font: F_Cinzel (composite: Regular/Bold/Black) in Content/UI/Fonts. Dialogue stays on its readable font, NOT Cinzel.
- Widgets: WBP_ChapterCard (DarkOverlay + ChapterText + LogoImage, ChapterCardAnim slam+fade), WBP_BattleCommenced (CommencedImage, CommencedAnim punchy slam), WBP_WorldFinished (black bg + WorldText "WORLD 1 . . ." + red FinishedText "FINISHED" bottom-right, WorldFinishedAnim slow eerie fade).
- DESIGN_RATIONALE.md added at repo root (decision/why/rejected format).

### TestLevel Level Blueprint map (current — the integration target)
**Event BeginPlay (one synchronous chain, in order):**
1. Set View Target with Blend → `CameraActor` (via Get Player Controller).
2. Construct Turn Manager → SET `Turn Manager` var.
3. Add Combatant ×3 — BP_Player, BP_Enemy, BP_Boss.
4. Setup Combat (BP_Player) [In Turn Manager, In Target] → Add Target ×2 to player (BP_Enemy, BP_Boss) → Setup Combat (BP_Enemy) + Setup Combat (BP_Boss) [In Turn Manager, In Player Target].
5. Create WBP_BattleHUD → SET `HUD`; set HUD's Player/Enemy/Boss Character + Turn Manager refs → Add to Viewport → **SET HUD Visibility = Hidden**.
6. Create WBP_DialogueBox → Add to Viewport → SET `Dialogue Box Widget`.
7. Bind OnLineShown + OnDialogueFinished (Dialogue Component on BP_Player).
8. **Start Dialogue** (pre-battle, "The Narrator") → **Start Combat** (Turn Manager) → **Refresh Strip** (Turn Order Strip via GetTurnOrder).
9. Create WBP_CommandMenu → SET `Command Menu` → Setup Command Menu [In Player Character = BP_Player] → Add to Viewport.
10. Bind OnCombatEnded (Turn Manager) → `OnCombatEnded_Event`.

**Custom events:**
- `OnDialogueStarted(SpeakerName, LineText)` → Show Line on Dialogue Box.
- `OnDialogueFinished` → Hide Dialogue → Sequence(Then0/1/2) → set HUD Visibility = Visible, Command Menu Visibility = Visible.
- `OnCombatEnded_Event(Player Won)` → Branch → True: Print "Player Won!"; False: Print "Game Over…". **(These Prints are the placeholders to replace.)**

**Level BP vars:** `Turn Manager`, `HUD`, `Dialogue Box Widget`, `Command Menu`.

**Integration seams for next session:**
- **Battle start:** play WBP_ChapterCard FIRST (over the greyed, idle battlefield) before Start Dialogue. Play WBP_BattleCommenced AFTER `OnDialogueFinished`, right before combat actually begins. PROBLEM: Start Dialogue → Start Combat are currently chained synchronously, so combat would begin during dialogue/card. FIX: pull `Start Combat` out of the BeginPlay chain and call it from a delayed point after BattleCommenced finishes (HUD/Command Menu reveal already waits on OnDialogueFinished, so that part is fine). No engine pause needed — characters just idle until Start Combat.
- **Battle end:** in `OnCombatEnded_Event`, replace the True-branch Print with the ending sequence → ending slides (reuse a configurable WBP_CutsceneScreen) → WBP_WorldFinished (fade out the persistent GameInstance BGM here for silence) → Chapter 2 card → Chapter 2 opening slide.

## Key Notes
- Don't paste large error logs in chat — first 5-10 lines is enough to diagnose
- Hot reload errors always clear on full UE5 restart
- GE modifier order matters: MaxHealth must be set before Health to avoid clamping
- For local command-line builds, use `-Log=/private/tmp/ANamelessWorld-UBT.log -NoUBA`; default UBT logging/UBA can hit sandbox permission issues

## Session History
| Session | Topic | Status |
|---|---|---|
| 1 | Project setup, UCRPGAttributeSet | ✓ Done |
| 2 | ABaseCharacter + ASC | ✓ Done |
| 3 | UInventoryComponent | ✓ Done |
| 4 | UTurnManager | ✓ Done |
| 5 | GA_BasicAttack C++ | ✓ Done |
| 6 | GE_DamageInstant + end-to-end damage test | ✓ Done |
| 7 | APlayerCharacter, AEnemyCharacter, key binding, UTurnManager turn loop | ✓ Done |
| 8 | Win/lose detection, WBP_BattleHUD with health bars and turn indicator | ✓ Done |
| 9 | Enemy moveset, ABossCharacter, 3-combatant fight | ✓ Done |
| 10 | Command menu, Cycle Target, Boss HP bar | ✓ Done |
| 11 | Data Assets — UCRPGCharacterData, per-character DA assignments | ✓ Done |
| 12 | Protagonist's real kit — Embolden, Intimidate, Provoke | ✓ Done |
| 13 | d20 dice system — UCRPGCombatLibrary, hit/miss, Stunned skip, Enraged self-damage | ✓ Done |
| 14 | Character models + animations — ABP_Nameless/Enemy/Boss, FProperty anim wiring | ✓ Done |
| 15 | Battle log — UE_LOG damage/HP tracking in AttributeSet | ✓ Done |
| 16 | Turn order indicator UI strip — WBP_TurnSlot, WBP_TurnOrderStrip, GetTurnOrder() | ✓ Done |
| 17 | Fix simultaneous turns — AI EndTurn timing, enemy animations | ✓ Done |
| 18 | Dialogue system foundation — component, widget base, advance input | ✓ Done |
| 19 | Dialogue box styling — FFT-style panel, yellow speaker name, white dialogue text | ✓ Done |
| 20 | Main menu — background art, logo, custom button, 6-slide cutscene, BGM via GameInstance | ✓ Done |
| 22 | Chapter loop assets + screen widgets (ChapterCard, BattleCommenced, WorldFinished), Cinzel font, DESIGN_RATIONALE.md | ◐ Part 1 done — integration pending |
