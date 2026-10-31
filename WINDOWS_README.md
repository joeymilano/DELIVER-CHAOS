# 猫手猫脚：Windows 协作者构建交接

状态：本包是完整工程，不是 Windows 可玩包。Mac 已验证；Windows 构建和实机验收待完成。本轮按项目拥有者要求，交由 Windows 协作者打包。

## 环境

- 将整个工程解压到本机短路径，例如 `C:\Projects\DeliverChaos`，避免在 ZIP 里直接打开。
- 安装 Windows 版本 Unreal Engine **5.8.4**，并安装适配的 Visual Studio C++ 游戏开发工具和 Windows SDK。
- 当前引擎 Windows_SDK.json 指定 SDK 首选为 **10.0.22621.0**，最低 **10.0.19041.0**；MSVC 首选家族包括 **14.44.35211 及以上的 14.44**，14.40–14.43 属于该引擎禁止范围。以所安装引擎的 SDK 检查结果为准，不要只安装 VS 外壳而漏掉 C++ 工具。
- 本机 Mac 引擎尝试 Win64 打包返回 “The Win64 platform is not supported from this engine distribution.”，因此未生成 Windows EXE。

## 构建

在 PowerShell 里进入工程目录，执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tools\build_windows.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8"
```

`ExecutionPolicy Bypass` 只作用于本次进程，不更改系统策略。脚本固定引擎版本，先检查关键模型、字体和音乐是否齐全，再编译、处理 Windows 资源、打包并附带运行库。完整日志位于 `Builds\windows-build.log`。

成功后产物位于 `Builds\Windows\BadCatLab.exe`，同时在 `Builds` 下生成带时间戳的 Windows ZIP 和 SHA256。请发送**整个 ZIP**，EXE 单文件不能运行。

也可以先打开 `BadCatLab.uproject`，允许编译模块，地图使用 `/Game/Maps/LivingRoom`，在编辑器中选择 Windows 打包。

## 必须完成的 Windows 验收

1. 关闭编辑器，从打包后的 EXE 启动；断网后仍能进入首页、选猫并开局。
2. 确认中文字清晰、四只猫与主人动画正常、Fab 家具和 Nasha 茶会角均可见。
3. 确认开局 CanCan、追赶音乐、挥爪和落地声音正常；M 静音有效。
4. 试走高台路线、拍落茶壶和水晶球，确认损失只收费一次。
5. 被主人抓到有惩罚；R 重试清空账单；Tab 返回选猫；完整结束一轮后显示结算。
6. 保留 Windows 构建日志、实机截图和使用的系统/GPU 信息，验证后再标记为可玩版本。

## 资产和协作

完整本地交接包已包含 `Content/FabLocal`、`Content/CozyLocal`、角色、动作、音乐和字体，无需重新生成或导入。`.uasset` 可以在 Windows 上由同版本 UE 加载，Mac 的编译缓存和应用包不需要带过去。

本包也保留 ArtSource 制作源文件与 Tools 导入脚本，但不包含 Unreal 引擎、Git 历史和 Mac 构建缓存。版本依据见 `HANDOFF_VERSION.json`，文件完整性见 `HANDOFF_MANIFEST.json`；若有 Python，可运行 `python Tools/verify_handoff.py`。

Fab / Nasha 等素材仅用于有权参与本项目的协作者内部开发，继续遵守各自许可，不要把本交接包作为公开素材库上传 GitHub。公开仓库仍只提交许可允许公开的文件；原有 .gitignore 已包含本地素材排除规则。ElevenLabs 本地试听和用户提供音乐的发行状态见 Content/Licenses 与 ArtSource/Audio/README.md。
