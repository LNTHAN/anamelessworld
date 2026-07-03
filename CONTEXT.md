# ANamelessWorld — Session Context

## Last Session (large — spanned integration, a full redesign, and Block G start)

**Battle-start cinematic wired** into TestLevel Level BP: ChapterCard (over idle field) → dialogue → BattleCommenced → combat. Made **WBP_CutsceneScreen reusable** (SlideImages/SlideTexts exposed + Instance Editable/Expose-on-Spawn; `OnFinished` event dispatcher replaced hardcoded Open Level; Branch uses `SlideImages.Length`). Fixed: dialogue box hidden until a line plays; all battle UI revealed together after BattleCommenced (deleted a stale validated-GET that dead-ended the chain).

**Repo hygiene:** scrubbed ALL Claude/AI mentions from git history + tracked files; ignore rules for CLAUDE.md/AGENTS.md/memory/.claude moved to local `.git/info/exclude`. Repo is shown to teacher — keep it clean, **no co-author trailers, no AI mentions anywhere**.

**Phase 1 redesigned → depth-first** ("World 1 / Chapter 1 to near-final quality"); Phase 2 = breadth. New blocks **G–M** in ROADMAP.md.

**Combat design locked** (DESIGN_RATIONALE §6): Nameless = **pure manipulation, zero direct damage**. **Confuse** (nearest-creature, dmg-buff + low accuracy) replaces Provoke. **Boss immune to status** = unkitable clock, killed via confused mobs + environment. **Intimidate** → displacement/AoE. **Embolden** → Chapter 2 (needs a companion). **Interact** = core verb (Ch1 = rigged bookshelves). Movement ≈ equal; safety from control, not speed.

**Controls** (§7): modal LMB (Idle = move, armed ability = target; RMB/Esc cancel), 1/2/3 ability hotkeys, **tactical camera built in Block G**. **Data** (§8): migrate stats to DataTables/CSV in a dedicated session (Block I2) before balance. **UX** (§9): FE colours — **cyan = movement, red = attack/threat**.

**Block G built so far:** NavMeshBoundsVolume in TestLevel (floor bakes green). Click-to-move (`APlayerCharacter::OnMoveClicked`, LMB, cursor enabled in BeginPlay). Movement **radius rule** (`ABaseCharacter::MoveRange`, path-distance check rejects far/partial clicks). **M_MoveRange** decal range indicator on BP_PlayerCharacter (character mesh **Receives Decals off**). Build.cs += AIModule, NavigationSystem.

