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

## Decision — Block H part 2 (Intimidate) moved after Block I
Displacement + AoE don't strictly *need* new geometry to work (TestLevel's existing floor +
NavMeshBoundsVolume is enough to validate a push landing somewhere legal), but the actual
DESIGN_RATIONALE §6 payoff — "herding enemies into hazards, e.g. under a rigged bookshelf" —
depends on Block I's environment work existing. Testing displacement against today's bare
open floor would be functional but not dramatic. **User chose to hold Intimidate until real
terrain/hazards exist**, so it can be tested properly against something worth pushing enemies
into, rather than build it twice (once thin, once for real).

## Block I, phase 1 — level layout + camera focus — DONE
**Library room greybox built** in TestLevel (20×20m, floor Plane was already Scale 20 at origin,
NavMeshBoundsVolume already covered it). Perimeter walls (cube StaticMeshActors, 20cm thick,
300cm tall, west wall split for a 150cm door gap near Nameless). Five staggered aisle rows of
bookshelf cubes (30cm thick, 200cm tall) through the middle — `Shelf_RowN_1/2`, with the middle
row renamed `Shelf_Rigged_N/S` (future Interact target, no logic yet). Two `Elevation_West/East`
book-stack platforms (30cm tall — low enough the navmesh steps onto them with no ramp needed;
aesthetic/positional only for now, no mechanical rule — see DESIGN_RATIONALE §5). Combatants
repositioned: Nameless SW near door, Enemy mid-east, Boss NE corner (entrenched). `BattleFieldCenter`
TargetPoint placed at (0,0,100).
- **Capsule-height gotcha:** a character's `Location.Z` must equal its Capsule **Half Height**
  (found via Details search "capsule") — it's the capsule *center*, not the feet. All three are
  88. Setting Z=0 sinks them into the floor.
- **PlayerStart is only the camera-rig spawn**, not a gameplay position (Nameless is AI-possessed,
  positioned independently). Placed outside the room at (−1400,0,1400) for a clean establishing
  vantage — but this is now largely moot, see camera focus below.

**Turn-based camera focus — DONE** (`ATacticalCameraPawn` + `ATacticalPlayerController`):
- **Establishing shot:** `BeginPlay` snaps the pivot to `BattleFieldCenter` (via
  `GetActorOfClass<ATargetPoint>`) at `WideZoom` (2200). Also fixed the old spawn-drift by
  starting `TargetZoom` at MinZoom.
- **Focus-on-turn:** controller's `BindToTurnManager()` (called from Level BP right before
  `Start Combat`) subscribes `OnCombatTurnStarted` to `TurnManager->OnTurnStarted`; each turn it
  calls `CameraPawn->FocusOn(active->GetActorLocation() + (0,0,150))`. The **+150 Z offset is
  required** — focusing on the capsule center puts the pivot *inside* the character's own collision,
  triggering the spring-arm wall pull-in (camera-inside-character bug).
- **`FocusOn` eases via a `bIsFocusing` flag**, NOT a distance check: Tick eases toward
  `TargetLocation` only while the flag is true, clears it on arrival (within 5uu), never touches
  location again until the next `FocusOn`. A distance check re-triggered every time WASD panned away
  → camera fought manual pan / drifted back. Flag fixes it: manual pan is free between turns.
- **Zoom:** added `=`/`-` keyboard zoom (macOS trackpad has no scroll wheel) alongside MouseWheelAxis.
- **Rotation already existed** (Q/E free orbit from Block G) — user was fine with it, no snap-rotate added.

