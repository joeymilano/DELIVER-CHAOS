# 茶会与占卜角接入

PR #2 已由仓库合并。本轮从 main 新建 feature/cozy-party-integration 接入素材。

声音：CanCan 仍是拆家主曲。Pixabay Menu 用于首页/选猫；Chase 用于主人追赶和最后 20 秒，脱离追赶保持 3 秒再恢复；Win 用于有损失的结算。Swipe、Landing、Glass、Fusion、Key 分别对应攻击、落地、破坏、合成和最后 5 秒开门；Cash 用于零损失结算；MeowLoud 裁为 1.8 秒，用于音波技能。普通 MeowNice 来源编号不一致，未使用；CatEating 暂留候选。M 可关闭 BGM。

Nasha 模型：7 件静态装饰（南瓜壁炉、糖果柜、糖果餐台、女巫帽架、幽灵挂饰、南瓜灯串、月相板），9 件物理摆件（水晶球、茶壶、月亮饼干、糖果罐、幽灵饼干、南瓜茶点、幽灵、黑猫玩偶、魔药瓶）。放置于房间北侧，保留原高台及中央通道；小物件使用简化碰撞，沿用一次性计价和重试销毁重建。

两套模型许可允许用于成品游戏，禁止独立分享模型。源模型与导入资产保留 ArtSource/CozyLocal、Content/CozyLocal，公开仓库不含模型本身。协作者需持有相同素材包：默认从 ~/Downloads 读取（COZY_DOWNLOADS 可覆盖），用 Blender 执行 Tools/prepare_cozy.py，再在完整 Unreal 编辑器执行 Tools/import_cozy.py。模型颜色来自原始 GLB 材质，尺寸统一为厘米。

音频制作：运行 python3 Tools/prepare_pixabay.py，再由编辑器执行 Tools/import_pixabay.py。来源及处理记录随包保存于 Content/Licenses。没有增加游戏内制作署名。
