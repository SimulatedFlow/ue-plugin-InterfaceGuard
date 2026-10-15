# InterfaceGuard - The Interface Call That Skipped The Blueprint Override

**Unreal Engine 5.8 · Win64 · full C++ source · no third-party code · one runtime module + one editor module**

`IInterface::Execute_FunctionName(Object, Args)` - the function UnrealHeaderTool generates for
every `BlueprintNativeEvent`/`BlueprintImplementableEvent` interface function - is the **only**
call path that routes through `UObject::ProcessEvent`, and `ProcessEvent` is the only mechanism
that reads a Blueprint override's compiled script.

A direct C++ call - casting to the interface pointer and calling the function, or calling
`TargetObject->OnInteract_Implementation()` directly - resolves through the native C++ vtable
instead. A Blueprint-only subclass **never rewrites that vtable slot**: it can only replace the
`UFunction`'s script bytecode, which only `ProcessEvent` ever reads. The direct call compiles
clean, warns about nothing, and silently runs the base C++ implementation instead of the
Blueprint override - with nothing in the log pointing at why.

This is one of the most repeatedly reported Unreal interface mistakes there is: "C++ Interface
implemented in BP is null", "Cannot implement interface function in blueprint inherited from
C++", "\[Solved\] Blueprint interface function is not called", "Impossible to Implement an
Interface" - the Epic Developer Community Forums have been collecting variations of this exact
report for years, and Epic's own "Interfaces in Unreal Engine" documentation dedicates a section
to it specifically because it keeps happening.

**InterfaceGuard never translates, imports, or rewrites a single line of your code.** It finds
the gap and shows you the mechanism.

---

## The one fact that tells the two call paths apart

`UFunction::Script.Num() > 0` means a Blueprint graph compiled real bytecode onto that function -
exactly what `ProcessEvent` (and therefore `Execute_`) reads, and exactly what a direct call to
`..._Implementation()` never reaches. `Script.Num() == 0` means the function is purely native, and
a direct call is harmless there - InterfaceGuard does not paint every direct call red, only the
ones that actually skip something.

```
InterfaceGuard FAIL | Overrides found: 1 | Safe calls (Execute_): 1 -> Blueprint hits: 1 | Direct calls (wrong way): 1

BP_InterfaceGuardPedestal_Overridden::OnInteract -> has a Blueprint override -
  a direct C++ call would skip it, call via Execute_ instead
```

## Two ways to run it, one checker

**In the running game** - console commands:

```
InterfaceGuard.Scan [FunctionName]   scan every actor implementing the watched interface
InterfaceGuard.Dump                  print every finding from the last scan
InterfaceGuard.Show [0|1]            show or hide the on-screen panel
```

**In the editor** - `Tools > Interface Guard > Scan Project For Interface Overrides` walks every
Blueprint asset under `/Game` through the Asset Registry, loaded or not, and applies the exact
same check to each one's `GeneratedClass`.

## The demo map shows the mistake and the fix, side by side

Two pedestals, one plugin-supplied C++ class, `AInterfaceGuardDemoPedestal`. Pedestal A is used
directly - no Blueprint override anywhere. Pedestal B is `BP_InterfaceGuardPedestal_Overridden`,
a Blueprint child that overrides the interface event. Each pedestal gets two buttons:

* **"Call Direct (the mistake)"** - `UInterfaceGuardStatics::CallInterfaceFunctionDirectly_WRONG_WAY`
  casts to the native class and calls `OnInteract_Implementation()` directly. On **both**
  pedestals this turns the pedestal **red** - even on Pedestal B, because the Blueprint override
  never touched the native vtable slot this call resolves through.
* **"Call via Execute_ (correct)"** - `UInterfaceGuardStatics::CallInterfaceFunctionSafely` calls
  `ProcessEvent`, exactly like the UHT-generated `Execute_` wrapper would. On Pedestal A it falls
  back correctly to the native implementation (still red - there is no override to reach). On
  Pedestal B it reaches the Blueprint graph and turns the pedestal **green**.

Same function name, same two pedestals, two different results depending only on which call path
was used - which is the entire point.

## Honest about its own limits

* `CallInterfaceFunctionSafely` is a generic stand-in for the UHT-generated `Execute_` wrappers,
  deliberately limited to a parameterless `void` function. It does not replace the real wrappers
  for an interface function with parameters or a return value - those remain the right tool.
* InterfaceGuard scans classes already loaded in memory, or - via the editor module - Blueprint
  assets the Asset Registry knows about. It is not a C++ source parser; it does not search your
  project's `.cpp` files for the direct-call pattern in your own code.
* `UFunction::Script` bytecode can be stripped from a pure Shipping cook depending on project
  settings. The check is aimed at Editor and Development use, where the question matters most:
  while the code is still being written.
* It changes nothing and fixes nothing - no code is ever rewritten by this tool. It names the
  gap; calling the right function instead is still yours to do.

## Technical

One Runtime module (the interface, the demo pedestal, the statics, the world subsystem, the HUD
panel - all usable in a cooked `-game` build with no editor or Asset Registry involved) plus one
Editor module (the Asset Registry project-wide scan and its Tools menu entry). Settings are a
`UDeveloperSettings`. Automation tests cover the detector and the call mechanism itself.

Dependencies: `Core`, `CoreUObject`, `Engine`, `DeveloperSettings` (Runtime); `AssetRegistry`,
`UnrealEd`, `Kismet`, `ToolMenus`, `Slate`, `SlateCore` (Editor).

Documentation: <https://wiki.teufel-engineering.com/en/InterfaceGuard/documentation>

---

Win64. Source included. No third-party code.

Copyright 2026 Silvan Teufel. All Rights Reserved.

<!-- SF-STORE-BLOCK:BEGIN -->
## 🛒 Source-available — see before you buy

This repository contains the **full source** of a commercial Unreal Engine plugin. It is **source-available, not open source**: read it, evaluate it, then buy a license to use it. See **the Fab Content License Agreement / Unreal Engine EULA (purchase required)**.

**Get it / Buy:**
- Fab store — all our UE5 plugins: https://www.fab.com/sellers/Silvan%20Teufel

_This plugin does not have its own Fab listing yet — the store link above is where everything we currently sell lives._

### 📬 **Free UE5 Snippet-Pack**

10 ready-to-use C++/Blueprint building blocks (subsystems, versioned saves, async nodes, editor tooling) — MIT licensed. Get it by joining the newsletter — plus a heads-up when something new ships. Double opt-in, unsubscribe in one click, no address sharing.

👉 **[Get the free pack](https://silvan.teufel-engineering.com/newsletter/plugins/?q=gh)**

_© 2026 Silvan Teufel. All rights reserved._
<!-- SF-STORE-BLOCK:END -->
