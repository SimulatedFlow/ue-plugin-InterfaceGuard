# InterfaceGuard — Documentation

**Unreal Engine 5.8 · Win64 · C++ source · one Runtime module + one Editor module**

InterfaceGuard finds every class that overrides a C++ `UINTERFACE` function in Blueprint —
exactly the classes where a direct C++ call would silently skip the override and run the base
native implementation instead. It reports them by class name and function name, and it ships the
one call path (`Execute_`-style, via `ProcessEvent`) that never skips them.

---

## 1. The problem this plugin detects

`IInterface::Execute_FunctionName(Object, Args)` — the static wrapper UnrealHeaderTool generates
for every `BlueprintNativeEvent` / `BlueprintImplementableEvent` interface function — is the
**only** call path that routes through `UObject::ProcessEvent`. `ProcessEvent` is the only
mechanism in the engine that reads a Blueprint override's compiled script bytecode.

A direct C++ call — casting to the interface pointer and calling the function, or calling
`Target->OnInteract_Implementation()` directly — resolves through the native C++ vtable instead.
A Blueprint-only subclass **never rewrites that vtable slot**: it can only replace the
`UFunction`'s script bytecode, and nothing except `ProcessEvent` ever reads that bytecode. The
direct call compiles cleanly, produces no warning, and silently runs the base C++ implementation
instead of the Blueprint override — with nothing in the log pointing at why.

InterfaceGuard's whole detector is one fact: **`UFunction::Script.Num() > 0`** means a Blueprint
graph compiled real bytecode onto that function — exactly what `ProcessEvent` (and therefore
`Execute_`) will read, and exactly what a direct call to `..._Implementation()` will never reach.

## 2. Supported platforms & engine

| | |
|---|---|
| Engine version | Unreal Engine **5.8.0** |
| Target platform | **Win64** (both modules declare `PlatformAllowList: ["Win64"]`) |
| Modules | `InterfaceGuard` (Runtime, `LoadingPhase: Default`), `InterfaceGuardEditor` (Editor, `LoadingPhase: Default`) |
| Runtime module dependencies | `Core`, `CoreUObject`, `Engine`, `DeveloperSettings` |
| Editor module dependencies | `AssetRegistry`, `UnrealEd`, `Kismet`, `ToolMenus`, `Slate`, `SlateCore` (plus the Runtime module) |
| Shipping/cooked builds | The detector (`HasBlueprintOverride`) and the two call-path statics work in a cooked `-game` build with no editor present. `UFunction::Script` bytecode can be stripped from a pure Shipping cook depending on project cook settings — treat the check as an Editor/Development tool. |
| Third-party code | None |

## 3. Installation

1. Copy the `InterfaceGuard` folder into your project's `Plugins/` directory (create the folder
   if your project has none yet).
2. Regenerate project files (or just open the `.uproject` — Unreal will prompt to rebuild missing
   modules) and build, or launch the editor directly.
3. In the editor: **Edit > Plugins**, confirm **InterfaceGuard** is enabled under *Installed >
   Gameplay*. It is enabled by default once present in `Plugins/`.
4. Restart the editor if prompted.

No project `Build.cs` changes are required to use the Blueprint-callable statics or the console
commands. If you want to call `UInterfaceGuardStatics` from your own C++ module, add
`"InterfaceGuard"` to that module's `PublicDependencyModuleNames` in its `Build.cs`.

## 4. Quick start

### 4.1 Watch your own interface

Project Settings → **Plugins > InterfaceGuard**:

* `Watched Function Name` (default `OnInteract`) — the interface function name `InterfaceGuard.Scan`
  and the editor's project-wide scan check when no function name is given explicitly on the
  command line.
* `Scan On Begin Play` (default `true`) — runs one scan automatically when a game world starts, so
  a cold screenshot of a level already shows a real finding count.
* `Show Overlay` (default `true`) — draws the on-screen status panel.

These are read by `UInterfaceGuardSettings::Get()` (a `UDeveloperSettings`, `config=Game`), so they
persist in `DefaultGame.ini` under `[/Script/InterfaceGuard.InterfaceGuardSettings]`.

### 4.2 Scan a running world

```
InterfaceGuard.Scan [FunctionName]   # scans every actor implementing the watched interface
InterfaceGuard.Dump                  # prints the summary + every finding from the last scan
InterfaceGuard.Show [0|1]            # shows or hides the on-screen panel
```

`InterfaceGuard.Scan` with no argument uses `UInterfaceGuardSettings::WatchedFunctionName`.
Findings are reported by `UInterfaceGuardSubsystem::ScanWorldForOverrides`, which iterates every
actor in the current game world that implements `IInterfaceGuardDemoInteractable` and calls
`HasBlueprintOverride` on each actor's class.

