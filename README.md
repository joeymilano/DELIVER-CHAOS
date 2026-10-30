# BAD CAT LAB — 坏猫不讲物理

2026-10-30 开赛后创建的 Unreal 5.8.4 原型工程。第一提交为真实本机提交时间，无倒签。此前概念与文档在相邻 Preproduction 目录，未作为参赛代码提交。

## 启动原型

打开 `Builds/Mac/BadCatLab.app`，无需启动编辑器。当前本机已运行并停在暂停界面，Esc 恢复。验证细节见 `Verification/STATUS.md`。

## 当前范围

原生 C++ 实现：分件潮玩猫、第三人称移动与跳跃、轻挥爪/蓄力扑击、18 件交互道具、45 秒正常轮 → 8 秒固定融合提示 → 60 秒弹性轮 → 结算、唯一计分与独立因果链、重试与暂停、HUD 隐藏和固定镜头。

这是首个玩法原型，不是最终美术与比赛完成品。当前无外部生成资产、无音效；融合为文字演示，猫动作使用部件运动。三条事故路线的成功率、真实手感、性能、20 轮稳定性与五人试玩尚待验证。

## 操作

- WASD 移动，鼠标观察，Space 跳跃。
- 左键轻点挥爪，按住 0.25 秒以上后松开扑击，每轮 3 次；0.8 秒蓄满。
- R 重试；完成第一次正常轮后，重试默认进入弹弹轮。
- T 完整重玩教学；Esc 暂停/恢复。
- H 隐藏 HUD；C 固定拍摄镜头。

## 开发

使用 Xcode 26.1.1 和 Metal Toolchain 17B54；引擎 `/Users/Shared/Epic Games/UE_5.8`。

1. 用引擎 Build.sh 编译 BadCatLabEditor Mac Development。
2. 用 UnrealEditor-Cmd 执行 Tools/bootstrap.py，生成材质和 LivingRoom 关卡（可重复执行）。
3. 打开 BadCatLab.uproject，在 LivingRoom 中 Play。
4. Automation 测试名：BadCatLab.Scoring.UniqueAndCausal。

来源：当前只有 Unreal Engine 内置基础形状与自编程序；没有使用来源不明素材。Python 仅用于编辑器生成资产，不参与打包后玩法。
