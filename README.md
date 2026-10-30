# DELIVER CHAOS

**中文游戏名：合成大坏猫**

中文 Unreal 5.8.4 连锁恶作剧游戏。2026-10-30 18:46:16（北京时间）首次提交参赛代码；此前准备材料在相邻 Preproduction 目录。

## 协作获取

```bash
git clone https://github.com/joeymilano/DELIVER-CHAOS.git
cd DELIVER-CHAOS
```

仓库包含 Unreal 工程、C++ 源码、已导入的角色/场景资产、生成源文件、制作脚本和验证记录。编译缓存与本地 Mac 应用包不进 Git；克隆后需用兼容的 Unreal 环境构建。当前实测平台为 macOS，Windows 尚未验证。

开始修改前阅读 [协作说明](CONTRIBUTING.md)。当前没有 Git LFS 依赖；首次克隆包含完整二进制素材，会比纯代码仓库大。

## 启动

本机已构建版本可打开 `Builds/Mac/BadCatLab.app`，无需启动编辑器。此目录不随 GitHub 克隆下载；协作者先安装下方开发环境并运行构建脚本。最新验证范围与未完成项见 `Verification/STATUS.md`。

## 选猫与合成

三只可选择角色采用用户确认的大眼、大耳、收窄脸型和低多边形毛簇比例，均有独立的 3D 模型、材质与待机、走、跑动作。

| 主角 | 特点 |
|---|---|
| 阿坏 · 奶牛猫 | 均衡移动与挥爪 |
| 橘团 · 橘猫 | 跳得更高，挥爪更有力 |
| 乌梅 · 黑猫 | 移动更快，便于调整攻击位置 |

先选择主猫，在普通客厅玩 45 秒，再选择另一只猫作为搭档。配方固定公开，保留主猫外观；4 秒合成过场后进入 60 秒变异轮。

| 组合（顺序不限） | 合成结果 | 规则 |
|---|---|---|
| 阿坏 + 橘团 | 弹力惯犯 | 小物件高弹性，最多 4 次额外弹跳；弹簧配件 |
| 阿坏 + 乌梅 | 共振惯犯 | 首次击中向附近最多 3 件小物体传递冲量；音箱配件 |
| 橘团 + 乌梅 | 蹦迪惯犯 | 较弱弹跳 + 最多 2 件共振；组合配件 |

大家具不参加共振。每条链最多发动一次共振。正常轮与变异轮重建互动道具；重试清空计分与连锁。

## 操作

- 选猫页：1 / 2 / 3 或点击卡片；Enter 确认。
- WASD 移动，鼠标观察，Space 跳跃。
- 左键轻点挥爪，按住 0.25 秒以上再松开扑击，每轮 3 次；0.8 秒蓄满。
- R 重试；已完成合成后直接重试当前变异轮。
- Tab 返回选猫；T 完整重玩普通轮；Esc 暂停/恢复。
- H 隐藏 HUD；C 固定拍摄镜头。

## 当前完成度

这是可继续打磨的参赛开发版。角色模型与三种合成规则已制作；攻击、扑击和无辜姿态仍使用简化动作表现，尚未完成专门的猫动画、音效、完整美术精修、性能和真人试玩验收。不能将生成模型、自动测试通过视为商业发行质量证明。

## 开发与素材

`Tools/build_mac.sh` 编译、生成基础关卡并打包。脚本固定使用 Xcode 26.1.1，避免其他项目切换全局工具链影响 Unreal。可用 `BAD_CAT_DEVELOPER_DIR` 和 `UNREAL_ENGINE_ROOT` 明确覆盖路径。

- `Tools/prepare_cat.py` / `prepare_rig.py`：Blender 处理源模型、导出静态和骨架 FBX；`BAD_CAT_NAME` 选择 Ahuai / Jutuan / Wumei。
- `Tools/import_cat_models.py`：Unreal 编辑器导入，须用完整编辑器的 `-ExecutePythonScript`，而非无 Slate 的 FBX 导入命令模式。
- `Tools/build_room_art.py` / `import_room_art.py`：原创道具与合成配件制作、导入。
- `ArtSource/README.md`：Hyper3D 费用、来源、许可状态与修改记录。
- `Verification/fusion-tests/index.json`：计分、选猫配方、共振与重试自动检查。

中文字体为引擎附带 DroidSansFallback，许可位于 `Content/UI/Fonts/LICENSE.txt`。Python 只用于制作，不参与打包后玩法；游戏运行不调用生成平台，不包含平台密钥。
