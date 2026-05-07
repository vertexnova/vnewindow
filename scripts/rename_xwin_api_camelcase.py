#!/usr/bin/env python3
"""One-off: rename vne::xwin public API methods to camelCase per CODING_GUIDELINES.md."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DIRS = [
    ROOT / "include" / "vertexnova" / "xwin",
    ROOT / "src" / "vertexnova" / "xwin",
    ROOT / "tests",
    ROOT / "examples",
]

# Longest keys first (whole-identifier replacement via word boundaries).
PAIRS: list[tuple[str, str]] = [
    ("GetPrimaryMonitorIndex", "getPrimaryMonitorIndex"),
    ("GetFramebufferHeight", "getFramebufferHeight"),
    ("GetFramebufferWidth", "getFramebufferWidth"),
    ("IsFrameRateLimitEnabled", "isFrameRateLimitEnabled"),
    ("SetFrameRateLimitEnabled", "setFrameRateLimitEnabled"),
    ("CalculateSmoothedDeltaTime", "calculateSmoothedDeltaTime"),
    ("GetAverageFrameRate", "getAverageFrameRate"),
    ("GetWindowAPICapabilities", "getWindowAPICapabilities"),
    ("GetSupportedWindowAPIs", "getSupportedWindowAPIs"),
    ("IsWindowAPISupported", "isWindowAPISupported"),
    ("GetBestWindowAPIForPlatform", "getBestWindowAPIForPlatform"),
    ("CreateWindowManager", "createWindowManager"),
    ("DestroyAllWindows", "destroyAllWindows"),
    ("SetEventCallback", "setEventCallback"),
    ("GetPrimaryWindow", "getPrimaryWindow"),
    ("GetFocusedWindow", "getFocusedWindow"),
    ("SetPrimaryWindow", "setPrimaryWindow"),
    ("GetWindowCount", "getWindowCount"),
    ("GetWindows", "getWindows"),
    ("RemoveWindow", "removeWindow"),
    ("IsFeatureSupported", "isFeatureSupported"),
    ("GetPlatformInfo", "getPlatformInfo"),
    ("GetWindowAPIInfo", "getWindowAPIInfo"),
    ("GetClipboardText", "getClipboardText"),
    ("SetClipboardText", "setClipboardText"),
    ("SetWindowLimits", "setWindowLimits"),
    ("SetWindowMode", "setWindowMode"),
    ("GetWindowMode", "getWindowMode"),
    ("SetWindowIcon", "setWindowIcon"),
    ("GetNativeHandle", "getNativeHandle"),
    ("GetNativeWindow", "getNativeWindow"),
    ("GetMonitorCount", "getMonitorCount"),
    ("GetMonitorInfo", "getMonitorInfo"),
    ("IsVSyncEnabled", "isVSyncEnabled"),
    ("SetSmoothingEnabled", "setSmoothingEnabled"),
    ("IsSmoothingEnabled", "isSmoothingEnabled"),
    ("SetSleepPacingEnabled", "setSleepPacingEnabled"),
    ("IsSleepPacingEnabled", "isSleepPacingEnabled"),
    ("GetCurrentTime", "getCurrentTime"),
    ("GetPlatformTime", "getPlatformTime"),
    ("GetWindowAPI", "getWindowAPI"),
    ("GetProperties", "getProperties"),
    ("SetProperties", "setProperties"),
    ("ShouldCloseAll", "shouldCloseAll"),
    ("GetDeltaTimeMs", "getDeltaTimeMs"),
    ("GetTargetFrameRate", "getTargetFrameRate"),
    ("SetTargetFrameRate", "setTargetFrameRate"),
    ("GetMinDeltaTime", "getMinDeltaTime"),
    ("GetMaxDeltaTime", "getMaxDeltaTime"),
    ("SetMaxDeltaTime", "setMaxDeltaTime"),
    ("GetFrameCount", "getFrameCount"),
    ("GetElapsedTime", "getElapsedTime"),
    ("GetFrameRate", "getFrameRate"),
    ("GetDeltaTime", "getDeltaTime"),
    ("ClearLastError", "clearLastError"),
    ("GetLastError", "getLastError"),
    ("GetBuildInfo", "getBuildInfo"),
    ("IsInitialized", "isInitialized"),
    ("ShouldRender", "shouldRender"),
    ("ShouldClose", "shouldClose"),
    ("GetDPIScale", "getDpiScale"),
    ("IsFullscreen", "isFullscreen"),
    ("SetFullscreen", "setFullscreen"),
    ("IsTransparent", "isTransparent"),
    ("SetTransparent", "setTransparent"),
    ("OpenWindow", "openWindow"),
    ("PollEvents", "pollEvents"),
    ("ProcessEvents", "processEvents"),
    ("SwapBuffers", "swapBuffers"),
    ("FocusWindow", "focusWindow"),
    ("GetPosition", "getPosition"),
    ("SetPosition", "setPosition"),
    ("SetMonitor", "setMonitor"),
    ("GetMonitor", "getMonitor"),
    ("SetCursor", "setCursor"),
    ("SetTitle", "setTitle"),
    ("SetVSync", "setVSync"),
    ("GetHeight", "getHeight"),
    ("GetWidth", "getWidth"),
    ("IsAvailable", "isAvailable"),
    ("GetVersion", "getVersion"),
    ("SleepRemainder", "sleepRemainder"),
    ("ClampDeltaTime", "clampDeltaTime"),
    ("Initialize", "initialize"),
    ("Shutdown", "shutdown"),
    ("Minimize", "minimize"),
    ("Maximize", "maximize"),
    ("Restore", "restore"),
    ("Resize", "resize"),
    ("IsOpen", "isOpen"),
    ("Sleep", "sleep"),
    ("Update", "update"),
    ("Reset", "reset"),
    ("Close", "close"),
]

EXTRA_SUBSTS = [
    # Do not use word-boundary "Create" — would break CreateWindowExW, etc.
    ("IWindow::Create", "IWindow::create"),
    ("static std::unique_ptr<IWindow> Create(", "static std::unique_ptr<IWindow> create("),
    ("std::unique_ptr<IWindow> IWindow::Create(", "std::unique_ptr<IWindow> IWindow::create("),
]


def process_file(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    orig = text
    for old, new in PAIRS:
        text = re.sub(rf"\b{re.escape(old)}\b", new, text)
    for old, new in EXTRA_SUBSTS:
        text = text.replace(old, new)
    if text != orig:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def main() -> int:
    changed = 0
    for d in DIRS:
        if not d.is_dir():
            continue
        for path in d.rglob("*"):
            if path.suffix not in {".h", ".cpp", ".mm"}:
                continue
            if process_file(path):
                changed += 1
                print(path.relative_to(ROOT))
    print(f"Modified {changed} files.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
