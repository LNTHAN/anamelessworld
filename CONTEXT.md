# ANamelessWorld — Session Context

## 🎨 UI THEME PALETTE — locked 2026-07-27 (single source of truth)
All values UE **linear RGBA**. Font = **Cinzel** (Regular/Bold/Black already imported in `Content/Fonts/`).
**Rule that governs the whole pass — CHROME vs SIGNAL.** Chrome (panels, borders, fonts, labels, button
fills) gets restyled freely. **Signal** (cyan=move, red=threat, ally-blue/enemy-red, HP/MP fill) carries
*information* — it may be harmonized, never recoloured into ambiguity. A prettier UI that makes the
player misread whose turn it is would be a downgrade.
**Restraint rule:** dark chrome dominates, parchment carries information, **crimson is an accent only**
(titles, active turn, danger). Crimson everywhere = crimson means nothing.

| Role | RGBA | Used on |
|---|---|---|
| Crimson (accent) | `0.60, 0.18, 0.16, 1.0` | Titles, danger, emphasis (Block L locked) |
| Deep crimson | `0.38, 0.11, 0.10, 1.0` | Pressed / hover states |
| Parchment | `0.86, 0.82, 0.72, 1.0` | Primary text |
| Dim parchment | `0.58, 0.54, 0.46, 1.0` | Labels, secondary text (Lv, EXP) |
| Panel fill | `0.035, 0.030, 0.028, 0.82` | Card + menu backgrounds |
| Panel raised | `0.09, 0.07, 0.06, 0.90` | Hovered buttons |
| Border / rule | `0.32, 0.26, 0.20, 1.0` | Frames, dividers |
| Disabled | `0.35, 0.33, 0.29, 1.0` | Spent abilities, greyed buttons |
| HP fill *(signal, muted)* | `0.35, 0.50, 0.22, 1.0` | Muted moss — reads as health without shouting |
| MP fill *(signal, muted)* | `0.28, 0.32, 0.55, 1.0` | Muted indigo |
| Ally / enemy, move cyan, threat red | **unchanged** | §9 colour law — do not touch |

Ashen crimson `0.45, 0.14, 0.13, 1.0` stays reserved for the lose screen only.

### Palette amendments learned in the field (2026-07-27)
- **Allegiance is now ONE pair of values used everywhere** (strips, card HP bars, floating health bars,
  turn-indicator text): **ally `0.02, 0.02, 1.0, 1.0` / enemy `1.0, 0.02, 0.02, 1.0`**. One blue and one
  red meaning one thing — two near-identical blues is the kind of drift that reads as unfinished.
- **MP/mana = amber `0.85, 0.55, 0.15, 1.0`** (darkened slightly in practice). Chosen over green because
  **green→health is too strong a convention to spend on mana**, and amber echoes the room's candlelight.
  Bonus: HP is always blue-or-red and MP always warm, so the two bars separate by *temperature* — legible
  at a glance and for colour-blind players.
- **Perceived colour depends on AREA.** Identical values read differently at different sizes: thin strokes
  and 6px strips get anti-aliased against the background and look washed (this is why crimson text looked
  pink, and why the strip went to 10–12px); large solid bars read *more* saturated than intended. Tune per
  element rather than assuming one value works everywhere.
- **Blue is the darkest primary** (~1/3 the perceived brightness of red at equal saturation). Blue-on-dark
  or blue text on a pale floor needs its luminance lifted (e.g. `0.25, 0.45, 1.0`), not just more width.
- **HUD text over a bright 3D scene needs an outline, not just a colour** — Outline Size `2` @
  `0, 0, 0, 0.85` + Shadow `2,2` @ `0, 0, 0, 0.6`. The dark edge also makes the fill read as more saturated.
- **Empty placeholders must recede** — the portrait slot was the brightest thing on the card until it was
  set to `0.08, 0.07, 0.07, 1.0`. An empty slot should never outrank real information.
- **Crimson appears exactly once in the HUD: the button Pressed state** (`0.38, 0.11, 0.10`). Committing
  an action briefly flashes the colour of the ending screens. That's the restraint rule earning its keep.

### UMG gotchas hit this session
- **A binding always beats the Details value** — the greyed-out swatch means "computed, not authored".
  Edit the binding function, not the field. An **unconnected Return Value** on a Slate Color binding falls
  through to the parent's **Foreground Color**, which is engine-default **magenta** on `UUserWidget` — that
  was the mystery magenta, not a project setting (nothing in Config).
- **Place nodes by dragging off the pin you want to feed**, never from the palette — dragging off an Image
  reference filters to the Image overload of `Set Color and Opacity`, making type mismatches impossible.
- **Select node pins stay wildcards** (and offer no value fields) until the output is connected to
  something typed. Connect the Return Value first, *then* set the colours.
- **Untick sRGB in the colour picker** before typing palette numbers — they are linear values.
- **A ProgressBar has two colour slots that MULTIPLY**: `Color and Opacity` (whole widget) and `Fill Color
  and Opacity` (fill only). Blue fill × red tint = the muddy navy-maroon bug. Keep the widget tint white.
- **Tick `Is Variable`** on any widget the graph must reach — the step most often forgotten.

### Portrait pipeline — DONE (2026-07-27). Rendered from the models, not sourced/generated.
Decision: **render portraits from the in-game skeletal meshes** rather than sourcing or generating them.
Reasons: the player sees portrait and model *simultaneously*, so likeness must match; generated/painterly
art would clash with flat polyart (same style-match logic that chose KayKit over a realistic pack); and a
capture pipeline is a thing that presents well to a coding class. Three portraits cover the whole cast —
all three mobs share `DA_EnemyCharacter`.
- **Data path:** `UTexture2D* Portrait` on `UCRPGCharacterData` + **`ABaseCharacter::GetPortrait()`**
  (BlueprintCallable, returns nullptr if no Data Asset / no portrait). Widgets talk only to the character,
  never to the Data Asset — same as `GetHealth`/`GetUnitName`. One null-safe place to change later.
- **`L_PortraitStudio`** (`Content/UI/Portraits/`) — empty level: backdrop plane (`M_PortraitBackdrop`,
  **Unlit**, emissive ≈`0.08,0.07,0.065`, **Two Sided**), skeletal mesh at origin, warm key + cool rim
  point lights, unbound PostProcessVolume with **Manual** metering, `SceneCapture2D` → `RT_Portrait` (512²).
- **THE THREE SETTINGS THAT MUST NOT DRIFT** (they are what makes the portraits a matched *set*):
  1. **Capture Source = `Final Color (LDR) in RGB`.** The default (`SceneColor HDR … Inv Opacity in A`)
     writes **inverse opacity into alpha**, so an opaque scene captures alpha 0 → the static texture is a
     fully transparent checkerboard. Final Color also runs the post-process chain, so the exposure lock
     actually applies (it was silently doing nothing before this was changed).
  2. **Manual exposure + Exposure Compensation = `10`** (on the studio's PostProcessVolume — Lens →
     Exposure). **This exact number must not change**, or a recaptured portrait won't match the set.
     With auto-exposure, a dark hooded character
     gets brightened and a bright armoured one dimmed → five portraits at five brightnesses. Raise
     *exposure*, never the individual lights, so the key/fill/rim ratio survives.
  3. **Texture settings on every capture:** Compression **`UserInterface2D (RGBA)`**, Mip Gen
     **`NoMipmaps`**, **sRGB on**. Default compression is built for 3D surfaces and smears a face at UI size.
- **Craft notes:** FOV **30**, not the default 90 — a long lens reads as a portrait; wide FOV up close
  bulges the face. **Don't light a hood from above** — the brim blocks it; key must come in at eye level
  or below. **Never flatten the shadow side**: the lit/shadow gradient is the only thing making a
  low-poly face read as 3D. Set `Animation Mode = Use Animation Asset`, uncheck **Playing**, and scrub
  **Initial Position** to freeze a chosen frame (avoids T-pose, and is reproducible).
- **Framing rule:** match the *crop*, not the camera distance — the cast has wildly different proportions
  (TinyHero is chibi). Preserving relative scale is honest information; forcing equal head-fractions
  makes the chibi grotesque. **Judge the set as Content Browser thumbnails**, not at 512px.
- **Facing convention:** ally portraits face **right**, enemy portraits face **left**, ~20–30° off-axis —
  so the two unit cards read as confronting each other across the forecast arrow.

### UI theme pass — progress (2026-07-27)
DONE: turn indicator (Cinzel Black + outline, allegiance-coloured via its binding), unit cards (dark panel
+ allegiance edge strip + allegiance HP bar + amber MP + portrait), dialogue box (portrait + crimson
speaker name + parchment line), turn indicator/strip **merged into one Border cluster** with `M_Divider`
between, turn slots (portrait + `GetUnitName` + permanent allegiance frame + active slot grows 64→80 +
click-to-focus). **REMAINING: command-menu buttons**, then the ornament pass.
- **Dialogue portraits needed a pipeline change:** `FDialogueLine` gained `UTexture2D* SpeakerPortrait`,
  `FOnDialogueLine` went Two→**ThreeParams**, `ShowLine` gained a third arg, `UDialogueWidget` gained
  `SpeakerPortraitImage` (**`BindWidgetOptional`**, so the WBP compiles before the Image exists). Chosen
  over a speaker-name→portrait lookup because **name-matching breaks silently** — the same objection that
  rejected name-convention row lookup in I2. Empty portrait ⇒ image **Collapsed**, text gets full width.
- **New C++:** `ATacticalPlayerController::FocusOnUnit(ABaseCharacter*)` (BlueprintCallable) — the general
  form of `FocusOnPlayerForResult`; `ToggleInspect` got a `UFUNCTION`. Kept as two functions so the slot
  button calls both and either can be dropped if it feels wrong in play.
- **Turn-slot design (locked):** allegiance colour on **every** slot, always — so the whole turn order's
  composition is readable at a glance — and **size**, not colour, marks whose turn it is. Two signals on
  two channels; colour was previously doing both jobs. Size is also pre-attentive: you see "bigger"
  before you read a colour.

### Command menu + Interact parity (2026-07-27)
- **Buttons styled:** four state brushes (Normal `0.035,0.030,0.028,0.82` / Hovered `0.09,0.07,0.06,0.90` /
  Pressed `0.38,0.11,0.10` / Disabled `0.02,0.02,0.02,0.55`), **Draw As = Rounded Box** (radius 4, outline
  1px `0.32,0.26,0.20`), **Cinzel Bold 24** parchment labels. Rounded Box is a *placeholder* for the
  ornament pass's 9-slice frames — don't over-tune it. Embolden collapsed (Chapter 2 ability, nothing to
  target in Ch1; a dead button invites the wrong question in a narrated demo).
- **Disabled state is real, not hypothetical** — `FireAbility` refuses once `bActionAvailable` is false, so
  without the binding a spent action makes buttons silently inert, which reads as "broken" to an audience.
- **`IsAbilityButtonEnabled(AbilityTag)`** — one parameterised helper, three thin per-button bindings that
  call it with their own tag. Logic: `bActionAvailable AND (NOT Confirming OR ArmedAbilityTag == my tag)`
  — *"enabled if I have an action left, and either we're not confirming, or the thing being confirmed is
  me."* The armed button stays live so it reads as the active choice, not another casualty.