## Tactical Camera — DONE (decoupled rig)
Chose the **decoupled rig**: controller possesses the camera, commands Nameless from afar.
- **`ATacticalCameraPawn`** (Public/Private **Camera/**): Pivot(root)→SpringArm→Camera. WASD pan (yaw-relative, ground plane), Q/E orbit, wheel zoom (eased `TargetArmLength` in Tick). Tunables `PanSpeed/RotateSpeed/ZoomStep/Min/MaxZoom` are EditAnywhere.
- **`ATacticalPlayerController`** (files live in **Camera/** folder, class name says Controller): the "brain". BeginPlay turns on cursor + `GetActorOfClass` to cache `ControlledCharacter`. `SetupInputComponent` binds MoveClick/Attack/AdvanceDialogue → forwards to Nameless. `OnMoveClicked` does the cursor trace, hands the point to `TryMoveTo`.
- **`APlayerCharacter`**: old `OnMoveClicked` refactored → **`TryMoveTo(FVector)`** (range check + navmesh path + `SimpleMoveToLocation(GetController(),…)`). MoveClick binding removed. Its own input bindings + BeginPlay cursor block are now dead leftovers (controller owns them) — not yet cleaned.
- **BP wiring:** `BP_TacticalGameMode` (PlayerController=BP_TacticalPlayerController, DefaultPawn=BP_TacticalCameraRig) set as TestLevel **GameMode Override**. `BP_PlayerCharacter`: Auto Possess Player=**Disabled**, Auto Possess AI=**Placed in World or Spawned**, AIControllerClass=**AIController** (this is what drives `TryMoveTo`). BP assets in **Content/Camera/**.
- **Gotcha (fixed):** TestLevel Level-BP `Set View Target with Blend`→old CameraActor was overriding the rig; **deleted that node**. A possessed pawn's camera shows automatically — no Set View Target needed.
- **Input:** DefaultInput.ini AxisMappings CamPanForward(W/S) CamPanRight(D/A) CamRotate(E/Q) CamZoom(MouseWheelAxis).

## Action economy + turn gating — DONE
- **Stocks on `ABaseCharacter`:** `bMoveAvailable` + `bActionAvailable` (BlueprintReadOnly) + `ResetTurnResources()` (refills both). `UTurnManager::BeginTurn` calls `ResetTurnResources()` on the active combatant → everyone starts their turn full.
- **Player gating/spend:** `TryMoveTo` refuses unless state==PlayerTurn AND `bMoveAvailable`; spends Move on a committed (in-range) walk only — rejected clicks cost nothing. `FireAbility` refuses unless PlayerTurn AND `bActionAvailable`; spends Action; **removed the old 2s auto-EndTurn timer** — abilities no longer end the turn.
- **Explicit End Turn:** `APlayerCharacter::EndPlayerTurn()` (BlueprintCallable, guards on PlayerTurn) → `TurnManager->EndTurn()`. Wired to an **End Turn button** in WBP_CommandMenu (button reads the widget's `Player Character` variable, same pattern as the ability buttons). `EndTurnNow`/`TurnEndTimerHandle` removed.
- **CRITICAL FIX (camera-refactor fallout):** `IsPlayerControlled()` broke as the player identity test — since the rig refactor, Nameless is AIController-possessed, so the engine no longer sees him as player-controlled. TurnManager was flagging him an enemy → instant "Game Over" + would mislabel his turn as EnemyTurn (blocking his own gates). Fixed with a **possession-proof** `virtual bool IsPlayerCharacter()` on ABaseCharacter (false) overridden in APlayerCharacter (true); replaced all 3 `IsPlayerControlled()` uses in UTurnManager.
- **Enemy turn flow unchanged** (ExecuteAITurn via OnTurnStarted → 2s → EndTurn). Enemies inherit the stocks but don't use them yet (enemy-movement session).

## Deferred idea — auto-end turn when both stocks spent
Nice QoL (end the turn automatically once Move + Action are both used), but do it
**with** the **move-completed callback**, not now: `SimpleMoveToLocation` is async, so a
naive "both booleans false → EndTurn" cuts the walk/attack animation short. Build the
"AIController reached destination" signal during **enemy movement** (the AI needs it too),
then auto-end can wait for move-arrival + animation-finish. Until then End Turn is explicit.

## Modal-input state machine — DONE
Idle ↔ armed-ability, all on `ATacticalPlayerController` (the "brain").
- **`ArmedAbilityTag` (FName)** on the controller — `NAME_None` = Idle, else = Armed. No enum needed; this one field *is* the state machine.
- **`ArmAbility(FName)`** (BlueprintCallable) — arms a tag, gated on PlayerTurn + `bActionAvailable` (fails fast, same gate `FireAbility` re-checks anyway).
- **`OnMoveClicked`** now branches: Armed → cursor trace, `Cast<ABaseCharacter>` the hit, fire-and-clear on a hit, **stay armed on a miss** (armed clicks never fall through to movement). Idle → unchanged floor-click move.
- **`OnCancelPressed`** (RMB + Esc) unconditionally clears `ArmedAbilityTag`.
- **`APlayerCharacter::FireAbilityAtTarget(FName, ABaseCharacter*)`** — new public entry point; sets `CurrentTarget` then reuses the existing private `FireAbility` (same gating, no duplicated logic).
- **1/2/3 hotkeys** — thin no-arg wrappers (`OnAbilityOnePressed` etc., legacy `BindAction` can't carry a payload) calling `ArmAbility` with the Embolden/Intimidate/Provoke tags.
- **BP wiring:** `WBP_CommandMenu` gained a `TacticalController` variable, set via a new `In Tactical Controller` param on its `Setup Command Menu` function (fed by `Get Player Controller` → **Cast to Pure** `ATacticalPlayerController` in the Level BP, alongside the existing `In Player Character` wiring). All three ability buttons rewired from direct `Use Embolden/Intimidate/Provoke` calls to `Get TacticalController → Arm Ability`. **Cycle Target button removed** (grepped — `CurrentTarget` has no other reader; redundant now that clicking a target sets it directly via `FireAbilityAtTarget`). C++ `CycleTarget()`/`AllTargets`/`AddTarget` left in place, unused — not cleaned up yet, user's call whether/when.
- **Bug found + fixed:** target-click traced on `ECC_Visibility`, same channel as `MoveClick` — but character capsules apparently **don't block Visibility** (likely intentional, so floor-clicks aren't obstructed by standing characters), so the trace always fell through to the floor underneath. Fixed by tracing armed clicks on **`ECC_Pawn`** instead (what `CharacterMovementComponent` already relies on for pawn-blocking) — floor clicks keep using `ECC_Visibility`.
- **Bug found + fixed:** Intimidate and Embolden buttons' `Arm Ability` nodes had crossed-prefix tag typos (`Ability.Support.Intimidate`, `Ability.Debuff.Embolden` — neither registered). Corrected to `Ability.Debuff.Intimidate` / `Ability.Support.Embolden` (confirmed against `DefaultGameplayTags.ini`). Both were silently burning the player's Action for no effect.
- No UI feedback yet for "an ability is armed" (Block K polish item, not blocking).

## Camera spawn-drift bug — FIXED (unrelated, but a real gotcha)
Zoom appeared to creep further out on every Play→Stop cycle, even with zero input. Root cause: **`TestLevel` has no `PlayerStart`**, and Play mode is **Selected Viewport** — with no `PlayerStart`, GameMode falls back to spawning the pawn at the editor viewport's current camera transform, and Stop syncs the viewport back to wherever PIE ended → compounds every cycle in a single editor session (confirmed by the `FindPlayerStart: NO PLAYERSTART` line in the log). `SpringArm->TargetArmLength` itself was never wrong (verified always `1400`) — this was 100% a spawn-position issue, not a zoom-code bug. **Fix: added a `PlayerStart` to `TestLevel`.** Doesn't require changing Play mode. A `BeginPlay` override was briefly added to `ATacticalCameraPawn` chasing the wrong theory — reverted, not needed.

## Decision — movement stays eager (no undo)
Considered folding in tentative-move-with-undo (FFT/Fire Emblem model) alongside the modal
state machine. **Rejected** — user wants to follow **Larian/BG3 philosophy**: movement is a
real-time committed event, not a revertible preview. Reasoning: BG3 keeps movement permanent
because things react *mid-walk* (opportunity attacks, trap/perception rolls, hazard surfaces,
fog-of-war reveal) — undo would let players scout/mine-sweep for free. ANamelessWorld doesn't
have any of those systems yet, but the user wants the *foundation* (movement = committed) in
place now so that when traps/stealth/reactive mechanics do get designed, they have the right
model to hook into, rather than retrofitting permanence later. **`TryMoveTo` stays exactly as
it is today** — click = walk + spend `bMoveAvailable` immediately. No `TurnStartLocation` / no
`ConfirmMove` / `CancelMove` on APlayerCharacter.

## Decision — Block H2 reordered to after I2, not right after H
Drafted H2 (progression) as the session right after modal-input, then reconsidered before
writing any code: `GA_Provoke`/`GA_Intimidate` are about to be reworked in Block H (Confuse,
displacement), and only `GA_Intimidate` currently even has a real success/fail roll to hook
XP into (`GA_Embolden`/`GA_Provoke` apply unconditionally — no roll exists yet). Wiring
Mana-cost + XP-on-success onto abilities mid-rework is exactly the "throwaway prototype"
pattern DESIGN_RATIONALE §5 warns against. **New order: H (kit rework) → I (Interact) → I2
(DataTables) → H2 (progression, built data-driven from I2 instead of hardcoded per-ability
GameplayEffect assets) → J.** ROADMAP.md updated to match.

## Block H, part 1 — retire Basic Attack + Confuse rework — DONE
Split Block H into two passes (see below for why). Part 1, done and verified this session:
- **Player's Basic Attack retired:** `APlayerCharacter::OnAttackPressed` no longer calls
  `FireAbility("Ability.Attack.Basic")` — dialogue-advance fallback only now. `GA_BasicAttack`
  itself, `GE_DamageInstant`, `GE_HeavyStrike` all untouched — **enemies still use this exact
  ability/class to attack Nameless**, retiring was player-access-only.
- **`GA_BasicAttack` gained a Disadvantage roll**, mirroring its existing `State.Advantage`
  check — new `State.Confused` check, `RollWithDisadvantage()`. Both cancel out if present
  together (D&D rule).
- **`AEnemyCharacter::ExecuteAITurn` reworked**: `State.Enraged` handling → `State.Confused`
  handling. New `FindNearestOtherCombatant()` (via `TurnManager->GetTurnOrder()`, excludes
  self + dead) picks the target — confirmed in testing it correctly picked the *other enemy*
  over the player. Forced `Ability.Attack.Heavy` (the "damage buff" — reuses existing
  `GE_HeavyStrike` instead of a new bonus-damage system) — Disadvantage now comes free from
  `GA_BasicAttack`'s own check, so the old manual roll + self-damage-on-miss quirk is gone
  (wasn't part of Confuse's spec anyway).
- **`GE_Provoke` now grants `State.Confused`** (was `State.Enraged`); `DefaultGameplayTags.ini`
  updated to match (`State.Enraged` removed, `State.Confused` added).
- **Deliberately NOT renamed yet:** `Ability.Debuff.Provoke` trigger tag and the command-menu
  button still say "Provoke" — the ability behaves as Confuse now, just under the old name/tag,
  to avoid a window where the button's Tag Name and the ability's trigger tag are out of sync.
  Rename both together next time this is picked up.
- **Verified end-to-end via Output Log:** Confuse cast → next enemy turn correctly redirected
  to the nearest other combatant, roll showed `[Disadvantage]`, turn flow continued normally.

## Next Task
**Block H, part 2 — Intimidate → displacement + AoE fear, then UI pass.** Bigger/more novel
than Confuse was — nothing like "reposition a target" or "hit multiple targets in a radius"
exists in the codebase yet (Confuse reused the old Enraged AI skeleton almost directly;
Intimidate has no equivalent to build from). Do the Intimidate rework, **then** the deferred
rename pass together: `Ability.Debuff.Provoke` → `Ability.Debuff.Confuse`, command-menu button
labels/Tag Names for both Confuse and the reworked Intimidate, hide/remove the Embolden button
(deferred to Chapter 2 per DESIGN_RATIONALE §6). Source of truth: DESIGN_RATIONALE §6.

**Queued after H:** Block I (Environment & Interact) → I2 (data architecture) → **H2**
(progression — XP/leveling/Mana, see decision above) → J (AI/boss/balance).

**Still pending from Block G** (queue whenever convenient): enemy movement; (later polish)
drive decal size from MoveRange + show only on player's turn; optional cleanup of dead
input/cursor code in APlayerCharacter; optional cleanup of now-unused
`CycleTarget()`/`AllTargets`/`AddTarget`.

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
- **Character capsules don't block `ECC_Visibility`** — floor-clicks (`MoveClick`) rely on this so standing characters don't obstruct movement clicks. Any future click-to-target trace (Confuse, Interact, etc.) must use **`ECC_Pawn`** instead, not `ECC_Visibility`.
- **Every level needs a `PlayerStart`.** Without one, GameMode falls back to spawning at the editor viewport's camera position (Selected Viewport play mode) — compounds into visible camera drift over repeated Play/Stop cycles in one editor session. `TestLevel` now has one; check new levels too.

## Session History
| Session | Topic | Status |
|---|---|---|
| 1–17 | Core: AttributeSet, ASC, TurnManager, abilities, dice, models, battle UI, turn strip | ✓ |
| 18–19 | Dialogue system + styling | ✓ |
| 20 | Main menu, intro cutscene, BGM | ✓ |
| 22 pt1 | Chapter-loop assets + screen widgets, Cinzel font, DESIGN_RATIONALE | ✓ |
| 22 pt2 | Reusable cutscene widget; battle-start cinematic wired into TestLevel | ✓ |
| 23 | Phase-1 depth-first redesign + manipulation combat design; controls/data/UX decisions; **Block G**: navmesh, click-to-move, movement radius + decal indicator | ◐ Block G in progress |
| 24 | **Block G — tactical camera**: decoupled rig pawn + tactical PlayerController possesses it, commands Nameless via `TryMoveTo`; AI-driver possession; WASD/QE/wheel; removed old Set View Target | ◐ Block G in progress |
| 25 | **Block G — action economy + turn gating**: Move/Action stocks on ABaseCharacter, refill in BeginTurn, gate+spend in TryMoveTo/FireAbility, explicit End Turn button; fixed IsPlayerControlled→IsPlayerCharacter (camera-refactor fallout) | ◐ Block G in progress |
| 26 | **Block G — modal-input state machine**: ArmedAbilityTag + ArmAbility on ATacticalPlayerController, FireAbilityAtTarget on APlayerCharacter, 1/2/3 hotkeys, WBP_CommandMenu rewired (arm-then-click-target, Cycle Target removed); fixed target-click trace (ECC_Visibility→ECC_Pawn); fixed unrelated PlayerStart-missing camera-drift bug. Scoped out tentative-move/undo (Larian-philosophy decision). Scoped in Block H2 (XP/leveling/Mana) for next session. **Known bug to fix first: Intimidate button tag typo.** | ◐ Block G in progress |
