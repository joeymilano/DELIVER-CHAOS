"""Check local content needed for the full scene; no downloads or account access."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
required = ['Content/Audio/Gameplay/CanCanGameplay.uasset', 'Content/FabLocal/Furniture', 'Content/FabLocal/Decor', 'Content/CozyLocal/Crystal_Ball.uasset', 'Content/CozyLocal/Pumpkin_Fireplace.uasset', 'Content/FabLocal/Characters/Owner_Idle.uasset', 'Content/FabLocal/Characters/Guest_Idle.uasset']
required += ['Content/UI/Title/T_TitleMark.uasset','Content/Art/Halloween/T_StickerAtlas.uasset','Content/Art/Halloween/M_Sticker.uasset','Content/Art/M_CameraFootprint.uasset']
required += ['Content/ResidentsLocal/Owner_Idle.uasset'] + ['Content/ResidentsLocal/Owner_'+c+'_Anim.uasset' for c in ['Idle','Walk','Run','Confused']]
required += ['Content/ResidentsLocal/M_'+n+'.uasset' for n in ['Wolf3D_Body','Wolf3D_Skin','Wolf3D_Headwear','Wolf3D_Outfit_Bottom','Wolf3D_Outfit_Footwear','Wolf3D_Outfit_Top','Wolf3D_Teeth','Wolf3D_Eye']]
required += ['Content/CozyLocal/'+n+'.uasset' for n in ['Pumpkin_Fireplace','Candy_Hutch','Candy_Buffet','Ghost_Window_Garland','Pumpkin_Fairy_Lights','Broom_and_Hat_Stand','Little_Happy_Ghost','Ghost_Cookie_Tray','Candy_Jar_Set','Pumpkin_Tea_Cake','Friendly_Cat_Plush','Crystal_Ball','Plum_Teapot','Potion_Trio','Moon_Cookies','Moon_Phase_Board']]
missing = []
for name in required:
    p = root / name
    ok = p.is_file() or (p.is_dir() and any(p.glob('*.uasset')))
    print(('OK      ' if ok else 'MISSING ') + name)
    if not ok: missing.append(name)
if missing:
    print('See CONTRIBUTING.md for licensed Fab downloads and import steps.')
raise SystemExit(1 if missing else 0)
