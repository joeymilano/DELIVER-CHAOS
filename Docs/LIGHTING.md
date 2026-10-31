# 暖色客厅与 Lumen

按阶段核查：0 基线 → 1 软件 Lumen → 2 局部暖光 → 3 调色 → 4 淡暖色雾。正式游戏默认阶段 4；阶段参数仅供 Development 对比，不绘制任何测试文案。

- `DefaultEngine.ini`：GI=1、Reflection=1、GenerateMeshDistanceFields=True、Lumen.HardwareRayTracing=False；Mac 使用 SM6。High 档 GI/反射（2），正式模式保持原来的 60 FPS 上限。
- 去掉无阴影的 Fill 平行光；保留主方向光。四个大范围环境点光从 95000 降为 18000，半径从 1600 缩为 850，避免继续照平整个房间。
- 台灯和南瓜灯使用暖橙色、有阴影的局部点光；可破坏台灯与南瓜灯的点光作为子组件，随物体移动，重试随物件一起重建。
- 全局后期体积：饱和度 1.15，对比度 1.1，Color Temperature 模式 5800K，泛光 0.5，暗角 0.3。
- 指数高度雾：密度 0.003，高度衰减 0.2，暖色，200cm 起雾，最大不透明度 0.12；关闭体积雾，保持薄雾与性能预算。

## 复测方法

用本机 Mac 打包版在前台运行，例如：

```bash
Builds/Mac/BadCatLab.app/Contents/MacOS/BadCatLab -windowed -ResX=1920 -ResY=1080 -NoSave -UIReview=cozy -LightingStage=4 -VisualBench=stage4_fog
```

同一固定机位，100% 渲染比例，NPC 正常巡逻。每次预热 15 秒，采样 30 秒；只统计前台游戏帧，不计后台降频。基准测试关闭 VSync 与帧率上限；正常运行仍有 60 FPS 上限。测量正常 HUD，截图时用 H 隐藏 HUD；窗口标题栏不属于 1920×1080 渲染区域。

`LightingStage` 取 0–4；`LumenQuality` 可取 0–2，分别用于退回、低成本档与 High。原始指标写入 ProjectSavedDir/VisualBench。若某阶段最低 5 秒平均帧率低于 45，先测 `-LumenQuality=1`，仍不达标再退回，并在报告保留原始不达标记录。

帧率是实测墙钟时间的帧数/秒，另记录 P99 帧耗时和最低 5 秒平均值。固定机位和短时移动检查不能代替全关卡、所有电脑的性能验收；Windows 必须独立验证。

逐步截图、原始 JSON、FPS 表和并排对比见 `Verification/lighting`。运行 `python3 Tools/lighting_report.py` 可从这些证据生成报告。

参考：Epic [Lumen 技术细节](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine)与[性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)。

本轮全部 6 个采样均超过 45 FPS 门槛，保留 High。固定机位最终阶段平均 117.83 FPS、最低 5 秒 107.63 FPS；连续操作平均 119.97 FPS、最低 5 秒 119.81 FPS。多组结果接近 120 FPS 呈现频率，不据此推算更高峰值；NPC 和物理灯具保持活动，因此不将小幅差异解释为某一步提升性能。
