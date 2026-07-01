# ANamelessWorld — Session Context

## Last Session (large — spanned integration, a full redesign, and Block G start)

**Battle-start cinematic wired** into TestLevel Level BP: ChapterCard (over idle field) → dialogue → BattleCommenced → combat. Made **WBP_CutsceneScreen reusable** (SlideImages/SlideTexts exposed + Instance Editable/Expose-on-Spawn; `OnFinished` event dispatcher replaced hardcoded Open Level; Branch uses `SlideImages.Length`). Fixed: dialogue box hidden until a line plays; all battle UI revealed together after BattleCommenced (deleted a stale validated-GET that dead-ended the chain).

**Repo hygiene:** scrubbed ALL Claude/AI mentions from git history + tracked files; ignore rules for CLAUDE.md/AGENTS.md/memory/.claude moved to local `.git/info/exclude`. Repo is shown to teacher — keep it clean, **no co-author trailers, no AI mentions anywhere**.

**Phase 1 redesigned → depth-first** ("World 1 / Chapter 1 to near-final quality"); Phase 2 = breadth. New blocks **G–M** in ROADMAP.md.

**Combat design locked** (DESIGN_RATIONALE §6): Nameless = **pure manipulation, zero direct damage**. **Confuse** (nearest-creature, dmg-buff + low accuracy) replaces Provoke. **Boss immune to status** = unkitable clock, killed via confused mobs + environment. **Intimidate** → displacement/AoE. **Embolden** → Chapter 2 (needs a companion). **Interact** = core verb (Ch1 = rigged bookshelves). Movement ≈ equal; safety from control, not speed.

**Controls** (§7): modal LMB (Idle = move, armed ability = target; RMB/Esc cancel), 1/2/3 ability hotkeys, **tactical camera built in Block G**. **Data** (§8): migrate stats to DataTables/CSV in a dedicated session (Block I2) before balance. **UX** (§9): FE colours — **cyan = movement, red = attack/threat**.

**Block G built so far:** NavMeshBoundsVolume in TestLevel (floor bakes green). Click-to-move (`APlayerCharacter::OnMoveClicked`, LMB, cursor enabled in BeginPlay). Movement **radius rule** (`ABaseCharacter::MoveRange`, path-distance check rejects far/partial clicks). **M_MoveRange** decal range indicator on BP_PlayerCharacter (character mesh **Receives Decals off**). Build.cs += AIModule, NavigationSystem.

## Next Task
**Block G — Tactical camera.** Build rotate / zoom / pan for the battlefield.
- **Decide first:** decoupled camera *rig* (free pan around field) vs spring-arm attached to Nameless (rotate/zoom around him). Player currently views a placed **CameraActor** (set via Set View Target in TestLevel BeginPlay) — likely build the rig off/replacing that.
- **Then remaining Block G:** action economy (Move + Action budgets) + turn gating + the modal-input state machine; enemy movement; (later polish) drive decal size from MoveRange + show only on player's turn.

## Workflow (important)
- Claude **writes** code as chat blocks and says exactly **what it does / which file / where**; the **USER applies it** to the game's C++/Blueprint source. NOT vibecoding. Claude may directly edit docs/config/git only.
- Session structure: **brief → C++ → Blueprint wiring → 3–5 Feynman questions.** No end-of-response recaps.
- User is learning C++/gamedev — explain concepts, not just mechanics.

## Key Notes
- **No Claude/AI anywhere in the repo** (files, commit messages, no co-author trailer).
- Adding/removing a `UPROPERTY`/`UFUNCTION`/`UCLASS` member → **full compile + UE restart** (Live Coding only patches existing function bodies).
- Legacy input: `Config/DefaultInput.ini` ActionMappings + `BindAction`. `MoveClick = LeftMouseButton`.
- Don't paste large error logs — first 5–10 lines. Hot reload errors clear on full UE restart.
- GE modifier order: MaxHealth before Health to avoid clamping.
- Local command-line builds: `-Log=/private/tmp/ANamelessWorld-UBT.log -NoUBA`.
- NOTE: some older "Key Design Decisions" in CLAUDE.md (Protagonist kit, Basic/Heavy attacks) are **superseded** by the manipulation redesign — DESIGN_RATIONALE §6 is the source of truth.

## Session History
| Session | Topic | Status |
|---|---|---|
| 1–17 | Core: AttributeSet, ASC, TurnManager, abilities, dice, models, battle UI, turn strip | ✓ |
| 18–19 | Dialogue system + styling | ✓ |
| 20 | Main menu, intro cutscene, BGM | ✓ |
| 22 pt1 | Chapter-loop assets + screen widgets, Cinzel font, DESIGN_RATIONALE | ✓ |
| 22 pt2 | Reusable cutscene widget; battle-start cinematic wired into TestLevel | ✓ |
| 23 | Phase-1 depth-first redesign + manipulation combat design; controls/data/UX decisions; **Block G**: navmesh, click-to-move, movement radius + decal indicator | ◐ Block G in progress |