- **`M_ArmedBorder`** — animated ring on the armed button during Confirming. Combines `M_RangeZone`'s
  border-band mask with `M_ThreatLine`'s Time-driven sweep. **Per-axis `ThicknessX`/`ThicknessY`** because
  the ring is computed in UV space (0→1 on both axes) — one shared thickness on a 200×40 button gives 15px
  sides and 3px top/bottom. Rule: `ThicknessX = ThicknessY × (Height ÷ Width)`. Colour is **gold
  `1.0,0.85,0.1`**, not crimson: gold is unclaimed in the palette (green reads "go", blue is the ally
  colour, and the MP amber is close enough that a browner gold would read as mana-related).
  Visibility bound per button — bindings can't take parameters, so this is the one place duplication is
  correct: the differing tag literal *is* the point.
- **UI-domain materials output to `Final Color`, not Emissive.** Preview quirks that cost time: the
  material editor's preview **only updates on Apply** (the canvas is always live, the preview is not); a
  transparent centre shows the *scene behind* the preview plane, so "black" ≠ "broken"; and the square
  250² preview makes a per-axis ring look wrong — judge it on the real widget in PIE.
- **Interact now has full parity with the status abilities** (was: one-click, instant, no staging).
  `PendingInteractable` (its own slot — `PendingTarget` is an `ABaseCharacter`), staged on click, resolved
  in `ConfirmPendingAction` via `TryInteract`. **The Confirming check must sit ABOVE the Interact block in
  `OnMoveClicked`**, or Interact's early `return` swallows the confirming click and re-stages forever.
  `ArmAbility` and `ResetTargeting` both clear *both* pending slots.
- **`AInteractableActor::SetTargetAura(bool)`** → `Mesh->SetOverlayMaterial`, driven from
  `UpdateTargetAuras` (gated on `bPlayerTurn`, radius = `InteractRange` 200). Reuses **`MI_TargetAura`**
  (red) so "red = you can act on this" covers enemies *and* objects with one colour.
  Editor-side requirements: tick **Used with Static Meshes** on `M_Aura` — `SetOverlayMaterial` fails
  *silently* on a StaticMeshComponent otherwise — and assign `AuraTargetableMaterial` on `BP_RiggedBookshelf`.
  **`InteractRange` raised 200 → 300**: distance is measured origin-to-origin, and a rigged shelf's origin
  is its base-hinge Pivot, so on a 188-wide bookcase 200 wasn't reachable even standing at its edge. The
  aura and `TryInteract` share this value deliberately — an aura promising reach the interact then refuses
  would be worse than no aura at all.

### ⚠ UMG lessons from this pass (each cost real time)
- **Alignment beats size overrides.** `Fill` means "ignore your desired size, take what's available", so it
  silently discards Width/Height Override. It was *wrong* on the Size Box's own slot (squashed the 120→172)
  and *right* on the Image inside it (needed to expand). If a widget won't respect a size, check alignment first.
- **Single-child vs multi-child panels.** Button, Border, Size Box, Scale Box take **exactly one** child;
  Vertical/Horizontal Box, Overlay, Grid, Wrap Box take many. **`Wrap With…`** inserts a container above an
  already-nested widget; **`Replace With Child`** removes a container and keeps its child (user's find —
  better than neutralising a Border with alpha 0).
- **A Canvas Panel root reports NO desired size**, so an embedded widget gets squashed to nothing. Widgets
  placed inside another layout need a self-sizing root (Button/Border/Size Box/Overlay). Canvas roots are
  for things added straight to the viewport.
- **`Set Content Color and Opacity` tints a Border's CHILDREN; `Set Brush Color` tints the Border itself.**
  The old highlight used Content Color, which was fine for text but would have tinted the new portrait yellow.
- **A Border only shows where its child isn't** — it needs **Padding** for the frame colour to be visible.
- **Colour on a widget that already shows an image is a MULTIPLIER** — the dark placeholder tint
  (`0.08…`) was multiplying the portrait to a twelfth brightness. White = "leave it alone".
- **Wrong-overload trap:** `Set Color and Opacity` exists on UserWidget/Image/Text/Border. Placing it from
  the palette defaults to the **UserWidget** version with `self` pre-filled — compiles, does nothing useful.
  **Always place nodes by dragging off the pin you want to feed.** Same for `Target = self` on cast results.
- **Refresh Node** fixes call sites left holding an old signature after a function/event input changes.
- **Text-driven sizing distorts a row.** One long label ("The Protagonist") stretched its portrait. Let the
  *container* define size and make content adapt: fixed slot width + **Text Overflow Policy = Ellipsis**.
- **Read the variable name on the pin, not just the graph shape** — the size overrides were structurally
  perfect and aimed at the outer (portrait+text) Size Box instead of the inner portrait one.