### 4.3 Scan the whole project (Editor only)

**Tools > Interface Guard > Scan Project For Interface Overrides** walks every Blueprint asset
under `/Game` through the Asset Registry — loaded or not — and applies the same
`HasBlueprintOverride` check to each one's `GeneratedClass`. Findings are logged and folded into
the active editor world's `UInterfaceGuardSubsystem` so the on-screen panel reflects the
project-wide result too. This is a read-only report: no asset is loaded for anything beyond
reading its `GeneratedClass`, and nothing is ever written back.

### 4.4 Call an interface function the correct way

```cpp
// Correct: dispatches through ProcessEvent, exactly like a UHT-generated Execute_ wrapper would.
// Reaches a Blueprint override of OnInteract if one exists.
UInterfaceGuardStatics::CallInterfaceFunctionSafely(TargetActor, TEXT("OnInteract"));
```

```cpp
// WRONG — included deliberately as the demonstrated antipattern, not hidden or disguised.
// Always resolves to the native vtable slot; never reaches a Blueprint-only override.
UInterfaceGuardStatics::CallInterfaceFunctionDirectly_WRONG_WAY(TargetActor);
```

### 4.5 Ask the question directly

```cpp
FString Reason;
if (UInterfaceGuardStatics::HasBlueprintOverride(SomeActor->GetClass(), TEXT("OnInteract"), Reason))
{
    UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *SomeActor->GetClass()->GetName(), *Reason);
    // "has a Blueprint override - a direct C++ call would skip it, call via Execute_ instead"
}
```

`HasBlueprintOverride` is a pure reflection question — no world, no network, no Asset Registry —
so it works identically in the editor and in a cooked `-game` build, and it is covered by
automation tests (`Source/InterfaceGuard/Private/Tests/InterfaceGuardTests.cpp`).

## 5. Class & API overview

### `UInterfaceGuardStatics` (`UBlueprintFunctionLibrary`, Runtime)

| Function | Signature | What it does |
|---|---|---|
| `HasBlueprintOverride` | `static bool HasBlueprintOverride(UClass* TargetClass, FName FunctionName, FString& OutReason)` | The detector. Checks `TargetClass->FindFunctionByName(FunctionName)->Script.Num() > 0`. Returns `false` with an explanatory reason for a null class or an unknown function name — that is not an error, it means there is nothing a direct call could get wrong. |
| `CallInterfaceFunctionSafely` | `static bool CallInterfaceFunctionSafely(UObject* Target, FName FunctionName)` | A generic stand-in for a UHT `Execute_` wrapper: `Target->ProcessEvent(Target->FindFunction(FunctionName), nullptr)`. Parameterless `void` functions only. Reports the call (and whether it actually hit a Blueprint override) to the target's `UInterfaceGuardSubsystem`. |
| `CallInterfaceFunctionDirectly_WRONG_WAY` | `static bool CallInterfaceFunctionDirectly_WRONG_WAY(UObject* Target)` | The antipattern, deliberately not hidden: casts to `AInterfaceGuardDemoPedestal` and calls `OnInteract_Implementation()` directly. Reports the call to the target's subsystem as a direct call. |

### `IInterfaceGuardDemoInteractable` / `UInterfaceGuardDemoInteractable` (`UINTERFACE(BlueprintType)`, Runtime)

One function: `UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void OnInteract();` with
`virtual void OnInteract_Implementation() {}` (empty on purpose — the interface's own fallback is
never meant to run; every implementing class supplies its own). This is the watched interface the
demo content implements; your own interface does not need to inherit from or reference it in any
way — `HasBlueprintOverride` works on any `UClass`/`FName` pair.

### `AInterfaceGuardDemoPedestal` (`AActor`, Runtime)

Implements `IInterfaceGuardDemoInteractable` with the base ("red") behaviour.
`SetPedestalColour(bool bGreen)` is `BlueprintCallable` so a Blueprint override of `OnInteract`
can flip the pedestal's material without reimplementing colour logic in the graph.
`IsShowingGreen()` is `BlueprintPure`. Used directly (no override) by the demo's Pedestal A, and
subclassed in Blueprint as `BP_InterfaceGuardPedestal_Overridden` for Pedestal B.

### `UInterfaceGuardSubsystem` (`UWorldSubsystem`, Runtime)

The collector. `UInterfaceGuardSubsystem::Get(WorldContextObject)` returns the instance for a
given world. Key API:

* `void ScanWorldForOverrides(FName FunctionName)` — iterates every actor in this subsystem's
  world implementing `IInterfaceGuardDemoInteractable` and replaces `Findings` with the classes
  that have a Blueprint override of `FunctionName`. An instantaneous reflection check, not an
  observation window.
