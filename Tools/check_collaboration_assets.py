"""Check local content needed for the full scene; no downloads or account access."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
required = ['Content/Audio/Gameplay/CanCanGameplay.uasset', 'Content/FabLocal/Furniture', 'Content/FabLocal/Decor', 'Content/CozyLocal/Crystal_Ball.uasset', 'Content/CozyLocal/Pumpkin_Fireplace.uasset', 'Content/FabLocal/Characters/Owner_Idle.uasset', 'Content/FabLocal/Characters/Guest_Idle.uasset']
missing = []
for name in required:
    p = root / name
    ok = p.is_file() or (p.is_dir() and any(p.glob('*.uasset')))
    print(('OK      ' if ok else 'MISSING ') + name)
    if not ok: missing.append(name)
if missing:
    print('See CONTRIBUTING.md for licensed Fab downloads and import steps.')
raise SystemExit(1 if missing else 0)