## Block I, phase 2 — Interact framework + rigged bookshelf — DONE
**Two-stage proximity trap** (design locked with user, DESIGN_RATIONALE §6 rewritten): Interact
**arms** a shelf (stage 1, costs the Action, Nameless must be in interact range); when a living
**enemy** enters its AoE it **springs** (stage 2), crushing **every** living character in the
blast — enemies, allies, and Nameless (physics is universal). Trigger is enemy-only (so arming
from inside the blast doesn't self-detonate); damage hits everyone.
- **`AInteractableActor`** (new class, `Public/Private Interactables/`): base for env objects.
  `Mesh` (root) + `TriggerSphere` (USphereComponent) + `TriggerRadius`/`DamageEffect` (EditAnywhere,
  designer data). `Arm()` (stage 1, virtual), `Detonate()` (stage 2), `OnSphereBeginOverlap` (the
  future enemy-enter trigger), `GetDistanceToBody()` (helper). One-shot `bTriggered` latch.
- **Detection is distance-to-**body**, not the origin sphere.** Both `Arm()`'s immediate sweep and
  `Detonate()`'s victim list use `GetAllActorsOfClass(ABaseCharacter)` + `GetDistanceToBody() <=
  TriggerRadius`. `GetDistanceToBody` = `Mesh->GetClosestPointOnCollision` (nearest surface point,
  0 if inside), origin-distance fallback. This fixed BOTH: long-shelf coverage AND the crush not
  depending on overlap-event tracking.
- **Player:** `APlayerCharacter::TryInteract(AInteractableActor*)` — gate (PlayerTurn +
  bActionAvailable) + range check (`GetDistanceToBody <= InteractRange`, default 200) + spend
  Action + `Arm()`. Returns bool so a too-far click stays armed (walk closer, retry).
- **Controller/modal input:** Interact reuses the `ArmedAbilityTag` machine via sentinel FName
  `Action.Interact` (never a real GameplayTag — never reaches FireAbility). `OnMoveClicked` armed
  branch: interact traces `ECC_Visibility` (shelf mesh blocks it, capsules don't) → cast
  `AInteractableActor` → `TryInteract`; abilities still trace `ECC_Pawn`. Key **4** = `OnInteractPressed`.
- **BUG fixed (stale armed state):** `ArmedAbilityTag` was never cleared between turns — a leftover
  armed Interact/ability hijacked every click on later turns (player "couldn't do anything"). Fixed:
  clear `ArmedAbilityTag = NAME_None` at the top of `OnCombatTurnStarted` (fires each turn-start).
- **BP:** `BP_RiggedBookshelf` (Content/Interactables/) parented to `AInteractableActor`, Mesh =
  shelf cube, DamageEffect = GE_HeavyStrike, TriggerRadius 300; replaced the `Shelf_Rigged_N/S`
  placeholder cubes in TestLevel. **Interact button** added to WBP_CommandMenu (Arm Ability →
  `Action.Interact`, same wiring as ability buttons).
- **Verified:** arm from inside blast → immediate crush of the in-range enemy AND Nameless (925 HP),
  confirming the physics rule.
- **Known cosmetic quirk (deferred):** the `[Combat]` log reads "victim dealt N to victim" (self-
  attribution) because the shelf has no ASC, so the spec is built from the victim's own ASC.
  `AddSourceObject(this)` points to the shelf but the log reads the instigator. Fix later by routing
  an instigator through the effect context; harmless.
- **Trap is inert in normal play until enemy movement exists** — the enemy-enter spring needs enemies
  to move; today only the arm-time immediate sweep can fire it. *(Now unblocked — see enemy movement.)*

## Turn-start intermission — DONE
Enemies no longer act the instant their turn starts (the swing used to fire while the camera was
still easing in, so you missed the animation). `AEnemyCharacter::ExecuteAITurn` is now a thin
handler: guard `!= this` → start `TurnStartTimerHandle` for `TurnStartDelay` (EditAnywhere, 1.5s,
tunable per enemy) → `PerformAITurn()` (the old decision+attack brain, renamed, unchanged). Camera
settles during the beat, then the enemy acts.
- **Player turn deliberately gets NO intermission** — the human doesn't auto-act, so their own
  think-time already covers the camera settle; a forced delay would just add input latency.
- **Deferred polish:** if clicking a move *while the camera is still panning in* feels jittery in
  playtest, add a lightweight "ignore clicks until focus finishes" gate in the controller using the
  camera pawn's existing `bIsFocusing` flag — NOT a blanket timer. 2-line follow-up, only if needed.

## Enemy movement — DONE
Enemies now path toward their target and only strike when close, capped to a per-turn budget.
Flow (all in `AEnemyCharacter`, after the turn-start intermission): `PerformAITurn` picks
`PendingTarget` + `PendingAttackTag` (confused → nearest combatant / forced Heavy; else PlayerTarget
/ Basic-Heavy roll) → `EngageTarget()` range-checks: in `AttackRange` (EditAnywhere, 200) → `AttackTarget()`
in place (no retreat); else `MoveTowardTarget()`.
- **Async move → arrival callback.** `MoveToActor`/`MoveToLocation` finish over many frames, so we
  can't `Move();Attack();`. Bind the AIController's `ReceiveMoveCompleted` → `OnMoveCompleted` re-checks
  range and either `AttackTarget()` or ends the turn. Requires enemies be **AI-possessed**
  (BP_Enemy/BP_Boss: AutoPossessAI = Placed in World or Spawned, AIControllerClass = AIController).
- **Per-turn MoveRange cap** (`ABaseCharacter::MoveRange`, 500): compute the nav path
  (`FindPathToLocationSynchronously`), then `PointAlongPath()` (file-static helper) picks the point
  `min(MoveRange, PathLength − StopDistance)` along it and `MoveToLocation`s there. Enemy within
  `MoveRange + StopDistance` (~600) reaches and strikes in one turn; farther ones chip in over turns.
- **Stop INSIDE AttackRange, not at its edge.** `StopDistance = AttackRange * 0.5` (≈100). Aiming at
  the exact 200-line failed: `MoveToLocation` halts within its acceptance radius (50), leaving the
  enemy ~250 out and perpetually just-out-of-range. Half-range-in gives slack the wobble can't eat.
- **Bind-once + unbind-on-complete.** `IsAlreadyBound` guard on bind; `RemoveDynamic` at the top of
  `OnMoveCompleted` so a stray second broadcast can't fire the attack twice (would double-hit once in range).
- **MoveToActor/MoveToLocation return value matters** — only `RequestSuccessful` fires the callback;
  `Failed`/`AlreadyAtGoal` are resolved inline or the turn hangs forever.
- **Graceful degrade:** no AIController → warn + end turn (don't stall).
- **Verified:** enemy at ~300 and boss at ~560 both walked in and struck; confused enemies path to
  the nearest combatant the same way. Enemy movement now makes the rigged-shelf enemy-enter spring live.

## Character-vs-character avoidance — DONE (Plan B, after a long detour)
Movers now path AROUND other characters (enemies AND Nameless), crisply. This ate a big chunk of the
session — RVO and Detour Crowd both failed/janked; the working answer is **per-turn nav obstacles**:
- Every character is a **dynamic navmesh obstacle** (`GetCapsuleComponent()->bDynamicObstacle=true` +
  `SetCanEverAffectNavigation(true)` in `ABaseCharacter::BeginPlay`; needs Runtime Generation=**Dynamic**).
- **The mover excludes itself:** `UTurnManager::BeginTurn` calls `ABaseCharacter::SetNavObstacleEnabled`
  — everyone carves EXCEPT the active combatant. So the mover never routes around (or churns on) its
  own footprint, and the carvers are all stationary → the navmesh is stable during the walk (no wiggle).
  The obstacle flip settles during the turn intermission / player think-time before anyone moves.
- **Enemies use plain `AIController`** (NOT the crowd controller — this is a pathing fix, not steering).
- **`MoveTowardTarget` projects its clamped destination** onto the navmesh (`ProjectPointToNavigation`)
  so it never aims at a just-off-mesh point and grinds a wall.
- **HUGE gotcha (cost hours):** dynamic-navmesh obstacle changes **don't apply until a full compile +
  fresh PIE** — Live Coding / half-restart leaves the nav data stale, so it *looks* like the obstacles
  aren't carving. When nav/obstacle behaviour looks wrong, FULL RESTART before trusting it.
- **`AEnemyAIController`** (Public/Private **AI/**, Detour Crowd via `UCrowdFollowingComponent`) still
  exists in the project but is **currently unused** (crowd approach abandoned). Safe to delete later.

## Rigged-bookshelf walk-in trigger — FIXED (deterministic poll, not overlap)
The enemy-enter spring never fired via physics: `OnSphereBeginOverlap` needs both the sphere AND the
capsule to generate overlap events, and the `TriggerSphere` (attached to the non-uniformly scaled shelf
mesh) had its radius collapsed to ~30% by inherited scale. Two physics fixes (capsule overlap events;
`SetUsingAbsoluteScale(true)` on the sphere) still didn't work reliably. **Replaced overlap with a
deterministic poll:** `Arm()` starts a 0.15s repeating `ProximityTimerHandle` → `CheckProximity()` does
the same `GetAllActorsOfClass` + `GetDistanceToBody <= TriggerRadius` sweep as the arm-time check;
`Detonate()` clears the timer. No collision channels / overlap events / component scale involved →
reliable. Verified: arm empty, enemy paths in, `crushed` on entry. (Sphere + `OnSphereBeginOverlap`
now dormant — can be deleted later.)

## Juice pass — rigged shelf detonation — DONE
First "make it feel like a game" pass, all on the rigged bookshelf. Pattern: **C++ decides WHEN,
Blueprint decides how it LOOKS**, via `BlueprintImplementableEvent`s the C++ fires.
- **Detonation sequence** (was instant): `CheckProximity` → `BeginDetonation(Trigger)` freezes the
  enemy (`AIController::StopMovement`), fires `OnTelegraph()` (BP "!!" beat), waits `DetonationDelay`
  (0.8s), then `Detonate()` (damage + `OnDetonated()`). So: enemy stops → telegraph → crush + shake.
- **Camera shake:** the PlayerCameraManager shake system does NOT work on the decoupled tactical rig.
  Solution = shake our own camera in C++: `ATacticalCameraPawn::TriggerShake()` sets `ShakeTimeRemaining`;
  Tick applies a decaying sine wobble to the Camera component's relative rotation. BP calls it via
  Get Controlled Pawn → Cast → TriggerShake, off the topple Timeline's **Finished** pin (shake on landing).
- **Topple animation:** BP Timeline (0.4s, Alpha 0→1) → `Set Relative Rotation` (Lerp 0→Roll 90).
  Needs a **base hinge**: added a `Pivot` SceneComponent as root, Mesh child lifted by half-height in
  `OnConstruction` (WYSIWYG — editor shows real position) so it rotates around its bottom edge, not center.
  Mesh Mobility must be **Movable**. Shelf actor origin is now the BASE — place with base on floor.
- **Directional blast:** replaced the circular AoE with a shelf-aligned rectangle — `IsInBlastZone`
  (dot products vs forward/right) + `GetBlastHalfExtents` (auto-sized from mesh bounds, thin axis
  expanded by `ToppleReach`). Hits front/back only, not sideways. Debug `DrawDebugBox` used to tune, removed.
- **VFX assets:** `CS_Detonation` (LegacyCameraShake) in Content/Effects; `OnTelegraph` is a Print String
  placeholder for now.
- **Lessons:** new `UFUNCTION`/BP events need a FULL editor restart to appear; a shape component inherits
  its parent's non-uniform scale (use `SetUsingAbsoluteScale`); `SetRelativeRotation` needs Movable
  mobility; center vs base pivot is the whole game for a topple; use `OnConstruction` (not BeginPlay)
  for editor-visible transforms.

## Juice — deferred polish (save for later)
- **Toppled shelf should linger then fade/despawn**, not vanish — user-requested next tweak.
- Real **"!!" telegraph VFX** (particle/widget) to replace the Print String; **dust/debris burst** on crush.
- **Fall toward the triggering enemy** (compute `ToppleSign` in `BeginDetonation`, multiply the topple angle).
- Swap greybox cube for a real **bookshelf model** (topple animation + pivot already carry over).

## Block H, part 2 — Intimidate displacement + Confuse rename — DONE
**Intimidate reworked** from single-target stun to **AoE fear / forced displacement** (`GA_Intimidate`,
DESIGN_RATIONALE §6): on cast, every living enemy within `IntimidateRadius` (EditDefaultsOnly, ~300 after
tuning; 600 was too wide) rolls INT-vs-WIS; each failer is flung straight **away from Nameless** up to its
own `MoveRange` via a capsule `SweepSingleByChannel(ECC_Pawn)` (ignores caster + self). Stops at the first
solid hit; **impact damage** (`DamageEffect` = GE_DamageInstant) on any terrain/enemy hit, **mutual damage**
if it slams another enemy. Nameless excluded from the sweep (no backdoor damage). **Enemies flung into a
rigged shelf's AoE → detonation** (the herd-into-hazard combo). Two-phase closest-first ordering was
considered for reliable enemy-vs-enemy billiards but NOT applied — the real cases work (both-in-AoE scatter;
one flung into a stationary out-of-AoE enemy = reliable mutual damage). Instant teleport for now; smooth
panic-slide + dust VFX + fall-over anim are a deferred juice pass (logic-first, same as the shelf).
- **Confuse rename** (deferred pass): tag `Ability.Debuff.Provoke` → `Ability.Debuff.Confuse` (config +
  `+GameplayTagRedirects` so old refs resolve — **gameplay tags only reload on a FULL editor restart**,
  bit us). Hotkey 3 → Confuse. WBP_CommandMenu: Provoke button relabeled "Confuse" + tag; **Embolden button
  hidden** (Collapsed, deferred to Ch2). Assets `BP_GA_Provoke`/`GE_Provoke` kept as legacy filenames.
- **Confuse now lasts 1 turn** (was forever/OP): `GE_Provoke` → **Infinite** duration; `AEnemyCharacter::EndTurnNow`
  strips it via `RemoveActiveEffectsWithGrantedTags(State.Confused)` after the confused turn resolves (removed
  at turn-END so the Disadvantage attack still sees the tag).
- **Characters face their target when attacking**: `ABaseCharacter::FaceActor(AActor*)` (instant horizontal
  SetActorRotation), called in `AEnemyCharacter::AttackTarget` and `APlayerCharacter::FireAbility` before the send.

## Deferred — turn-based status-duration SYSTEM (for variable-turn statuses)
Confuse is hardcoded to 1 turn. Making statuses last **N turns** (Confuse 2, Stun 1, etc.) needs a small
turn-counting SYSTEM (not just data) — e.g. TurnManager ticks active turn-statuses down each round, removes at 0.
The per-status **number** is data → build this WITH **I2** so durations are data-driven from the start.

## Next Task — Tactical UX loop
**Battle flow end-to-end:** click ability → target-select mode (skip for no-target AoE like Intimidate) →
**confirm/cancel step** (Space/button to confirm, Back button + Esc/RMB to abort) → a **forecast panel**
(both portraits — caster + target — with hit-%/damage-range + effect preview; abilities use dice so it's a
Fire-Emblem-style *chance* preview, not exact). This is the reusable **"AoE/ability targeting mode"** layered
onto the modal-input controller (`ArmedAbilityTag`). Build in 2 stages: (1) target-select + confirm/abort
input backbone, then (2) the forecast UI.

## SCOPE LOCKED — deadline-driven (see memory [[deadline-coding-final]])
Build doubles as a **coding-school final, needs a passable demonstrable build by end of Aug 2026** (~50 daily
sessions from 2026-07-09). **The current state is already roughly passable** — so remaining work is
high-value polish + a satisfying climax, NOT obligation. Priority order:
1. **Tactical UX loop** (above) — makes the battle flow feel right.
2. **Boss encounter + balance** (J) — the climax that proves the manipulation combat; single highest-value deliverable.
3. **Full loop wiring** — intro → battle → **win AND lose** screens → outro (a complete experience).
4. **Flair the user wants** (the fun part): retreat + collision animation (Intimidate's smooth panic-slide + fall-over,
   replacing the teleport), **floating combat text / ability damage numbers**, ability VFX, more screenshake, UI skin/juice.
5. **I2 data architecture** (portfolio value — shows clean architecture to a grader) + turn-based status-duration system; **H2** progression as time allows.
- **Guard the critical path from rabbit holes** — time-box exploration, ship "good enough." A complete rough slice beats a gorgeous half-finished one for a grade.
- Leave the last ~1–1.5 weeks as a **playtest/balance/bug-fix buffer**.

**Still pending from Block G** (later polish): drive decal size from MoveRange + show only on
player's turn; optional cleanup of dead
input/cursor code in APlayerCharacter; optional cleanup of now-unused
`CycleTarget()`/`AllTargets`/`AddTarget`.

## Workflow (important)
- Claude **writes** code as chat blocks and says exactly **what it does / which file / where**; the **USER applies it** to the game's C++/Blueprint source. NOT vibecoding. Claude may directly edit docs/config/git only.
- Session structure: **brief → C++ → Blueprint wiring → 3–5 Feynman questions.** No end-of-response recaps.
- User is learning C++/gamedev — explain concepts, not just mechanics.

## Key Notes
- **No Claude/AI anywhere in the repo** (files, commit messages, no co-author trailer).
- **Live Coding boundary (sharpened):** the line is *structure vs logic*. Adding/removing **any member variable** (reflected or not), adding/removing a function, or changing a function's **signature** → **full compile + UE restart** (they change the class's memory layout / function table). Only changes **inside an existing function body** (new logic, values, log lines) are Live-Coding-safe. Analogy: values patch live, structure needs a build.
- **New `UFUNCTION`/`UPROPERTY` don't appear in Blueprint search until a full editor restart** — a successful compile alone isn't enough (bit us twice: `ArmAbility`, `BindToTurnManager`).
- **Blueprint "Cast To" search strips the leading type-prefix letter** — search `Cast To TacticalPlayerController`, NOT `Cast To ATacticalPlayerController`.
- **Character `Location.Z` must equal the Capsule Half Height** (all 3 chars = 88), or the model sinks into / floats above the floor. It's the capsule *center*, not the feet.
- Legacy input: `Config/DefaultInput.ini` ActionMappings + `BindAction`. `MoveClick = LeftMouseButton`.
- Don't paste large error logs — first 5–10 lines. Hot reload errors clear on full UE restart.
- GE modifier order: MaxHealth before Health to avoid clamping.
- Local command-line builds: `-Log=/private/tmp/ANamelessWorld-UBT.log -NoUBA`.
- NOTE: some older "Key Design Decisions" in CLAUDE.md (Protagonist kit, Basic/Heavy attacks) are **superseded** by the manipulation redesign — DESIGN_RATIONALE §6 is the source of truth.
- **Character capsules don't block `ECC_Visibility`** — floor-clicks (`MoveClick`) rely on this so standing characters don't obstruct movement clicks. Any future click-to-target trace (Confuse, Interact, etc.) must use **`ECC_Pawn`** instead, not `ECC_Visibility`.
- **`bGenerateOverlapEvents` defaults to FALSE** on C++-created primitive components — `GetOverlappingActors()` / begin-overlap silently return nothing until you call `SetGenerateOverlapEvents(true)`. Cost us the first rigged-shelf crush.
- **Measure to the BODY, not the origin, for extended meshes.** A long shelf's origin is meters from its faces, so origin-distance range/AoE checks are wrong. Use `Mesh->GetClosestPointOnCollision(Point, Out)` (returns nearest-surface distance, 0 if inside) — see `AInteractableActor::GetDistanceToBody`.
- **`ArmedAbilityTag` must be cleared each turn** (done in `OnCombatTurnStarted`) — a leftover armed ability/interact hijacks all clicks on later turns.
- **AIController move is async — sequence via `ReceiveMoveCompleted`, never `Move();Attack();`.** Bind once (`IsAlreadyBound`) and `RemoveDynamic` on completion (a double broadcast = double attack). Check the `MoveTo*` return: only `RequestSuccessful` fires the callback; `Failed`/`AlreadyAtGoal` must be resolved inline or the turn hangs. A pawn needs AutoPossessAI set to have an AIController at all.
- **`MoveToLocation` stops within its acceptance radius**, so aim a margin INSIDE the target distance, not at the exact edge, or the agent parks just short forever (bit the enemy AttackRange approach).
- **Gameplay tags load ONCE at editor-process startup** — editing DefaultGameplayTags.ini (rename/add/redirect) needs a FULL app quit + relaunch to show; a recompile/Live Coding/level reopen won't do it. Use `+GameplayTagRedirects=(OldTagName=..,NewTagName=..)` so existing refs survive a rename.
- **Turn-based status removal**: for "lasts N turns", strip the tag manually (`ASC->RemoveActiveEffectsWithGrantedTags(...)`) at turn-END, with the GE set to **Infinite** duration — NOT a real-time GE timer (fragile in turn-based). Confuse uses this (1 turn). A general N-turn duration system is deferred (see Deferred section).
- **Dynamic-navmesh obstacle changes need a FULL compile + fresh PIE to apply** — Live Coding / half-restart leaves the nav data stale, so obstacles that ARE registered (logs confirm the flags) still won't carve. When nav/obstacle behaviour looks wrong, full-restart before debugging further. Cost hours this session.
- **Character avoidance = per-turn nav obstacles, mover excluded** (`SetNavObstacleEnabled`, toggled in `UTurnManager::BeginTurn`): everyone carves except the active combatant. RVO and Detour Crowd were both tried and abandoned. `AEnemyAIController` (crowd) exists but is unused.
- **A shape component attached to a non-uniformly scaled parent inherits that scale** — a sphere collapses to `radius × min(scale)`. Use `SetUsingAbsoluteScale(true)`. (Also: for a turn-based env trigger, a deterministic distance poll beats physics overlap events — see the rigged shelf's `CheckProximity`.)
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
| 27 | **Block H pt1** — retire player Basic Attack + Confuse rework (Provoke→Confuse: nearest-combatant targeting via FindNearestOtherCombatant, forced Heavy Strike, GA_BasicAttack gained a State.Confused Disadvantage check); fixed Intimidate/Embolden button tag typos. Reordered H2 after I2, deferred Intimidate pt2 + Block I until real terrain exists; locked concrete Intimidate mechanic in DESIGN_RATIONALE §6. | ✓ |
| 28 | **Block I pt1** — library room greybox (walls/door/5 aisle rows/elevation stacks), combatants repositioned, capsule-Z fix; **turn-based camera focus** (establishing wide shot → FocusOn active combatant each turn via OnTurnStarted, bIsFocusing eased pan, +150 Z offset, =/− zoom keys); committed the long-pending BP_ character rename. | ◐ Block I in progress |
| 29 | **Block I pt2** — Interact framework: `AInteractableActor` two-stage proximity trap (Arm→enemy-enter Spring→AoE crush hits everyone incl. Nameless), distance-to-body detection, `TryInteract` on player, Interact reuses modal `ArmedAbilityTag` via `Action.Interact` sentinel (key 4 + WBP button), `BP_RiggedBookshelf` in TestLevel. Fixed stale-armed-tag-between-turns bug; overlap-events + distance-to-body fixes for the crush. Turn-start intermission (enemies wait for camera before acting). | ✓ |
| 30 | **Enemy movement** — AttackRange gate + capped pathing + arrival callback: `PerformAITurn`→`EngageTarget` (in range → strike, else `MoveTowardTarget`), AIController `MoveToLocation` to a `PointAlongPath` point clamped to `MoveRange`, `OnMoveCompleted` re-checks range → strike/end. Fixes: stop inside AttackRange (acceptance-radius undershoot), bind-once+unbind (double-attack guard), MoveTo return-value handling. Enemy BPs set AutoPossessAI. Unblocks the shelf enemy-enter spring. | ✓ |
| 31 | **Character avoidance + trap trigger** — per-turn nav obstacles (every character carves the navmesh except the active mover, via `SetNavObstacleEnabled` in `BeginTurn`; Runtime Generation=Dynamic; dest projected to navmesh) so movers path around each other crisply; abandoned RVO/Crowd. Rigged-shelf walk-in trigger switched from unreliable physics overlap to a 0.15s `CheckProximity` poll. Long session — the nav-obstacle "needs a full restart to apply" gotcha cost hours. | ✓ |
| 32 | **Juice pass — rigged shelf detonation**: telegraph sequence (freeze enemy → "!!" beat → crush), C++ camera shake on the tactical rig (`TriggerShake`, PlayerCameraManager shakes don't work on it), topple Timeline with a base-hinge `Pivot` component + `OnConstruction` WYSIWYG lift, directional (front/back) blast rectangle. `BlueprintImplementableEvent` `OnDetonated`/`OnTelegraph` = C++-decides-when, BP-decides-look. | ✓ |
| 33 | **Block H pt2** — Intimidate reworked to AoE fear/displacement (roll → fling away from Nameless via ECC_Pawn capsule sweep → impact + mutual damage → herd into rigged shelves); Provoke→Confuse rename (tag + redirect + button + Embolden hidden); Confuse now 1 turn (Infinite GE stripped in EndTurnNow); characters `FaceActor` their target before attacking. | ✓ |