* `void ReportSafeCall(bool bHitBlueprintOverride)` / `void ReportDirectCall()` — called by the
  two statics above after every call, feeding the HUD's live counters.
* `void AddExternalFinding(const FInterfaceGuardFinding& Finding)` — folds in a finding discovered
  by the editor module's project-wide Asset Registry scan, deduplicated by class + function.
* `TArray<FInterfaceGuardFinding> GetFindings() const`, `int32 GetNumSafeCalls() const`,
  `int32 GetNumDirectCalls() const`, `int32 GetNumBlueprintPathHits() const`,
  `bool HasScannedAnything() const`, `FString SummaryLine() const`, `void DumpToLog() const` — all
  `BlueprintPure`/`BlueprintCallable`.

A `UWorldSubsystem` rather than a game-instance subsystem on purpose: `ScanWorldForOverrides`
needs one specific world to walk with `TActorIterator`, and `DoesSupportWorldType` is widened so
the subsystem also exists for the Editor world — which is what lets the panel draw in an editor
viewport with no PIE running at all.

### `FInterfaceGuardFinding` (`USTRUCT(BlueprintType)`, Runtime)

```cpp
FName ClassName;
FName FunctionName;
FString Reason; // e.g. "has a Blueprint override - a direct C++ call would skip it, call via Execute_ instead"
```

One entry per class with a confirmed Blueprint override. A class with no override is never added —
it is the normal case, not a finding.

### `AInterfaceGuardHUDActor` (`AHUD`, Runtime)

Draws the status panel directly on `UCanvas` — no widget, no material, no asset, survives a
cooked Shipping build. `DrawPanelStatic(UCanvas*, const UInterfaceGuardSubsystem*, const FVector2D& Origin, float Width)`
is the shared drawing path used both by `DrawHUD()` (the PIE/`-game` path) and by the subsystem's
editor-viewport draw hook, so there is exactly one implementation of what the panel looks like.

### `UInterfaceGuardSettings` (`UDeveloperSettings`, `config=Game`, Runtime)

See §4.1. Category "Plugins", section "InterfaceGuard".

### `FInterfaceGuardEditorModule` (Editor)

`static void ScanProjectBlueprints(FName FunctionName)` — the Asset Registry sweep described in
§4.3. Registers the Tools menu entry in `StartupModule`.

### Console commands (`InterfaceGuardCommands.cpp`)

```
InterfaceGuard.Scan [FunctionName]
InterfaceGuard.Dump
InterfaceGuard.Show [0|1]
```

All three resolve the actual game world first (not whichever world `GEngine->GetWorld()` happens
to return in the editor), so they work the same from a PIE console and from a packaged `-game`
console.

## 6. Demo content

`Content/InterfaceGuard/Maps/L_InterfaceGuardDemo` places two pedestals side by side:

* **Pedestal A** — plain `AInterfaceGuardDemoPedestal`, no Blueprint override anywhere.
* **Pedestal B** — `BP_InterfaceGuardPedestal_Overridden`, a Blueprint child that overrides
  `OnInteract` to turn the pedestal green and report a safe call.

Each pedestal exposes two on-screen buttons — **"Call Direct (the mistake)"** and **"Call via
Execute_ (correct)"** — wired through `WBP_InterfaceGuardDemoPanel`. `BP_InterfaceGuardDemoGameMode`
runs the first scan automatically on `BeginPlay` (per `bScanOnBeginPlay`), so the HUD already
shows `Overrides found: 1` before any click.

## 7. Known limits (by design)

* `CallInterfaceFunctionSafely` only supports parameterless `void` interface functions. It is a
  teaching/demo stand-in, not a replacement for the UHT-generated `Execute_` wrappers once an
  interface function takes parameters or returns a value — those remain the correct call in a real
  project.
* InterfaceGuard inspects classes already loaded in memory, or — via the editor module — Blueprint
  assets the Asset Registry knows about. It is not a C++ source parser; it does not scan your
  project's `.cpp`/`.h` files for the direct-call antipattern in your own code.
* `UFunction::Script` bytecode can be stripped from a pure Shipping cook depending on project cook
  settings. Treat the detector as an Editor/Development tool, used while code is still being
  written.
* It changes and fixes nothing — no code is ever rewritten by this plugin. It names the gap;
  calling the right function instead is still yours to do.
* No network replication of findings, no return-value/parameter support in the generic safe-call
  helper, no automatic code rewriting.

## 8. Support

Issues, questions: see the `SupportURL` in the plugin's Fab listing / `.uplugin` metadata.
