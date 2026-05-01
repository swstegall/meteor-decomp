# Known libraries — what to skip

Every game ships with statically-linked middleware. Decompiling it is
wasted effort: the source is either available or off-the-shelf, and
matching it function-by-function adds nothing to our understanding of
how the game works. This document lists the middleware we *expect* to
find in the binary so a contributor knows to look it up rather than
puzzle through it.

## Miles Sound System (RAD Game Tools)

**Section signature**: `MSSMIXER` (custom PE section). Confirmed
present in `ffxivgame.exe`.

**What it is**: Miles is the audio middleware used by virtually every
PC game of the era — mixing, DSP effects, MP3/Bink-Audio decoding.
It's loaded both as a DLL (`mss32.dll`) and as a statically-linked
mixer core (the `MSSMIXER` section).

**How to identify**: function names like `AIL_*` (AudioInternalLibrary)
appear in the import table or as call targets if static-linked. RTTI
class names like `MILES_DRIVER`, `MILES_SAMPLE`, `HMDIDRIVER`.

**Skip rule**: any function in or called from `MSSMIXER` is
out-of-scope. Mark as `status: middleware-miles` in
`config/ffxivgame.yaml`.

## DirectX 9 + DirectShow

**Imports**: `d3d9.dll`, `d3dx9_*.dll`, `dinput8.dll`, `dsound.dll`,
`dxguid.lib` (linked as static).

**RTTI seeds**: `IDirect3DDevice9`, `IDirect3DTexture9`, etc.

**Skip rule**: thunks into the DX9 API are obvious (`call eax` after
loading a vtable slot from `d3d9_device_ptr`). The wrapper functions
the game wraps around them are *interesting* (those are the rendering
pipeline) but the DX9 calls themselves are documented elsewhere.

## Windows Sockets 2 (`ws2_32.dll`)

The workspace already replaces `ws2_32.dll` with a Seventh Umbral /
garlemald-client shim that redirects connections to
`localhost:54994`. The binary's call sites are:

- `WSAStartup` / `WSACleanup` — once at startup.
- `socket` / `connect` / `send` / `recv` / `closesocket` — the entire
  network thread runs through these.
- `gethostbyname` — for the hardcoded retail server.

**Decomp value**: HIGH for the network thread (`net/` module), LOW
for the WSA bookkeeping. Decompile the thread function;
short-circuit the WSA bookkeeping wrappers as one-liners.

## MSVC 2005 C/C++ runtime (statically linked)

`/MT` build → `libcmt.lib`, `libcpmt.lib`, `libcpmtd.lib` (debug
unlikely in retail).

**Identifying functions**: `__security_init_cookie`,
`__security_check_cookie`, `__report_gsfailure`, `_initterm`,
`__crtGetEnvironmentStringsA`, `_RTC_CheckEsp`, `__chkstk`,
`__alloca_probe`, `__except_handler4`, etc.

**Skip rule**: every CRT entry point is named in the SP1 source
(`%VS90COMNTOOLS%\..\..\VC\crt\src\` for VS 2008; equivalent for
VS 2005). Mark `status: middleware-crt`. We don't decompile these;
we match them by linking against the same CRT.

## STL (MSVC 2005 dinkumware)

Mangled symbols like `?_Insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@`.
Visible all over the place once any class touches `std::string` or
`std::vector`.

**Skip rule**: we link the same STL. The vector/string member-function
addresses live in the binary because of MSVC's `/Gy` per-function
linking, but we don't need to "decompile" them — they're in the
SP1 sources, byte-for-byte.

## CryptoAPI shims

`advapi32.dll` (CryptAcquireContext / CryptHashData / CryptDestroyHash)
imports usually mean a CRC32 / MD5 / SHA1 thin wrapper. The game's
network handshake uses Blowfish (per Project Meteor reverse
engineering), implemented in-tree, not via CryptoAPI. CryptoAPI
imports are therefore likely just for the launcher's update-package
verification; check ffxivupdater.exe.

## zlib

The game's `.dat` archive decompression is zlib. Tell-tale: the
`adler32` polynomial constants (0x12345678-style) and the `inflate`
state-machine shape. zlib 1.2.3 was current in 2010-2012; it's
public-domain.

**Decomp value**: ZERO. Skip and link zlib.

## Lua

The Project Meteor Discord notes (`project_meteor_discord_context.md`)
mention `unluac` being used to decompile Lua bytecode shipped in the
game's data files. The Lua VM itself is statically linked into
`ffxivgame.exe`. Functions named `lua_*`, `luaL_*`, `luaB_*` will
appear in the symbol seed once we run RTTI + string-grep.

**Decomp value**: LOW for the VM (it's stock Lua 5.0 or 5.1 — link
upstream). HIGH for the *bindings* (the `XIV_*` wrappers around
`lua_pushinteger` etc. — those are how the game's Lua scripts
manipulate Actor / Director / Quest state).

## ATL / MFC fragments

Possibly. ATL macros have telltale `__declspec(allocate(".CRT$..."))`
constructors. MFC unlikely (game UIs don't need MFC).

---

When in doubt, the test is: **does Project Meteor / Seventh Umbral
reference this function name?** If yes, decompile. If no and the name
matches a CRT/STL/middleware fingerprint, skip.
