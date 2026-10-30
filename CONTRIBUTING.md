# DELIVER CHAOS 协作说明

## 首次启动

1. 克隆仓库。不要将只下载的单个源码文件当作完整工程；角色使用 Content 中的二进制资产。
2. 安装 Unreal Engine 5.8.4。Mac 当前通过验证的是 Xcode 26.1.1 与相应 Metal 工具链；Windows 尚未验证。
3. 在 macOS 中运行 `bash Tools/build_mac.sh`；引擎不在默认位置时设置 `UNREAL_ENGINE_ROOT`，Xcode 路径不同时设置 `BAD_CAT_DEVELOPER_DIR`。
4. 打开 `BadCatLab.uproject`，使用 `/Game/Maps/LivingRoom`。独立应用生成到 `Builds/Mac/BadCatLab.app`。
5. 普通代码协作不需要重新生成模型。只有修改制作源文件时才运行对应 Blender / 编辑器导入脚本。

## 日常分支流程

```bash
git switch main
git pull --ff-only
git switch -c feature/short-description
# 完成一个明确的小改动，并进行对应验证
git add <修改的文件>
git commit -m "Describe the change"
git push -u origin feature/short-description
```

在 GitHub 提交 Pull Request，说明玩家看到的变化、验证方式和仍未验证的部分。合并后其他成员重新拉取 main。公开仓库允许阅读和 fork；直接推送仍需要仓库拥有者添加写入权限。

## Unreal 资产协作

- `.uasset`、`.umap`、`.fbx`、`.glb` 是二进制文件，不适合逐行合并。同一个关卡或角色资产同一时间由一人修改；开始前在团队中确认负责人。
- `.gitattributes` 已将这些资源标记为 binary，但这不是自动锁。遇到二进制冲突，保留两个分支，交由负责人在编辑器中重新整合；不要盲目覆盖。
- 角色分别位于 `Content/Characters/Ahuai`、`Jutuan`、`Wumei`；基础道具位于 `Content/Art/Props`，派对道具位于 `Content/Art/Party`，音效位于 `Content/Audio`；代码位于 `Source/BadCatLab`。
- 模型源文件保存在 `ArtSource`，已确认参考在 `ArtSource/References/Approved`。不要重新引入被否决的圆脸比例。
- 不提交 `Binaries`、`Intermediate`、`Saved`、`DerivedDataCache`、`Builds`、个人 IDE 文件或账号密钥。
- 当前使用普通 Git 保存素材，没有 LFS 指针。未来迁移 LFS 应全队协调，不能单方面重写共享历史。

## 验证与交付边界

自动检查：Unreal Automation 中运行 `BadCatLab` 测试组。源文件为 `Source/BadCatLab/LabTests.cpp`。资产回读脚本为 `Tools/verify_art.py`。

玩法、材质、镜头或碰撞发生变化时，需要在游戏中实际检查。自动测试成功不能代替真人试玩、性能验收或独立打包。当前证据及未完成项见 `Verification/STATUS.md`。

## 来源与权利记录

素材来源、生成任务和 Credits 记录见 `ArtSource/README.md`，字体许可位于 `Content/UI/Fonts/LICENSE.txt`。仓库公开用于项目协作；尚未为整个项目授予统一的开源许可，第三方素材仍遵循各自条款。