### UI ornament pass — PULLED INTO THIS BLOCK (2026-07-27, user's call)
Was a stretch goal; moved in because the game is **presented to a class** and the UI is on screen for the
whole demo. **Timeboxed 2 sessions, fixed scope: unit cards + command-menu buttons + turn-indicator
backing only.** Ship the flat version if it stalls — the flat theme already reads as finished.
Assets: **Kenney UI Pack RPG Expansion** (CC0) via **9-slice** (Brush → Draw As "Box" + margins);
**ability icons rank above borders** for return-per-minute (`game-icons.net`, CC BY → credits line).
**Order rule: colour/type/hierarchy first, ornament last** — ornament goes on top of a settled layout.
Anti-rework habits: colour as brush **tint** (never baked), leave 10–20px padding slack, don't over-tune
Rounded Box (it's a placeholder for frame art). **Trim lever if the calendar tightens = VFX count**, not
the video or slides.

## ▶ NEXT SESSION: finish the UI ornament pass, then SFX/VFX juice
**Flat UI theme pass is DONE** — turn indicator, unit cards, dialogue box, merged turn cluster, turn
slots, command-menu buttons (incl. the armed-border material and the enable bindings), plus Interact
confirm-and-aura parity. All committed and pushed.
**Ornament pass is IN PROGRESS — icons first, then frames. Timebox: ~2 sessions total.**
1. **Ability icons (started, not finished):** source 3 from `game-icons.net` (white on transparent, 256
   PNG) → `Content/UI/Icons/`, texture settings **UserInterface2D (RGBA) / NoMipmaps / sRGB**. In
   `WBP_CommandMenu`, wrap each button's TextBlock in a Horizontal Box, add an Image as the FIRST child
   in a 28×28 Size Box (slot alignment Fill), tint parchment `0.86,0.82,0.72,1.0`, ~8px right padding.
   Icons inherit the Disabled dimming for free but NOT the armed-gold label binding — bind their Color
   and Opacity to the same function if you want them to go gold too.
   **CC BY 3.0 → needs a credits line** (ending cutscene text or main menu).
2. **Frames:** Kenney UI Pack RPG Expansion (CC0) via 9-slice (Brush → Draw As "Box" + margins) on the
   unit cards, command buttons, and turn cluster ONLY. The Rounded Box brushes are placeholders for these.
3. **Then the portrait backdrop question** — deliberately deferred until frames exist, because a frame
   changes how the plate reads. Re-capture at Exposure Compensation 10 if you adjust it.
**Grab a before/after PIE screenshot pair** for the presentation slides — the session-start shot was
magenta turn text + grey buttons + blue/green cards, which makes an excellent "before".

## ▶ Previous section (kept for reference): UI theme pass
**Block M is DONE** (parts 1 + 2) — the library is de-greyboxed, dressed, banner-varied and lit.
Pick up at the **whole-game UI theme pass**: propagate the locked requiem palette from `WBP_GameResult`
(Cinzel, muted crimson `0.60,0.18,0.16`, faded parchment, `M_Divider`) into the battle UI, which does
not yet look like it belongs to the same game as its own ending screens. Current offenders visible in
PIE: **magenta "Player Turn"** indicator, the **greyish `WBP_TurnOrderStrip`** sitting right below it
(default slate grey — reads as placeholder UI, and it's in the top-centre of every frame), default-grey
`WBP_CommandMenu` buttons, blue/green `WBP_UnitCard`. Scope + cut list are in the schedule-revision
note below. The environment is already on-palette (warm candle pools, dark corners, red banners as the
only saturated colour) — so this pass matches the UI **to the room**, not the two to each other.
*(Keep a pre-pass PIE screenshot as the "before" for the presentation slides.)*

### ⚑ Schedule revision (2026-07-27) — Block N CUT, build work ends July
L landed a week early, so the calendar was rewritten (see ROADMAP "REVISION 2026-07-27"):
- **Block N (onboarding) is CUT** — the final is a **narrated live demo**; the grader is walked through
  the manipulation loop in person and never plays it, so in-game teaching has no audience. The scripted
  turn-order override on `UTurnManager` is cut with it. Returns in Phase 2 / any public build.
- **Jul 27–31 = M part 2 → UI theme pass → SFX/VFX juice pass**, in that order.
  - The **whole-game UI theme pass** was Block K's deferred item and had fallen out of the schedule
    entirely (caught 2026-07-27). It goes BEFORE juice — the UI is in every frame of the showcase video.
    Scope = *propagate* the already-locked requiem palette from `WBP_GameResult` (Cinzel, muted crimson
    `0.60,0.18,0.16`, faded parchment, `M_Divider`) into `WBP_BattleHUD` / `WBP_CommandMenu` /
    `WBP_UnitCard` / `WBP_TurnOrderStrip` / `WBP_DialogueBox`, + the deferred ~5px arrow/card alignment
    + the lose-screen button reposition. NOT a new design — the battle UI just doesn't match its own
    ending screens yet. **Status icons over units: CUT** (the yellow threat-line already reads as
    "confused", and the narrator says it aloud).
  - Juice = *sourced* audio + Niagara into hooks that already fire (`OnDetonated`, `OnTelegraph`,
    `PlayHitReact`, `TriggerShake`).
  - **Phase 1 build work targets DONE by end of July** (~7–9 sessions vs ~10 available = no slack; a
    slip to Aug 1–2 is fine, a rushed theme pass is not). Hard-stop M: dressing and lighting have no
    natural stopping point and are the only realistic way this slips.
- **Aug 1–15 = deliverables** (showcase video, docs, presentation slides) + optional balance-lite
  (Block O) + buffer. No new features.
- **Debug keys: do NOT strip yet.** Level-BP **K** (instant win) / **L** (instant lose) are how the
  ending chain and both result screens get filmed without replaying a battle per take. Strip them
  **after video capture, before the final build** — this supersedes the "strip before final build"
  note in the Block L section below.
- **Doc debt:** `LEVELBP.md` is stale — it documents L-minimal but not the full Block L ending chain
  (`Advance To Ending` → slides → WorldFinish → tail) or the K/L debug keys. Update before the docs
  deliverable, since it's the text mirror a grader-facing writeup would draw from.

## Block M — environment de-greybox — PART 1 DONE
Greybox → real library. Asset stack (locked, all free): **KayKit Dungeon Pack** (itch.io, CC0) = room
shell + banners + props; **"Stylized Library — 48 Cozy Interior Props"** (Daria Borovleva, Fab) =
bookcases/books/scrolls. Characters are flat polyart, so a realistic pack (CobraGames) was rejected as
the base — style match to the characters drove the choice. Mood worry ("KayKit is too upbeat for a dark
game") resolved by the rule **style = mesh, mood = lighting**.

### Geometry (all built on KayKit's 400-unit grid)
- **Floor** — 5×5 grid of `floor_tile_large` (400×400×15), X/Y at −800/−400/0/+400/+800, **Z = −2**
  (eyeballed flush; the pivot is not a clean base pivot). Old greybox Plane deleted.
- **Walls** — wall piece is 400×100×400. Perimeter **centered on the floor-edge line at ±1000**, not
  ±1050 (at 1050 they hang over the void). All **Z = 0**; N/S at Yaw 0, E/W at Yaw 90; 5 segments a side.
  Corners overlap deliberately (reads solid). One segment omitted near Nameless = the doorway.
- **Aisle shelves** — each old white cube (e.g. Scale `0.3, 8, 2` = 30×800×200) replaced by a row of
  flat bookcases (188×92×250). Recipe: **count = big-scale ÷ 2**, spacing **190** (touching, no gaps),
  **Z = 5**, Yaw so the 188 width follows the shelf's long axis. Rows reduced 5 → **4** (less cramped).
  Bookcases are 92 deep vs the cubes' 30, so aisles tightened — navmesh rebuilt, AI pathing verified OK.

### `BP_RiggedBookshelf` — mesh swap + 3 fixes
Single bookcase per rigged shelf (not a toppling wall). Multiple rigged shelves are now spread across
different rows; each one's **+X (red arrow) must point at its intended kill zone** (front faces the fall).
- **Mesh Yaw = 90** — puts the bookcase front on the Pivot's **X** (the fall axis). Yaw 0/180 would tip
  it sideways onto its edge. Confirmed by playtest, not by the mesh gizmo (the mesh's own green arrow
  always points out its front, so it can never "face" its own red — judge against the **Pivot's** frame).
- **C++ fix 1 — `AInteractableActor::OnConstruction`** lifted the mesh by `BoxExtent.Z` (written for the
  centre-pivot cube). With a base-pivot bookcase that shoved it ~135 up and **re-applied every
  construction**, so a hand-set Z=0 kept snapping back to 135. Now computes `BaseOffsetZ =
  (BoxExtent.Z − Origin.Z) * Scale.Z`, which grounds **either** pivot type.
- **C++ fix 2 — `GetBlastHalfExtents()`** sized the blast rectangle from the mesh's **un-rotated** bounds,
  so the Mesh Yaw 90 left the damage zone turned 90° from the visible shelf (damage landed sideways).
  Now rotates the bounds by `Mesh->GetRelativeRotation()` into the actor frame first.
- **BP fix — placement-aware topple.** The Topple wrote `Set Relative Rotation` on the **root**, so every
  shelf snapped to the same world direction regardless of placement. Now: new **`InitialRotation`**
  (Rotator) ← **Get Actor Rotation** on `Event OnDetonated` (before Timeline Play); **Lerp A** =
  InitialRotation, **Lerp B** = **Combine Rotators**(A = Make Rotator Pitch 90, B = InitialRotation).
  Combine applies the pitch **locally**, so each shelf falls along its own facing. Yaw-0 shelves resolve
  to the old (0,0,0)→(0,90,0), so nothing already-working changed.
  *(Gotcha hit en route: the `SET InitialRotation` was first wired from its own getter — a self-assign
  that captured nothing. Must be fed by **Get Actor Rotation**.)*
- **TriggerSphere** reset to Absolute Scale **1,1,1** (it had inherited the long cube's `0.3,5,2`, making
  a stretched ellipsoid). Note the sphere is only a secondary trip-wire — `CheckProximity`/`Detonate`
  both use the mesh-derived `IsInBlastZone()` rectangle.

### ⚠ Asset-pipeline lesson (cost most of the session)
**Fab's "Download only → fbx" ships meshes with NO textures.** The 48-FBX download was 4 MB and every
auto-generated material (`MF_PhongToMetalRoughness` chain) pointed at **`DefaultTexture`** → the whole
room rendered grey-green. This is NOT a lighting or tint problem — check `DiffuseColorMap`'s texture
before touching anything else. The pack's real materials live in its **UE-project** version, which was
blocked by a **5.7 vs 5.6** version mismatch (that's also why the FBX route was taken).
- **Fix:** downloaded the **Unity `.unitypackage`** (99 MB) and extracted its textures. A `.unitypackage`
  is a **gzipped tar** of GUID folders each holding `asset` + `pathname`; a plain `tar -xf` gives useless
  GUID names, so the extractor reads each `pathname` to restore real filenames. 69 textures (23 sets).
- **Unity → UE naming:** `_AlbedoTransparency` = **Base Color** (renamed to `_BaseColor` on disk);
  `_Normal` = Normal; **`_MetallicSmoothness` is IGNORED** — Unity stores *smoothness*, UE wants
  *roughness* (inverted), so plugging it in makes matte wood look like plastic. Use a **Constant 0.9
  Roughness** instead (which doubles as the planned PBR-glint fix).
- **KayKit was fine** — its single gradient atlas `dungeon_texture` imported with the meshes; its
  material just needed that texture dropped into `DiffuseColorMap`. One connection coloured all 200
  assets, because KayKit meshes' UVs point at **swatches** on one 1024² colour chart.
- **KayKit stone is genuinely grey** — no hidden colour to unlock, and all KayKit meshes share ONE
  material so walls can't be tinted separately. Colour comes from banners / candlelight / books instead.

### Lighting (base pass done, final balance pending)
Directional Light **0.5 lux** cool `0.62,0.71,1.0,1.0`; Skylight **0.15** deep blue `0.05,0.07,0.13,1.0`;
**5 point lights** warm `1.0,0.65,0.30,1.0`, ~20 cd, atten 700, **Z≈280** (flame height), each sat on a
candle mesh on a shelf top = **motivated lighting**, and each placed **beside a rigged shelf** so light
also flags the interactables. **Exponential Height Fog was tried and DELETED** — it's a distance-based
outdoor tool; in a closed 20 m room it only tinted the skybox.
- **`PostProcessVolume2` was DELETED (part 2).** The battle grade now lives on the **Camera component of
  `BP_TacticalCameraRig`** (Details → Post Process). Better home: there is exactly one camera, so a
  camera-attached grade always applies and can never be outvoted by volume priority. Currently only
  **Bloom Intensity 0.35 / Threshold 1.0** is set there. The surviving `PostProcessVolume` is Block L's
  **WorldDrain** (Blend Weight 0→1 on win) — **never put battle-grade settings on it**: it sits at Blend
  Weight 0 for the whole fight, so anything set there is invisible until the win ramp.
- Standing rule regardless of where the grade lives: **the battle world must keep colour so the win
  screen can drain it away** — that's the emotional beat. A hard desaturation was tried and rejected.

### Block M part 2 — DONE (2026-07-27)
1. **Prop materials wired** — the whole dressing set is gated by only **4** materials, not all 22:
   `SM_Book_01–04`→**M_PropsSetO**, `SM_Book_05–08`→**M_PropsSetP**, `SM_Scroll_01–07`→**M_PropsSetE**,
   `SM_Stack`→**M_PropsSetF**. (User wired all of A–P anyway.) Same 2-node swap as the bookcase:
   `_BaseColor` → Base Color, `_Normal` → Normal, **Constant 0.9 → Roughness**, ignore `_MetallicSmoothness`.
2. **Shelf dressing — one cluster built + tell-books on all 5 rigged shelves.** Rule: **dress what the
   camera sees** (lit + rigged + camera-facing), NOT all ~40 bookcases. At the ~45° tactical angle only
   the **top surface and top board** read — lower boards are occluded by the shelf in front, so dress
   tops. Jitter Yaw ±15°, Pitch 8–12° on a leaner: breaking the straight top edge is 90% of the effect.
   **More books deliberately deferred** to after UI+VFX (additive, not blocking).
3. **Tell-books attached to the rigged shelves** — Outliner **drag the book row onto the
   `BP_RiggedBookshelf` row** (drop *on* it, not between rows; world transform is preserved
   automatically — there is no "Keep World Transform" dialog on this path). Attaching to an actor binds
   to its **root component**, which on `AInteractableActor` **is the `Pivot`** — the exact component the
   Topple rotates — so the book falls with the shelf for free. **Gotcha: a Static-mobility actor cannot
   be attached to a Movable one** (Static gets baked into precomputed lighting). Set each tell-book to
   **Movable** first; leave ordinary decorative books **Static** (cheaper, better baked lighting).
   Tell-books also need **Collision = NoCollision** or they jitter on the falling shelf.
4. **Banners de-wallpapered** — ~⅓ removed, kept near the doorway/lit areas; **palette narrowed to
   red + brown** (the banners ARE this level's colour, since KayKit stone is grey and untintable — and
   red/brown pre-echoes the requiem crimson `0.60,0.18,0.16` of the ending screens); pattern variants
   mixed (multi-select actors → Details → Static Mesh dropdown swaps them all at once); Z varied ±20–30
   and a **Roll of ±2–4°** (roll, not yaw — yaw swings them off the wall plane).
5. **Lighting balanced.** Concept that drove it: the room looked washed not because it was *bright* but
   because everything sat at a *similar* brightness — so cut the ambient fill and let the candles get
   **stronger**, not weaker. Pale KayKit stone throws far more bounce than the grey greybox did, and the
   walls can't be darkened directly (one shared material), so they're darkened by receiving less light.
   `r.DynamicGlobalIlluminationMethod=1` → **Lumen**, so bounce is dynamic: no lighting build, changes
   are live, no Lightmass settings involved. Auto-exposure is already **off** project-wide
   (`r.DefaultFeature.AutoExposure=False`), so it was never the culprit. Levers: Skylight down,
   Directional down, per-light/PP **Indirect Lighting Intensity**, then **Bloom 0.35 / Threshold 1.0**
   on the camera (Threshold controls *which* pixels bloom; Intensity how hard).
6. **Cleanup** — `VaultCache/` already gone. Still optional: drop the 23 unused `_MetallicSmoothness`
   textures; splitting `Environment/Library/` into `KayKit/` + `LibraryProps/` is NOT worth it (moving
   assets breaks placed-actor refs). `PropsSetH.uasset` is missing its `M_` prefix — cosmetic.

### ⚠ Two viewport lessons (cost a full tuning round-trip)
- **Post-process on a CAMERA COMPONENT does not affect the editor viewport** — the viewport renders
  through its own camera. Bloom/grade changes look like no-ops until you hit **PIE**. Judge every
  look-change in PIE, at the actual tactical angle.
- **Press `G` (Game View) before judging anything.** It hides editor-only sprites. The "candles are
  blowing out" diagnosis was wrong — most of those white blobs were **point-light bulb icons**, not
  bloom. In PIE the higher bloom (0.35/1.0) is the better look: the halo is what makes flames read as
  living light. Also: **every PP property is ignored until its checkbox is ticked**, and Color Grading
  values are 4-channel (R/G/B/**Y**) — set **Y**, the luminance/master channel, not RGB.

### Carry-forward (not M's job)
- Lose-screen Retry/Main Menu reposition → folded into the **UI theme pass**.
- **K/L debug keys stay in until after video capture** (see the schedule-revision note at the top).

**Rigged-shelf "tell" — SOLVED, diegetic:** each rigged bookshelf has an **open book** on it (plus the
candle beside it). No UI marker needed; it reads as "someone was working here." Fairness problem closed.

## Block K → L history
Block K **DONE** (core #1–#3 + click-to-inspect optionals). Both "this week" side-tasks now resolved:
- **Mob-pacing: decided NO change** — playtest shows mobs reach Nameless in ~2–3 turns at MoveRange 300;
  kept as-is (the stale 1800-unit log was far mobs, not the demo case). Optional future nudge = *cluster*
  far mobs for denser Confuse/Intimidate targets (layout tweak, NOT MoveRange). Not blocking.
- **L-minimal (win/lose result screen): DONE** this session — see the Block L-minimal section below.
- **Block L — DONE** (win screen + lose screen + full ending cutscene sequence). Design pivoted from
  triumphant FFT-gold to a **requiem** ("Closing the Page" — the win is a *sad* world-ending, not a triumph).
  The complete cinematic loop now exists end-to-end. See the Block L section below.
- Next: **M** (environment/terrain art, de-greybox) → **N** (onboarding). See [[deadline-coding-final]].
  Carry-forward for M's UI pass: reposition the lose-screen Retry/Main Menu buttons; and the deferred
  **MVP portrait** (data-driven `UTexture2D Portrait` on `UCRPGCharacterData` + `GetPortrait()`, one
  Image pulls it — FFT/FEH use a pre-made 2D still, not a live render). Also strip the K/L debug keys.
- **Deferred to the art/polish phase:** the **MVP portrait** (FFT/FEH use a pre-made 2D still, NOT a live
  render — sidesteps all the greying/SceneCapture/stencil issues + scales to the party of 5). Plan: add
  `UTexture2D Portrait` to `UCRPGCharacterData` + `GetPortrait()`, one Image in the result screen pulls it,
  one texture per character. Build it WITH the portrait art, not before. Also strip the K/L debug keys.

## Block L — WIN result screen (requiem, "Closing the Page") — DONE
Full cinematic pass on the WIN path, built on the L-minimal loop. **Design pivot (locked):** winning ends
a *world* and is mournful, not triumphant — so NO gold/glow/fanfare; muted crimson, drained world, dust.
Sequence on win: **killing blow → 2s beat → title fades in → 1.5s hold → title disintegrates into
ember-dust (left→right) → camera glides to Nameless in the drained world → "The Hand Unseen" fades in →
"Continue on." fades in.**
- **Typography:** Cinzel, muted-crimson title `0.60,0.18,0.16`, faded subtitle/contributor/prompt (requiem
  palette); tapered **`M_Divider`** (procedural UI material — spindle mask to sharp points).
- **World desaturation:** unbound Post Process Volume (Saturation 0 + vignette), Blend Weight ramped 0→1
  over a 2s **WorldDrain** Timeline in `OnCombatEnded_Event` (win only). A **Sequence** node splits
  OnCombatEnded: Then 0 = hide UI + start drain (instant); Then 1 = Delay(2.0) → build result screen.
- **Disintegration:** **`M_TextDissolve`** (Retainer-Box effect material) — Voronoi noise vs a `Dissolve`
  scalar erodes the text; **directional** via `Lerp(U, noise, NoiseWeight=0.3)` so it sweeps left→right.
  Widgets can't use Timelines, so the widget drives `Dissolve` 0→1 in **Event Tick** (bDissolving /
  DissolveElapsed / DissolveDuration=3.5) → `TitleRetainer→Get Effect Material→Set Scalar "Dissolve"`.
  `TitleGroup` (title+divider+subtitle) is wrapped in **`TitleRetainer`** (Texture Parameter = "Texture").
- **Sequencing:** `Show Result` win branch → `PlayVictorySequence` (Delay 1.5 → start dissolve). Tick
  completion → `OnDissolveComplete`: `FocusOnPlayerForResult` (camera) → reveal ContributorName → Delay(1.0)
  → reveal ContinuePrompt. ContributorName/ContinuePrompt start **Hidden** (reserve layout space so the
  reveal doesn't reflow), flipped to Visible. ContributorName has ~180 top padding (text sits below the model).
- **In-world model framing (no SceneCapture):** `ATacticalPlayerController::FocusOnPlayerForResult()`
  (BlueprintCallable) eases the camera to `ControlledCharacter` (+150 Z) and clears the follow target;
  generalizes to any MVP via `FocusOn(MVP)`. Nameless stands hooded in the drained world (victory anim = later).
- **Debug:** Level BP keys **K** = win (GetAllActorsOfClass `AEnemyCharacter` → `Die`), **L** = lose
  (`APlayerCharacter` → `Die`). `Die()` is BlueprintCallable. **Strip before final build.**
- **Deferred:** MVP portrait (art phase — see NEXT), optional "colour-in-grey-world" (the portrait gives
  it free), hiding floating HP bars in the reveal, victory anim on Nameless.

### Lose screen — the requiem's dark twin — DONE
`Show Result` False branch: world stays COLOURED (WorldDrain is win-gated, so the contrast is free — win =
dead grey world, lose = the world lives on without you); `BackgroundDim` → 0,0,0,0.75 (darkness pressing
in, anchored Fill); TitleText → deeper ashen-crimson 0.45,0.14,0.13; "THE PAGE CLOSES / The story ends
here."; Retry / Main Menu buttons restyled dark-translucent + Cinzel. No dissolve/model (that's the win's
beat). **Bugfix (same Super bug as the enemy side):** `APlayerCharacter::SetupCombat` didn't call
`Super::SetupCombat`, so Nameless's `CachedTurnManager` was null → his death only registered at EndTurn
(lose was delayed on the player's own turn). Fixed by adding the `Super` call.

### Ending cutscene sequence — DONE
Full loop: **win → "Continue on." → 4 ending slides → WorldFinish → Ch2 intro + "To be continued…" →
MainMenuLevel.** All chained inside `WBP_GameResult` by reusing existing widgets (no new ones):
- `Advance To Ending`: Create `WBP_CutsceneScreen` (SlideImages = 4 Ch1-End textures, SlideTexts = 4
  lines) → store in `EndingCutscene` var → Bind `OnFinished` → `OnEndingSlidesFinished` → Add to Viewport.
- `OnEndingSlidesFinished`: **re-entry guard** (`bEndingTriggered` bool — Branch, first fire latches it;
  blocks the double-`OnFinished` that caused a duplicate WorldFinish to flash at the end) → Create
  `WBP_WorldFinish` → Add → Delay(5.0, = its anim length so the swap lands on black) → Remove WorldFinish →
  Create tail `WBP_CutsceneScreen` (Ch2 intro + Black slide, "To be continued…") → Bind `OnFinished` →
  `OnTailSlidesFinished` → Add to Viewport.
- `OnTailSlidesFinished`: `Open Level MainMenuLevel`.
- `WBP_WorldFinish`: UMG anim `Sequence` (WORLD 1 opacity 0→1 @0-1s, FINISHED opacity 0→1 @2-3s = the 2s
  stagger, FadeOverlay black 0→1 @3.5-5s = dim-to-black), played on Event Construct. Rule: the WorldFinish
  Delay must ≥ the anim length and end on black, so the transition is seamless.
- Cutscene transition rule (learned): the 4-slide `WBP_CutsceneScreen` is never removed and its auto-advance
  can re-fire `OnFinished` → the guard (and optional Remove-EndingCutscene) handles that.

## Block L-minimal — win/lose result screen — DONE
Complete-loop *insurance*: combat ends → result screen → back into the game. **Functional pass only**
(FFT styling + reveal anim + in-world model framing all deferred to Block L full).
- **Immediate win/lose detection (C++):** `CheckCombatOver()` previously ran ONLY inside `EndTurn`, so a
  killing blow *mid-turn* (confused mob / shelf crush / boss-fells-Nameless) didn't register until someone
  clicked End Turn. Fixed: `ABaseCharacter::Die()` now calls new `UTurnManager::NotifyCombatantDied(this)`
  (placed AFTER `bIsDead=true`, so the referee's headcount sees the fresh corpse) → shared `EndCombat()`
  helper (extracted from `EndTurn`; sets `GameOver` + resolves winner + broadcasts `OnCombatEnded`). The
  `GameOver` state makes any in-flight `EndTurn` short-circuit at its top guard, so `OnCombatEnded` fires
  exactly once. Base `SetupCombat` is no longer a no-op — it caches `CachedTurnManager` for **every**
  combatant (`AEnemyCharacter`'s override calls `Super::SetupCombat` first). Lose is now immediate too.
- **`WBP_GameResult`** (Content/UI/) — pure-BP UserWidget. `Show Result(bWon)` sets title/subtitle +
  branches the UI: **win = no buttons** (ButtonRow → Collapsed) + a "Continue on." prompt, and **LMB or
  Space advances** (widget `On Mouse Button Down` + `On Key Down` overrides, both gated on a stored
  `bPlayerWon` bool) → custom event `Advance To Ending` = **STUB** (`Open Level MainMenuLevel`; Block L
  repoints it to the ending cutscene). **Lose = Retry / Main Menu** buttons (Retry = `Open Level` on
  `Get Current Level Name`; Menu = `Open Level MainMenuLevel`). `BackgroundDim` Visibility = **Visible**
  so clicks bubble up to the mouse override.
- **Level BP `OnCombatEnded_Event`:** hides battle UI FIRST (HUD + Command Menu via their vars; the
  `WBP_InspectPanel` name-card bar via `Get All Widgets Of Class` → For Each → Set Visibility Collapsed —
  the nested forecast rides along), THEN Create `WBP_GameResult` → promote to var `Result Screen Widget` →
  `Show Result(Player Won)` → Add to Viewport → `Set Input Mode UI Only` (focus the widget) → Show Mouse Cursor.
- **Input-mode carry-over fix (C++):** `Set Input Mode UI Only` lives on the game viewport and **survives
  `Open Level`** — so Retry reloaded TestLevel still in UI-Only, and the opening dialogue wouldn't advance
  (LMB never reached the controller). Fixed: `ATacticalPlayerController::BeginPlay` now asserts
  `FInputModeGameAndUI` (DoNotLock + cursor visible) on every level start, clearing the carry-over.
- **MVP/contributor deferred:** for now the contributor is always Nameless ("The Hand Unseen"); real
  per-turn damage-attribution MVP scoring waits for **Phase 2** (needs a party to compare). Showing the
  model **in-world via camera framing (no SceneCapture)** is Block L full.

## Block K — clarity & feedback CORE (items #1–#3) — DONE
Large visual-legibility pass; each item's design was locked with the user, then built. Workflow: Claude
wrote code, user applied. New order confirmed **#1 → #2 → #3** + an **animation strand** woven in.

### Item #1 — Floating combat text + hit-react + stay-dead
- **`UFloatingCombatText : UUserWidget`** (Public/UI/) with `Init(FText, FLinearColor)` BlueprintImplementable.
  `WBP_FloatingText` (reparented) = TextBlock + **"Popup"** UMG anim (Render Transform Translation Y 0→−45
  + **Render Opacity** 1→1→0 — NOT Color-and-Opacity, so the C++ tint survives). `Init` sets Text + Color
  (Make Slate Color) + Play Animation.
- **`ABaseCharacter::ShowCombatText(FText, FLinearColor)`** spawns a transient **Screen-space WidgetComponent**
  over the head, casts to UFloatingCombatText → `Init`, self-destroys via `TWeakObjectPtr` + 1.3s timer.
  `FloatingTextClass` (TSubclassOf) set per character BP.
- **Call sites:** damage numbers in `UCRPGAttributeSet::PostGameplayEffectExecute` (threat red 0.90,0.16,0.16);
  "Miss!" (GA_BasicAttack miss branch, off-white 0.90,0.90,0.90); "Immune" (GA_Provoke resisted + GA_Intimidate
  immune-skip, cyan-white 0.60,0.90,1.0); "Resisted!" (GA_Intimidate held-ground, amber 1.0,0.65,0.15).
- **Hit-react anim:** `ABaseCharacter::PlayHitReact()` (bIsDead guard) → `SetIsHit(true)` (FProperty reflection,
  same pattern as SetIsAttacking) + `EndHitReact` timer (`HitReactDuration` 0.4s) → SetIsHit(false). Fired from
  PostGameplayEffectExecute **only if GetHealth() > 0** (survivors flinch; a lethal hit lets the death anim win).
  Each AnimBP: `bIsHit` bool + **Hit state** in the Combat State Machine (Idle↔Hit, Loop OFF). Boss (RPGHero)
  sparse — Hit state optional (SetIsHit no-ops).
- **Stay-dead fix:** looping death anim → uncheck **Loop** on the death sequence + make Death state **terminal**.
  `Die()` cleanup: **`SetNavObstacleEnabled(true)`** (corpses stay SOLID = tactical terrain — user's call) +
  `HealthBarWidget->SetVisibility(false)`.

### Item #2 — Range indicators (§9 colour: cyan=move, red=ability/threat) + target auras
- **Shared decal master `M_RangeZone`** (Deferred Decal): radial `R = Distance(TexCoord, 0.5)×2` → **circle mask**
  (Ceil path) + **border band** (Abs/BorderThickness). Params FillColor/BorderColor/BorderThickness/FillOpacity/
  BorderOpacity → 3 instances: **MI_MoveRange** (cyan/deep-blue), **MI_AbilityRange** (red/crimson), **MI_ThreatRange**
  (red/blood-red). *(Move zone later swapped OFF decals — see below — but the master + other two instances stay.)*
- **Move zone = TERRAIN-EXACT** (user overrode §9's circle default): **`MoveReachISM`** (UInstancedStaticMeshComponent,
  absolute transform) on APlayerCharacter. `ComputeMoveReachable()` grid-samples the navmesh (`Step` 50) + path-
  distance test (`FindPathToLocationSynchronously ≤ MoveRange`) → one tile (engine Plane, **M_MoveTile** cyan
  translucent unlit) per reachable cell. Recompute on show, clear on hide. Reads as **tiles/blocky** (accepted);
  smooth marching-squares outline = deferred. Gated: player turn + `bMoveAvailable` + Idle (hides after moving).
- **Ability ring (generic, party-ready):** `AbilityRangeDecal` on APlayerCharacter, radius per armed ability via
  **`GetAbilityRingRadius(Tag)`** (Confuse→ConfuseCastRange; Intimidate→`GA_Intimidate::GetIntimidateRadius()` read
  off the granted ability, no drift; Interact→InteractRange; else 0). Shown in Targeting/Confirming. `SetAbilityRingRadius`
  resizes it live.
- **Enemy threat zone:** `ThreatRangeDecal` on AEnemyCharacter (boss inherits), radius `MoveRange + AttackRange`,
  shown **on hover** — controller `UpdateHoveredEnemy()` in PlayerTick (ECC_Pawn cursor trace; **only in Idle**, so it
  doesn't clutter targeting).
- **Target auras (red/grey):** `SetTargetAura(ETargetAura None/Targetable/Immune)` on AEnemyCharacter via SkeletalMesh
  **`SetOverlayMaterial`**. Controller `UpdateTargetAuras()` (called from UpdateRangeIndicators): while a **status-ability**
  ring shows, living in-range enemies glow **RED** (affectable) / **GREY** (has `Immunity.Status` — the boss); else none.
  Materials: **M_Aura** (fresnel rim) → **MI_TargetAura** red / **MI_ImmuneAura** grey. Grey = free "he's immune" teaching.
- Controller **`UpdateRangeIndicators()`** owns move-zone + ability-ring visibility; called from ArmAbility + ResetTargeting.
  Overlay-material needs **Used with Skeletal Mesh** on M_Aura.

### Item #3 — Enemy-intent threat lines (arced, red/yellow, live)
- **Design (locked):** RED line enemy→its **ally-target**, **player turn only**, **imminent-only** (target within
  `MoveRange+AttackRange`). YELLOW line for **Confused** enemies → current-nearest, from cast **until the start of its
  own turn** (spans player+enemy turns), **always shown** (exempt from imminent rule), **SOLID if it'll strike this turn /
  FADED (0.35) if only approaching**. **No predictive preview** — lines are **live current-truth snapshots** (per-frame),
  never a promise the game can't keep (a confused unit's "nearest" only resolves at its own turn; other units move on
  initiative). Natural target = "the ally it would strike" — today `PlayerTarget`, structured so multi-ally is a one-spot change.
- **`AEnemyCharacter`:** `GetIntendedTarget()` (confused→`FindNearestOtherCombatant` else `PlayerTarget`), `IsConfused()`
  (State.Confused), `GetThreatRadius()` (MoveRange+AttackRange), `SetThreatLine(bShow,Target,Color,Opacity)` → **arced
  `USplineMeshComponent`** (absolute, engine Cylinder, ForwardAxis Z; parabolic arc `Z(t)=4H·t(1−t)`, peak ~120; endpoints
  at **+LineHeight** neck-height; `LineThickness`/`LineHeight` EditAnywhere tunables) driven by a dynamic MID.
- **Controller `UpdateThreatLines()`** (PlayerTick): applies the red/yellow rules above, calls each enemy's SetThreatLine.
- **Material `M_ThreatLine`** (Unlit/Translucent/Two-Sided): `LineColor×BaseBright` base **+** white streak
  `Power(saw,Exp)×FlowBright` (crisp moving dash); flow direction = **sign of the Time×FlowSpeed** term (−0.8 = enemy→target).
  Params driven per-frame by the MID. Tuned bold base + white flow; base-colour depth is material-tunable (Power/FlowBright/
  BaseBright) with optional deeper controller colours.

**Block K deferred/notes:** optionals now built (see next section); move-zone smooth outline = later;
threat-line/aura/tile colours are all live-tunable in materials or EditAnywhere props.

### Block K optionals — click-to-inspect — DONE
Clicking a unit now inspects it — one pass covering BOTH deferred optionals, built on a **unified
three-slot bottom HUD**: NameCard (left) / forecast arrow (center) / Inspect-or-Target card (right).
Two design forks locked with the user: **"pin over hover"** + **"move-click clears all."**
- **Pinned threat zones (FE3H):** click an enemy → its threat decal **PINS on** (survives the cursor
  leaving; several at once for planning); click it again un-pins. Controller `TSet<AEnemyCharacter*>
  PinnedEnemies` layered over the existing single-`HoveredEnemy` hover — **both drive the same
  `SetThreatRangeVisible`**, so each layer checks the other before hiding (un-pin keeps it lit if still
  hovered; hover keeps it lit if pinned). Move-click empty ground → `ClearInspection()` (drops all pins +
  card); turn-start also clears (added to `OnCombatTurnStarted`, alongside `ResetTargeting`).
- **Three-slot cards:**
  - **Name Card (left)** = the current-turn unit (ally OR enemy), always. `GetNameCardUnit()` =
    `TurnManager->GetCurrentCombatant()`. **The seamless trick: attacker == current unit**, so the left
    slot never changes between idle and forecasting — only the right swaps + the arrow fades in.
  - **Inspect/Target card (right)** = `GetTargetCardUnit()`: the forecast target if ANY forecast is live
    (reuses `GetActiveForecast`), else the clicked unit (unless it's already the current-turn unit, to
    avoid drawing the same unit twice).
  - **Forecast arrow (center)** = `WBP_AbilityForecast` **stripped to just the arrow** (its own two unit
    cards deleted — the corner cards own those now). Tick = IsValid(controller) → GetActiveForecast
    Return Value → show + fill arrow / else collapse. `Out Attacker`/`Out Target` now unused.
- **C++ (`ATacticalPlayerController`):** `ToggleInspect` / `ClearInspection` / `InspectedUnit` /
  `PinnedEnemies`; split the old single `GetInspectCardUnit` into `GetNameCardUnit` + `GetTargetCardUnit`.
  `OnMoveClicked` Idle branch traces **ECC_Pawn first** (living unit → ToggleInspect + return; corpse or
  empty ground → ClearInspection + move).
- **BP:** `WBP_InspectPanel` holds BOTH cards; one **`Sequence`** Tick drives each from its getter
  (widget stays dumb — all "which unit?" logic in C++). `WBP_InspectPanel` + `WBP_AbilityForecast` each
  spawned once in the Level BP.
- **Skin pass DONE (full-width bottom bar):** all three slots consolidated into ONE **Horizontal Box** in
  `WBP_InspectPanel` — `[NameCard (Fill) | SizeBox "ArrowBox" (fixed ~150–160w) | InspectCard (Fill)]` — so
  they pack edge-to-edge with zero gap (the H-box guarantees adjacency; free-floating canvas bands couldn't).
  `WBP_AbilityForecast` is **nested** inside ArrowBox (its standalone Level-BP spawn removed) and stripped to
  just the arrow (Scale Box User-Specified ~1.5). **Consistent card size in every state:** ArrowBox always
  reserves its width, and InspectCard's empty branch = **Hidden** (reserves its half) not Collapsed, so
  NameCard never stretches — each card is `(barW − arrowW)/2` in all three states (name-only / both / forecast).
- **InspectCard mirrored** via Flow Direction = Right-to-Left (portrait faces outward), HP/MP ProgressBars
  exempted back to LTR in the shared `WBP_UnitCard` (no-op on the left card → one asset serves both sides).
- **`WBP_UnitCard` redesign:** portrait = **full-height square** (`[Row]` is the H-box; portrait Size Box
  Width Override == CardBG Height Override, ~170, kept equal so it stays square — fixed, since the aspect-ratio
  lock collapsed it; `[Row]` padding 0 for flush-left); bigger Name/Lv/EXP/HP/MP fonts; HP/MP overlays set to
  **Fill** for taller bars.
- **`WBP_CommandMenu` repositioned** (abilities → middle-left, End Turn below them) to clear the bottom for the
  card bar. **Deferred:** ~5px arrow/card height alignment + a cohesive look = the **whole-game UI theme pass**
  (all sizes here are live dials — CardBG/portrait height, ArrowBox width, arrow scale, H-box anchor Min Y).

## Block J — BALANCE PASS — DEFERRED to Block O (2026-07-21)
Was "next"; now deferred (grader doesn't weight balance). Concrete targets kept here for when Block O runs.
All levers live in `DT_Characters` cells + a couple of EditAnywhere floats:
- **Concrete findings to tune** (from playtest):
  - **Mob walk-in pacing is too slow** — turn 1 logged all enemies "ended movement out of range",
    distances 1812/1985/640 vs MoveRange 300(Person)/400(Narrator)/600(Boss). Far mobs need 4–6 turns
    just to arrive → dead time. Fix by **tightening the layout** (mobs start closer) and/or **raising
    mob MoveRange** (300 is very low).
  - **`ConfuseCastRange` = 600** (on APlayerCharacter) — tune; lower = more positional pressure.
  - **Attack/HeavyDamage columns** were set intentionally LOW (defaults 10/20) pending this pass —
    set the real per-unit spread (mobs soft, boss hard = the clock). STR modifier adds on top.
  - Balance target (DESIGN §6): boss ~150 HP, a Heavy ~50 → ~3 indirect hits; boss ~25–50/turn vs
    Nameless 100 → ~2–4 turn budget. Make "3 hits in 3–4 turns" achievable-but-demanding; all three
    tools (Confuse / Intimidate / shelf) should feel needed.
- **Known flat-damage gap (fold into the pass):** `GA_Intimidate` collision damage + the rigged
  shelf crush still apply the OLD flat `GE_DamageInstant`/`GE_HeavyStrike` (attacks now use SetByCaller
  `GE_Damage`). Decide whether env damage should also be data-driven (cheap — same injection point) or
  stays flat. Do NOT delete GE_DamageInstant/HeavyStrike — those two call sites still use them.
- After J's balance → **Block L** (win/lose screens): the `OnCombatEnded_Event` hook already exists in
  the Level BP (currently Print "Player Won!"/"Game Over..."), see LEVELBP.md. That's the Wk2 milestone.

## Block J — status immunity — DONE (commit 510311e)
The block's only new logic. Data-driven: `InitStatsFromRow` grants `Immunity.Status` (loose tag) to
any unit whose row sets `bStatusImmune`. **Confuse blocked declaratively:** GE_Provoke gained a
**"Require Tags to Apply/Continue This Effect"** component (UE 5.6's Target Tag Requirements) with
Application Tag Requirements → Must Not Have `Immunity.Status`. **Intimidate blocked in C++:** the
displacement loop skips immune units (they still TAKE collision damage from mobs flung into them —
damage ≠ status). Verified: boss logs "resisted Confuse" / "status-immune — not displaced"; Narrator
still confusable/flingable.
- **Lesson (cost us a debug loop):** `GA_Provoke`'s old log claimed "is now Confused" unconditionally —
  `SpecHandle.IsValid()` only means "spec was built". Fixed to check the applied handle's
  `WasSuccessfullyApplied()` (false when an application tag requirement blocks). **Trust application
  results, not spec validity.** Also: `showdebug abilitysystem` misreports tags on AI-possessed pawns —
  use the GameplayDebugger (`'`) or a UE_LOG for tag ground truth.
- **UE 5.6 note:** GameplayEffects are component-based; tag gating lives in the "Require Tags to
  Apply/Continue This Effect" component ("Must Not Have Tags" = the old Ignore Tags).

## Playtest findings → next-session decisions (locked with user)
- **BG3 comparison:** our *resolution* layer (d20 + mod vs AC/DC, advantage, DEX initiative, 5e
  modifier formula) is already BG3-faithful; our *damage* layer is not (flat shared 25/50, no stats)
  → fixed by the SetByCaller damage system above.
- **Dice-roll visuals (split, BG3-style):** combat rolls NEVER get a cinematic dice moment — compact
  feedback only (Block K: floating "Miss!/Resisted!" + damage numbers; roll detail `d20: 17+3 vs AC 14`
  goes to the battle log). The cinematic 3D dice screen is reserved for **narrative action checks**,
  which don't exist yet → Phase 2. Don't build the stage before there's an actor.

### Block J design — locked with user (only ONE new logic piece)
The only genuinely NEW *logic* in J is **status immunity**: the tag + boss grants it + Confuse's
GameplayEffect gets an **Application Tag Requirement** (refuses to apply to an immunity-tag holder)
+ Intimidate's C++ displacement sweep **skips** immune units. Everything else already works —
boss pursuit (`AEnemyCharacter` already fixates on `PlayerTarget`, paths around the maze, strikes),
baiting the boss through an armed shelf (springs on *enemy*-entry; the boss is an enemy), confused-mob-
hits-boss and Intimidate-flings-mob-into-boss (both are DAMAGE, not status, so they already land;
`FindNearestOtherCombatant` already includes the boss). **So J = 1 small code task + a balance pass
(`DT_Characters`) + layout tweaks.**
- **Boss damage solution space** (the puzzle — all indirect, never a direct hit): (1) a confused mob
  strikes the boss (maneuver so the boss is the mob's *nearest* target), (2) bait the boss's pursuit
  through an armed rigged shelf, (3) Intimidate a mob INTO the boss (collision mutual damage).
- **Immunity scope (confirmed):** boss immune to BOTH Confuse AND Intimidate-displacement (can't be
  shoved onto a hazard — that would break the puzzle). Damage from flung mobs / shelf crush / confused
  hits still lands normally.
- **Mob count = 3** (chosen — livelier manipulation puzzle). Setup only: drop 3 `BP_EnemyCharacter` in
  the level + wire each into `SetupCombat` / initiative (TurnManager already handles an arbitrary roster).
- **Boss = melee-fast + relentless pursuit for Phase 1**; ranged deferred (no ranged system exists;
  terrain already supplies the "can't just walk away" tension). MoveRange boss ≥ Nameless.
- **Balance race math:** boss ~150 HP, a Heavy ~50 → ~3 indirect hits to fell him; boss ~25–50/turn vs
  Nameless's 100 → a ~2–4 turn survival budget. Tune so "3 hits in 3–4 turns" is achievable-but-demanding,
  and so all three tools feel *needed* (Confuse too slow alone, shelves too few alone, Intimidate for space).

## Block I2 — data-driven stats (DataTables) — DONE
Character balance numbers now live in a spreadsheet, read at spawn. CSV import/export **deliberately
deferred** — UE's in-editor DataTable grid is enough for 3 rows; flip CSV on later with no rework
(the struct is identical either way). Kept the numbers-vs-asset-refs split (§8): DataTable = numbers,
`UCRPGCharacterData` Data Asset = asset references (abilities, GE classes).
- **Schema:** `FCRPGCharacterRow : FTableRowBase` (Public/Data/CRPGCharacterRow.h) — DisplayName,
  MaxHealth/MaxMana, the 6 D&D scores, MoveRange, HeavyStrikeChance, **bStatusImmune** (capability
  flag, not an archetype; any unit can carry it). XP/Level deliberately omitted (Block H2's job).
- **Table:** `DT_Characters` (Content/Data/) — 3 rows authored in the editor grid: `Nameless`,
  `Narrator`, `Protagonist`.
- **Read path:** `FDataTableRowHandle CharacterRowHandle` on ABaseCharacter (EditDefaultsOnly, RowType
  meta filters the picker to our struct). New `ABaseCharacter::InitStatsFromRow()` (called first in
  BeginPlay; **legacy DefaultAttributeEffect GE is the fallback if it returns false**) sets AttributeSet
  base values via the `Init*` accessors (Max first, then current = full) + MoveRange, from the row.
  `AEnemyCharacter::BeginPlay` reads HeavyStrikeChance from the row (Data Asset fallback).
- **BP wiring:** each character BP → Class Defaults → Character Row Handle = DT_Characters + its row.
- **Verified:** floating HP bars show the table's Max values (boss 150 / enemy 80 / Nameless 100);
  Heavy-Strike log thresholds read 30 (Narrator) / 60 (Protagonist) from the table.
- **`bStatusImmune` has no consumer yet** — it rides in the data until Block J builds the behaviour.
- **Decision — auto-lookup vs. hand-wired row handle:** kept the per-BP handle (set-once for 3 chars;
  the thing you tune often — the numbers — is already frictionless). If the roster grows, the clean
  upgrade is an `FName StatRowName` on the existing Data Asset + one shared default table (option 2),
  NOT a C++ per-class key (loses the designer-editable benefit) and NOT name-convention matching (magic,
  breaks silently).

### Decision — J moved back behind I2 (revert to roadmap's authored order)
The deadline SCOPE-LOCK had pulled **J (boss + balance) to priority #2** (climax first). Reversed:
**I2 now precedes J**, matching ROADMAP's authored order. Reason: the boss's spine — **status
immunity** — is really two things: (1) *who* is immune = **data** (a `bStatusImmune` cell any unit
can carry), and (2) *what immunity does* = gameplay logic (status GE refuses to apply when the
target owns the immunity tag; Intimidate's C++ sweep skips immune units). Building I2 first lets J
read immunity from a data flag instead of a hardcoded per-class `AddLooseGameplayTag`, so the
assignment is built once, data-driven — cleaner architecture (and it reads well to the grader).
Prefer a **capability flag (`bStatusImmune`)** over an archetype label literally named "Boss" — the
flag stays honest about the mechanic (immune non-boss elites / feareable bosses stay possible). A
"Boss" archetype row can still *set* that flag among others. J itself is otherwise unchanged: boss
pursuit AI, encounter layout / mob count, balance pass. HUD arc (command-menu gating, floating HP
bars, enemy forecast) is DONE + committed (837effd).


## Walk animation + camera follow — DONE (short pass on top of I2)
Movement now animates, and the camera trails the mover. Both driven by the **same signal** — the
pawn's `GetVelocity()` magnitude ("is it walking?").
- **Walk anim (all 3 AnimBPs):** each AnimBP got a float `Speed` (EventGraph: Try Get Pawn Owner →
  Get Velocity → Vector Length → Set Speed) and a **Walk state added INSIDE the existing Combat State
  Machine** (Idle stays the hub, untouched; Idle→Walk when `Speed > 10`, Walk→Idle when `Speed < 10`).
  Used the **InPlace** walk-forward clips — travel is CharacterMovement-velocity-driven, and Root Motion
  Mode stays "from Montages Only" so a state-played clip's root motion is ignored anyway. Foot-slide
  tuned via **Max Walk Speed** per character: **Nameless 175, Boss 175, Enemy 250**. Each character BP:
  **Orient Rotation to Movement = ON**, Use Controller Rotation Yaw = OFF (so they face their path).
- **Camera follow (`ATacticalCameraPawn`):** new `FollowTarget` + `SetFollowTarget()`; the controller's
  `OnCombatTurnStarted` points it at the active combatant (alongside the existing turn-start `FocusOn`).
  Tick: while `FollowTarget->GetVelocity()` exceeds `FollowMoveThreshold` (10), it re-aims `TargetLocation`
  at them (+`FollowHeightOffset` 150) and sets `bIsFocusing = true` — **reuses the existing FocusOn easing**
  instead of a parallel follow system, so the camera trails smoothly and, when they stop, settles and
  leaves manual WASD panning free (the same courtesy FocusOn already had). Trail speed = `FocusPanSpeed` (4).
- **Note:** this is Block K (clarity/feedback) flavour pulled forward because it rode naturally on the
  movement work — not a scope change. `bStatusImmune`/Block J still next.

## Last Session (large — spanned integration, a full redesign, and Block G start)

**Battle-start cinematic wired** into TestLevel Level BP: ChapterCard (over idle field) → dialogue → BattleCommenced → combat. Made **WBP_CutsceneScreen reusable** (SlideImages/SlideTexts exposed + Instance Editable/Expose-on-Spawn; `OnFinished` event dispatcher replaced hardcoded Open Level; Branch uses `SlideImages.Length`). Fixed: dialogue box hidden until a line plays; all battle UI revealed together after BattleCommenced (deleted a stale validated-GET that dead-ended the chain).

**Repo hygiene:** scrubbed ALL Claude/AI mentions from git history + tracked files; ignore rules for CLAUDE.md/AGENTS.md/memory/.claude moved to local `.git/info/exclude`. Repo is shown to teacher — keep it clean, **no co-author trailers, no AI mentions anywhere**.

**Phase 1 redesigned → depth-first** ("World 1 / Chapter 1 to near-final quality"); Phase 2 = breadth. New blocks **G–N** in ROADMAP.md (M = environment art pass / de-greybox, N = onboarding).

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

## Tactical UX loop — Stage 1 (confirm/abort backbone) — DONE
Split the one-beat ability flow (arm → click = fire) into the two-beat Fire-Emblem loop:
**arm → stage a target → confirm**, all on `ATacticalPlayerController`. The state machine grew
from the single `ArmedAbilityTag` FName to a 3-value **`ETargetingPhase`** (Idle/Targeting/
Confirming) + a **`PendingTarget`** pointer (the staged-but-not-yet-fired target; null for self-cast).
- **`AbilityRequiresTarget(FName)`** branches on arm: targeted skills (Confuse, Interact) → Targeting;
  self-cast AoE (Intimidate) → straight to Confirming (nothing to click). Currently just
  `tag != Ability.Debuff.Intimidate` — move to data (I2) once more self-casts exist.
- **`ConfirmPendingAction()`** is the ONLY place an ability fires — the click no longer fires. Self-cast
  passes Nameless himself as the (ignored) target arg; `ResetTargeting()` returns to Idle after.
- **Mouse-only, two buttons, one rule:** in Confirming, **any LMB commits**; **RMB steps back**
  (Confirming→Targeting on a targeted skill so you re-pick immediately; else →Idle). Re-target = RMB
  then click. Space (Confirm key) also commits. **NO on-screen confirm/cancel buttons** — decided
  against them; RMB/LMB is enough.
- **LMB advances dialogue** (`APlayerCharacter::IsDialogueActive()` gate, checked FIRST in
  `OnMoveClicked`) so the game is fully mouse-playable. OnMoveClicked order: dialogue → interact →
  confirming → targeting → idle-move (dialogue must be first or the click is eaten by move and the
  line never advances).
- **Confirm key = SpaceBar, ADDED alongside** the existing `Attack`=SpaceBar (not repointed). One press
  fires both handlers safely: `OnAttackPressed` no-ops unless dialogue is active, `ConfirmPendingAction`
  no-ops unless Confirming — mutually exclusive contexts, never collide.

## Tactical UX loop — Stage 2 (forecast panel) — DONE
FFT-style forecast shown during Confirming: two mirrored unit cards + a middle "arrow" (skill name, hit%,
inflict%, damage, status pill). All Blueprint UI reading C++ state off the controller.
- **C++ data layer** (`ATacticalPlayerController`): `USTRUCT FAbilityForecast` (bValid, HitChance,
  InflictChance, Damage, bDealsDamage, StatusName, AbilityName) + `GetPendingForecast()` (const → pure BP
  node) — per-tag switch (Confuse for now; `// TODO(I2)` to move to a DataTable). Intimidate/self-cast has
  no single target so the right card collapses.
- **Card getters on `ABaseCharacter`**: added `GetMana/GetMaxMana/GetCharacterLevel/GetXP/GetUnitName`
  (mirror `GetHealth`). NB **not** `GetLevel` — collides with `AActor::GetLevel()`; used `GetCharacterLevel`.
- **`WBP_UnitCard`** (reusable, Content/UI): `Unit` (ABaseCharacter, Instance-Editable + Expose-on-Spawn)
  variable; portrait slot + name + Lv + EXP + HP/MP bars + job (blank) + status-icon slots. One `RefreshCard`
  function (guarded `IsValid(Unit)`) sets everything; called on Construct AND re-called by the panel when the
  Unit changes. Allegiance colour binds off `IsPlayerCharacter()` (blue player / red enemy). Sized ~320×96,
  80×80 square portrait. **Right card mirrored via Flow Direction Preference = Right to Left** (no duplicate asset).
- **`WBP_AbilityForecast`** (Content/UI): Canvas root (always `Not Hit-Testable Self & All Children` so LMB/RMB
  pass through AND it keeps ticking) → a `ForecastRow` (toggled Visible/Collapsed) inside a **Scale Box (User
  Specified 1.6)** for crisp scaling. Construct caches the controller (`Cast to TacticalPlayerController`); Tick
  shows/populates when `TargetingPhase == Confirming` (feeds both cards + `RefreshCard`, fills the arrow from
  `GetPendingForecast`). Spawned once via Level BP `Create Widget → Add to Viewport`.

## HUD follow-up arc — DONE (three tasks, one session)
Cleared the three HUD follow-ups surfaced during Stage 2, in dependency order.

**#4 Command menu → ally-turn-only + bound to active unit** (Blueprint-only): `WBP_CommandMenu`'s
`Setup Command Menu` now binds `PlayerCharacter→TurnManager` `OnTurnStarted`/`OnCombatEnded` (same
decoupled pattern the camera uses). `HandleTurnStarted(ActiveCombatant)` → `IsPlayerCharacter?` →
Visible + `SET Player Character = Cast ActiveCombatant` (retargets the menu to the active ally) / else
Collapsed; `HandleCombatEnded` → Collapsed. Menu starts Collapsed.
- **Bug fixed (missed first broadcast):** menu never showed when Nameless was turn 1 (bind ran AFTER
  `StartCombat`'s first `OnTurnStarted`). Fix = **catch-up**: after binding, call `HandleTurnStarted(
  GetCurrentCombatant())` once (guarded by `IsValid`) so the current turn is evaluated immediately.
  Same class of bug as the camera's `BindToTurnManager` "bind before StartCombat" warning.

**#3 Floating HP bars (FFT-style)** — chose FFT over BG3 hover (design call): persistent thin bars over
heads give the whole-board HP read the manipulation loop needs (compare several enemies at once). Card =
detailed inspection; floating bar = at-a-glance. Hover-to-inspect card deferred to Block K.
- **C++:** new `UHealthBarWidget : UUserWidget` (Public/Private **UI/**) holding `ABaseCharacter* OwnerCharacter`
  (same "BP inherits a C++ widget" pattern as `WBP_DialogueBox→UDialogueWidget`). `ABaseCharacter` gained a
  `UWidgetComponent* HealthBarWidget` (Screen space, DrawSize 150×18, Z +120, **NoCollision** so it never
  eats click traces); `BeginPlay` calls `InitWidget()` then sets `OwnerCharacter=this`.
- **BP:** `WBP_HealthBar` (reparented to `UHealthBarWidget`) — a ProgressBar; **Percent** binding = `IsValid(
  Owner)? GetHealth/GetMaxHealth : 0`; **Fill Color** binding = allegiance (`IsPlayerCharacter`) blue/red.
  Set `Widget Class = WBP_HealthBar` on the inherited `HealthBar` component in all 3 character BPs.
- **Color:** ally fill = same HUE as the card's `CardBG` navy (1A2D5A, hue ≈229°) but brightened for
  legibility (a dark navy fill reads as near-black in a thin bar). Bar's whole-widget `Color and Opacity`
  and `Fill Image → Tint` must be white (1,1,1,1) or they multiply the fill dark.
- Deleted the old fixed `WBP_BattleHUD` Player/Enemy/Boss HP bars (kept the turn indicator).

**#2 Enemy-turn forecast (FFT-style, attacker-left)** — the forecast panel now shows during an enemy's
pre-strike beat, enemy = attacker on the LEFT (same "attacker-left/target-right" rule as the player's).
- **Controller (`ATacticalPlayerController`):** refactored `GetPendingForecast` → shared
  `BuildForecast(Attacker, Target, AbilityTag)` (Confuse / Basic 25 / Heavy 50; `TODO(I2)` for data).
  Added enemy-forecast state (`ForecastAttacker/Target`, `bExternalForecastActive`, `ExternalForecast`) +
  `ShowAttackForecast`/`HideAttackForecast` + **`GetActiveForecast(out Forecast, out Attacker, out Target)`**
  — one unified read: player Confirming OR enemy pre-strike, always fills attacker/target/data.
- **Enemy (`AEnemyCharacter`):** split `AttackTarget` → `AttackTarget` (face + `ShowAttackForecast` + start
  `ForecastTimerHandle` for `ForecastDelay` 1.2s) → `ExecuteStrike` (`HideAttackForecast` + the actual
  `SendGameplayEventToActor` + `EndTurnAfterDelay`). Reaches the controller via `GetPlayerController(0)`.
- **Widget (`WBP_AbilityForecast`):** Tick rewired from 4 separate controller reads (TargetingPhase /
  ControlledCharacter / PendingTarget / GetPendingForecast) to ONE `GetActiveForecast` node — `Return Value`
  = show condition, `Out Attacker`→left card, `Out Target`→right card (Unit **and** its IsValid check),
  `Out Forecast`→arrow. Because `Out Attacker` feeds the left card, the enemy lands on the attacker side.

## HUD follow-ups surfaced during Stage 2 (each its own focused task)
1. **Command menu turn-gating (#4, do first):** `WBP_CommandMenu` is always visible + fixed → should show only
   on the player's turn and bind to the active unit (else it overlaps with multiple units). Tie to TurnManager.
2. **Floating HP bars (#3):** the fixed Player/Enemy/Boss HP bars → per-character world-space bars
   (`WidgetComponent` above each model).
3. **Enemy-turn forecast (#2, needs a design call first):** the forecast only fires on the player's Confirming
   phase; showing an FE-style preview before enemy attacks is a new hook in the enemy AI (do we want it?).
4. **Forecast native sizing / skin:** currently a Scale Box 1.6 (crisp). Real resize (fonts/dims) + the chevron
   arrow *shape* (needs a PNG brush — UMG has no polygon primitive) belong in the UI skin pass (Block K).
   Also: forecast row overlaps the End Turn / turn strip — formalise HUD element zones in that pass.

## Movement — stays committed; preview deferred to Block K
Reaffirmed the locked "no movement undo" decision (BG3/Larian: movement is a committed event). BG3's real
QoL is a **movement preview** (pathing line + move-cost + hazard/AoO warnings) + mid-stride cancel, NOT
rewind — build that in Block K once hazards/AoO exist to warn about.

## Deferred cleanup (grep-verified C++-dead, pending Blueprint Find-References check)
Post-Stage-1 tidy pass, do AFTER confirming each is Blueprint-unreferenced: `AEnemyAIController` (whole
class, both files — verify no enemy BP's AIControllerClass points at it); `UseEmbolden`/`UseIntimidate`/
`UseProvoke` on APlayerCharacter (buttons rewired to ArmAbility); the `CycleTarget`+`AddTarget`+`AllTargets`
cluster (verify the Level BP no longer writes `AllTargets` first). Also the player's dead input/cursor block
and the shelf's dormant `TriggerSphere`/`OnSphereBeginOverlap`. Do it as its own commit, separate from features.

## SCOPE LOCKED — deadline-driven (see memory [[deadline-coding-final]])
Build doubles as a **coding-school final. DEADLINE LOCKED: Aug 15 2026** (set 2026-07-16, ~30 days out;
tightened from the original end-of-Aug). **The current state is already roughly passable** — so remaining
work is high-value polish + a satisfying climax, NOT obligation.

### Locked week-by-week plan (→ Aug 15)
- **Wk1 (Jul 16–22) — Block J:** status immunity (1 code task) + 3-mob setup + layout + balance → a winnable, tuned boss fight.
- **Wk2 (Jul 23–29) — Block L finish:** win/lose screens + "World Finished" + ending slides + Ch2 teaser → **complete playable loop (the milestone — passable build reached here).**
- **Wk3 (Jul 30–Aug 5):** Block K legibility (enemy-intent indicators + floating damage numbers) + start Block N onboarding.
- **Wk4 (Aug 6–12):** finish onboarding + a **light** Block M art pass (floor material + one real shelf) + H2 only if time.
- **Aug 13–15 — buffer:** playtest / balance / bug-fix. **NO new features.**
- Critical path ≈ 11–13 sessions → comfortable margin. **If slipping, cut in this order:** H2 → full M art (keep the light version) → Tier-2 danger overlays. **The risk is scope creep in K/M, not time.**

Priority order:
1. **Tactical UX loop** (above) — makes the battle flow feel right.
2. **Boss encounter + balance** (J) — the climax that proves the manipulation combat; single highest-value deliverable.
3. **Full loop wiring** — intro → battle → **win AND lose** screens → outro (a complete experience).
4. **Flair the user wants** (the fun part): retreat + collision animation (Intimidate's smooth panic-slide + fall-over,
   replacing the teleport), **floating combat text / ability damage numbers**, ability VFX, more screenshake, UI skin/juice.
5. **I2 data architecture** (portfolio value — shows clean architecture to a grader) + turn-based status-duration system; **H2** progression as time allows. **NOTE: I2 is now *sequenced* before J** (see "Decision — J moved back behind I2" up top) so J's status-immunity flag is data-driven from the start — even though J outranks I2 on raw deliverable value. Keep I2 time-boxed so it doesn't eat J's sessions.
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
- **A `Collapsed` UMG widget does NOT tick** — so a panel can't start Collapsed and rely on its own Tick to
  un-collapse itself (catch-22). Keep the root always painted+click-through (`Not Hit-Testable Self & All
  Children` ticks) and toggle an inner container's Visible/Collapsed instead. (`Hidden` still reserves layout.)
- **Scale a widget with a Scale Box (`Stretch=User Specified`), NOT Render Transform Scale** — Render Transform
  rasterizes at 1.0 then stretches the pixels (blurry text); Scale Box applies a layout scale so fonts re-render
  crisp.
- **A `const` BlueprintCallable C++ function becomes a PURE node** (no exec pin) in Blueprint — wire it as data,
  not in the exec chain.
- **Mirror a whole widget with `Flow Direction Preference = Right to Left`** (reverses child order + text) — used
  for the forecast's right unit card, so one `WBP_UnitCard` asset serves both sides. Set an inner child back to
  Left-to-Right to exempt it (e.g. progress-bar fill).
- **`GetLevel` is taken** — `AActor::GetLevel()` returns a `ULevel*`; name character-level getters
  `GetCharacterLevel`. Same care for other engine-name collisions.
- **A C++ `USTRUCT` has no BP variable panel entry** — see its fields via a `Break <Struct>` node or a function's
  return pin; struct-layout changes need a FULL editor restart (recompile alone won't refresh it).
- **Expose-on-Spawn vars only get set via `Create Widget`** (that pin appears there) — a designer-placed widget
  leaves them null, so guard consumers (e.g. `RefreshCard` guards `IsValid(Unit)`).
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
| 34 | **Tactical UX loop — Stage 1** (confirm/abort backbone): `ETargetingPhase` (Idle/Targeting/Confirming) + `PendingTarget` on the controller; click stages, `ConfirmPendingAction` fires; any-LMB-commits / RMB-steps-back; `AbilityRequiresTarget` skips Targeting for self-cast Intimidate; LMB advances dialogue (`IsDialogueActive`); Confirm=SpaceBar added. No on-screen confirm/cancel buttons (RMB/LMB only). | ✓ |
| 35 | **Tactical UX loop — Stage 2** (forecast panel): `FAbilityForecast` struct + `GetPendingForecast` on the controller; `ABaseCharacter` card getters (`GetMana/GetMaxMana/GetCharacterLevel/GetXP/GetUnitName`); reusable `WBP_UnitCard` (RefreshCard, allegiance colour, mirrored right card via Flow Direction); `WBP_AbilityForecast` (click-through root that keeps ticking, ForecastRow toggled, Scale Box 1.6). Surfaced HUD follow-ups (command-menu turn-gating, floating HP bars, enemy forecast). | ✓ |
| 36 | **HUD follow-up arc** (3 tasks): command-menu ally-turn-gating + active-unit bind (catch-up fix for missed-first-broadcast); floating FFT HP bars (`UHealthBarWidget` + `UWidgetComponent` on ABaseCharacter, allegiance colour, deleted fixed HUD bars); enemy-turn forecast (shared `BuildForecast`/`GetActiveForecast`, `AttackTarget`→forecast beat→`ExecuteStrike`, widget rewired to one unified read, attacker-left). | ✓ |
| 37 | **Block I2 — data-driven stats**: `FCRPGCharacterRow : FTableRowBase` schema + `DT_Characters` DataTable (3 rows in the editor grid, CSV deferred); `FDataTableRowHandle` on ABaseCharacter → `InitStatsFromRow()` sets AttributeSet base values (`Init*` accessors) + MoveRange from the row, legacy stat-init GE as fallback; `AEnemyCharacter` HeavyStrikeChance from the row. `bStatusImmune` staged in data for Block J. Reordered J behind I2 so immunity is data-driven from the start. | ✓ |
| 38 | **Walk animation + camera follow** (short pass on top of I2): per-AnimBP `Speed` var + Walk state inside the existing Combat State Machine (InPlace clips, Speed>10/<10, Orient-to-Movement, Max Walk Speed 175/175/250 for Nameless/Boss/Enemy); camera trails the active combatant while walking via `SetFollowTarget` + a velocity-gated Tick re-aim that reuses FocusOn's easing. | ✓ |
| 39 | **Block J — status immunity** (the block's only new logic): `Immunity.Status` tag granted data-driven from `bStatusImmune`; GE_Provoke "Require Tags to Apply/Continue" component blocks Confuse; Intimidate displacement skips immune units (collision damage still lands). Fixed GA_Provoke's lying success log (`WasSuccessfullyApplied`). Also: Aug 15 deadline + week plan locked; blocks M (env art) / N (onboarding) added; STORY.md skeleton created (local-only); playtest decisions recorded (SetByCaller damage + honest Hit% next, dice visuals split combat-compact/K vs cinematic/Phase-2). | ✓ |
| 40 | **Block J — encounter + honest numbers** (long session): `UTurnManager::RegisterWorldCombatants` auto-gathers every ABaseCharacter (one Level BP node replaces 8; `SetupCombat` now a base virtual hook); 3-mob encounter (Person1/Person2 rows, EditAnywhere row handle); tactical camera spring-arm collision OFF (fixed unit-jams-establishing-shot). Data-driven damage: DT base + STR via SetByCaller into new GE_Damage; `CalculateAttackDamage`/`CalculateHitChance` shared by ability + forecast (no drift); Confuse gains `ConfuseCastRange` 600. Wrote **LEVELBP.md** (Level BP text mirror). Recorded Phase-2 data-driven ability system (deferred w/ rationale) + enriched Block K (range indicators, tiered enemy-intent). | ✓ |
| 42 | **Block K optionals — click-to-inspect** (unified 3-slot bottom HUD): click a unit to inspect — pinned enemy threat zones (`TSet<AEnemyCharacter*> PinnedEnemies` layered over the hover, mutual keep-lit guard, move-click/turn-start clear); NameCard (left = current-turn unit) / forecast arrow (center) / Inspect-Target card (right); split `GetInspectCardUnit`→`GetNameCardUnit`+`GetTargetCardUnit`, `ToggleInspect`/`ClearInspection`, ECC_Pawn click branch; `WBP_InspectPanel` two-card `Sequence` Tick (widget dumb, logic in C++); `WBP_AbilityForecast` nested into a full-width Horizontal-Box bar (`[NameCard\|ArrowBox\|InspectCard]`, cards Fill, consistent size via Hidden-reserved slots) + stripped to just the arrow; `WBP_UnitCard` redesign (full-height square portrait + bigger fonts/bars, RTL-mirrored right card); `WBP_CommandMenu` repositioned to clear the bottom. Design forks locked: pin-over-hover + move-click-clears-all; final 5px alignment + cohesive skin deferred to a whole-game UI theme pass. | ✓ |
| 41 | **Block K core (items #1–#3)** (very long session — full detail in the Block K section above): #1 floating combat text (`UFloatingCombatText`/WBP + `ShowCombatText`; damage/Miss/Immune/Resisted) + hit-react anim (`PlayHitReact`/bIsHit, survivor-only) + stay-dead (loop-off + solid corpses + hide HP bar); #2 range indicators — **terrain-exact move zone** (`MoveReachISM` navmesh flood + tiles), generic ability ring (`GetAbilityRingRadius`), enemy threat decal on hover, **red/grey target auras** (SkeletalMesh overlay), shared `M_RangeZone` master + instances; #3 **arced enemy-intent lines** (`USplineMeshComponent`, `GetIntendedTarget`/`UpdateThreatLines`, red natural / yellow confused solid-or-faded, live per-frame no-prediction, white-flow `M_ThreatLine`). Optionals (click-toggle threat + character cards) deferred to next. | ✓ |
