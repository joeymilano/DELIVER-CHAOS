# 开始页艺术字

用户提供参考图 `Delivery Chaos_ Cat Mischief.png`，要求用其中的艺术字替换开始页标题。`UserReference.png` 保留用户原图，`TitleMark.png` 为基于该参考提取制作的透明版本，由内置 imagegen 编辑工具生成。

1438 × 1093，RGBA，保留透明通道。Unreal 资源为 `/Game/UI/Title/T_TitleMark`，使用 UI 纹理组、无 Mip、RGBA8 压缩设置及常驻纹理。导入脚本为 `Tools/import_title_mark.py`。

开始界面按原比例绘制在 1280 × 720 安全区的 `(24, 74)`，宽 470；键盘按钮和右侧实时猫场景沿用现有实现。烹制配置显式包含 `/Game/UI/Title`，供最终统一打包使用。本轮没有更新 Mac 游戏包和协作者工程包。

## 最终编辑提示词

Edit target: the supplied game start-screen screenshot. Extract ONLY the existing Chinese brush lettering logo on the LEFT reading exactly 猫手猫脚, preserving the exact source design, letter shapes, brush texture, proportions, orange and charcoal ink, yellow-orange cat eyes inside the first 猫, cream negative-space paw inside the second 猫, exclamation marks, whisker scratches, fishbone, scattered ink flecks, and curved orange/black tail flourish. Treat the artwork as a precise cutout, not a redesign or new lettering. Remove the cream background behind and within the logo, preserving interior negative space as transparent except the actual yellow-orange eyes. Exclude all English header text, buttons, cat model, furniture, and screenshot borders. Return a tightly framed, high-resolution RGBA transparent-background game UI logo, landscape approximately 1.3:1, with a small clean transparent margin on all sides. Do not redraw or change the Chinese characters; all four must match the screenshot. This is a production texture for an Unreal Engine title screen.

设置 `transparent_background=true`，参考为用户原图。生成结果经过实机显示检查，证据见 `Verification/brush-title`。
