# BedrockLoaderGX

A GoldHEN plugin that redirects Minecraft Bedrock's resource and behaviour pack directories to a configurable location, so packs can be added without repopulating app_tmp. You can add custom resource and behavior packs in the default directory /data/BedrockLoaderGX

## How it works

Bedrock reads packs from /temp0/user<id>/games/com.mojang/{resource_packs,behavior_packs}. The plugin hooks sceKernelOpen, sceKernelStat and fopen, and rewrites any path containing those segments to point at the plugin root instead.

Matching is done on the path segment without a trailing slash, which catches both the directory open (used for enumeration) and the individual pack files inside it. The user<id> component is never parsed - substring matching means its value does not matter.

## Layout on the console
```
/data/BedrockLoaderGX/settings/BedrockLoaderGX.ini   config (fixed location)
/data/BedrockLoaderGX/res/icon.jpg                   notification icon
/data/BedrockLoaderGX/resource_packs/                packs (moves with config)
/data/BedrockLoaderGX/behavior_packs/
```
Set PluginRoot in the ini to move the pack directories elsewhere. They are created on next launch. Existing packs are not moved for you.

The config and icon stay at the default root regardless - the config cannot live inside the directory the config chooses.

## Building

Requires WSL or Linux. Building from a Windows path does not work: CMake's configure_file fails with "Operation not permitted" on /mnt/c, because drvfs cannot represent the operations it needs.

bash:
```
sudo apt install clang lld llvm cmake ninja-build
```
## Toolchain
note: the archive is 164 MB
```
mkdir -p third_party
curl -sL https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/releases/download/v0.5.2/v0.5.2.tar.gz \
  | tar -xz OpenOrbis/PS4Toolchain
mv OpenOrbis/PS4Toolchain third_party/PS4Toolchain
rmdir OpenOrbis

export OO_PS4_TOOLCHAIN=$PWD/third_party/PS4Toolchain
 
# PRINTF=1 must match the plugin's -D__USE_PRINTF__
git clone https://github.com/GoldHEN/GoldHEN_Plugins_SDK.git
make -C GoldHEN_Plugins_SDK PRINTF=1
mkdir -p "$OO_PS4_TOOLCHAIN/include/GoldHEN"
cp GoldHEN_Plugins_SDK/libGoldHEN_Hook.a "$OO_PS4_TOOLCHAIN/lib/"
cp GoldHEN_Plugins_SDK/build/crtprx.o    "$OO_PS4_TOOLCHAIN/lib/"
cp GoldHEN_Plugins_SDK/include/*.h       "$OO_PS4_TOOLCHAIN/include/GoldHEN/"

cmake --preset Ps4 && cmake --build build
```
Verify the toolchain extracted intact before building:

bash:
```
find third_party/PS4Toolchain/lib -name '*.so' -size 0 | wc -l
```

A truncated extraction leaves zero-byte stubs. libSceLibcInternal.so should be 471344 bytes. An empty one silently breaks every libc call in the plugin and produces crashes that look like anything but a missing file.

OO_PS4_TOOLCHAIN must be exported in the shell, not just set via set(ENV{...}) in CMakeLists - Ninja's subprocesses do not inherit that, and create-fself will fail at the packaging step.

## Environment constraints

Things that are not obvious and cost a lot of debugging time:

- **Link only** `-lSceLibcInternal -lkernel -lSceSysmodule -lGoldHEN_Hook`.
  Adding `-lc` pulls in static musl, which expects startup initialisation that
  `crtprx.o` never performs. Every libc call then faults.
- **No C++ standard library.** `std::string`, `std::vector` and exceptions need
  `libc++`, which needs a real libc. C++ as a language is fine - classes,
  namespaces, templates - just not the runtime. Use `char` buffers.
- **No `errno`.** OpenOrbis headers declare `__errno_location`; SceLibcInternal
  exports `_Errno`. They do not match. Use return values.
- **No stdio in `module_start`.** `fopen`/`fwrite` fault this early. `printf`
  works (it goes to klog). Use `open`/`read`/`write` for files.
- **`crtprx.o` does not walk `.init_array`.** Global objects with constructors
  are never constructed. Plain arrays with constant initialisers are fine.
- **`HOOK32`, not `HOOK`.** x64 detours fault in stub memory on these functions.
- **`sceKernelStat_hook` calls `stat()` directly**, not `HOOK_CONTINUE`. Going
  through the trampoline faults. This matches afr.prx.
## Deploying
put plugin .prx file in /data/GoldHEN/plugins/BedrockLoaderGX.prx

then in /data/GoldHEN/plugins.ini paste the following depending on your games region or just put both of them

ini:
```
[CUSA00265] - EU game version
/data/GoldHEN/plugins/BedrockLoaderGX.prx
[CUSA00744] - US game version
/data/GoldHEN/plugins/BedrockLoaderGX.prx
```
this is just the CUSA ids of the minecraft versions i know of

## Debugging

Build with __FINAL__=0 to enable debug_printf, which logs every intercepted open. Capture with:

bash
```
nc <ps4-ip> 3232 | tee mc.log
```
Leave it at __FINAL__=1 for release logs

## Credits

Hooking approach follows `afr.prx` by SiSTR0 and jocover from the
[GoldHEN plugins repository](https://github.com/GoldHEN/GoldHEN_Plugins_Repository).        
Notification code from [OSM-Made/PS4-Notify](https://github.com/OSM-Made/PS4-Notify).    
Cmake scaffolding provided by [xeghosted/OrbisForge](https://github.com/xeghosted/OrbisForge).
