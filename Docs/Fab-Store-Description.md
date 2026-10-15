# Fab Store Description — InterfaceGuard

## Headline

**InterfaceGuard — The Interface Call That Skipped The Blueprint Override**

*Find every Blueprint override of a C++ interface function that a direct call would silently
skip — before your players do.*

## One-paragraph pitch

`IInterface::Execute_FunctionName(Object, Args)` — the wrapper UnrealHeaderTool generates for
every `BlueprintNativeEvent` interface function — is the *only* call path that respects a
Blueprint override, because it is the only one that routes through `UObject::ProcessEvent`. A
direct C++ call (casting to the interface pointer, or calling `..._Implementation()` directly)
resolves through the native vtable instead, which a Blueprint-only subclass never rewrites. The
result compiles clean, warns about nothing, and silently runs the wrong implementation — one of
the most repeatedly reported Unreal interface mistakes on the Epic Developer Community Forums,
still surfacing in the UE 5.8 timeframe. InterfaceGuard finds the gap with the one fact that
actually decides it (`UFunction::Script.Num() > 0`), scans a running world or your entire project's
Blueprint assets for classes with an override, reports them by name with a plain-English reason,
and ships the one generic call path that never skips them. The included demo map proves it live:
two pedestals, one plugin class, the same two call paths — red stays red on the pedestal with no
override, and turns green only through the correct path on the pedestal that has one.

## Feature bullets

- **Finds the exact classes a direct C++ call would get wrong** — scans a live world
  (`InterfaceGuard.Scan` / `InterfaceGuard.Dump` console commands) or, in the editor, every
  Blueprint asset in the project via the Asset Registry — loaded or not.
- **One reflection fact, zero guessing** — the detector (`HasBlueprintOverride`) reads
  `UFunction::Script.Num()`, the same bit the engine itself uses to decide whether `ProcessEvent`
  has anything to run. No heuristics, no false positives on purely native classes.
- **Ships the fix, not just the warning** — `CallInterfaceFunctionSafely` is a generic
  `ProcessEvent`-based dispatch for parameterless `void` interface functions, demonstrating the
  exact mechanism a real `Execute_` wrapper uses.
- **Shows the mistake too, deliberately** — `CallInterfaceFunctionDirectly_WRONG_WAY` is included,
  named, and documented as the antipattern, so the failure is demonstrated rather than described.
- **Works outside the editor** — the Runtime module's detector and call-path statics run in a
  cooked `-game` build with no editor or Asset Registry present; only the project-wide sweep needs
  the editor.
- **On-screen live panel** — a `UCanvas`-drawn HUD shows the scan result and both call counters
  side by side, in PIE, in a packaged build, and even in an editor viewport with no PIE running.
- **Project-wide Blueprint sweep** — `Tools > Interface Guard > Scan Project For Interface
  Overrides` checks every Blueprint in `/Game` against your watched interface in one pass.
- **Configurable per project** — watched function name, scan-on-start, and panel visibility live
  in `UDeveloperSettings` (Project Settings > Plugins > InterfaceGuard), no hardcoded names.
- **Honest about its limits** — documented up front: parameterless `void` only for the generic
  safe call, reflection over loaded classes (not a C++ source-text scanner), and a note that
  Shipping cooks can strip the bytecode the check relies on.
- **Full C++ source, no third-party dependencies, automation test coverage** for the detector and
  the call mechanism itself.

## Technical specs

| | |
|---|---|
| Engine | Unreal Engine 5.8 |
| Platform | Win64 |
| Modules | 1 Runtime (`InterfaceGuard`) + 1 Editor (`InterfaceGuardEditor`) |
| Dependencies | `Core`, `CoreUObject`, `Engine`, `DeveloperSettings` (Runtime); `AssetRegistry`, `UnrealEd`, `Kismet`, `ToolMenus`, `Slate`, `SlateCore` (Editor) |
| Blueprint exposure | All detector/call functions are `BlueprintCallable`/`BlueprintPure`; findings struct is `BlueprintType` |
| Console commands | `InterfaceGuard.Scan`, `InterfaceGuard.Dump`, `InterfaceGuard.Show` |
| Demo content | 1 map, 2 demo pedestals (1 native, 1 Blueprint override), 1 UMG control panel, automation tests |
| Third-party code | None |

## Target audience

- **C++ gameplay programmers** who expose `BlueprintNativeEvent` interfaces to designers and need
  certainty that every Blueprint override is actually reachable from the call sites in their own
  code.
- **Technical leads / code reviewers** auditing a codebase (or a team's Blueprint library) for the
  Execute_-vs-direct-call mistake before it ships as a silent gameplay bug.
- **Plugin and framework authors** who expose C++ interfaces for Blueprint extension and want a
  cheap, honest way to prove to users (and themselves) which classes actually extend the intended
  behaviour.
- Teams mixing C++ and Blueprint where interface overrides are added over time by different people
  — the exact situation where a direct call written against an old, override-free class quietly
  stops being correct once someone adds a Blueprint override months later.

## Price idea

**€14** (one-time, self-serve tier). Matches the narrow-but-real-pain, single-mechanism utility
class of prior plugins in this line (TagGuard/ContextLatch/PhantomRep-style detectors); priced to
be an easy "yes" for any team that has ever hit this bug once.
