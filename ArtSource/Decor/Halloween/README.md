# 万圣节贴纸

`StickerAtlas.png` 是本项目新生成的透明 RGBA 贴图，原图 1254×1254。

使用内置 image_gen / imagegen 技能生成；提示词保存在 `prompt.txt`。没有采用第三方现成贴纸或外部商标。图集四格从左上起依次为鬼魂、蜘蛛、角落蛛网、地毯图案。原始图保持透明度，在 Unreal 导入时拉伸为二次幂并生成 Mip，远景使用遮罩材质过滤细小透明噪点。

运行 `Tools/import_halloween_stickers.py` 导入 `/Game/Art/Halloween`。墙面和地毯装饰由 `LabHalloween.cpp` 布置，全部无碰撞、不投影、不参与导航；不增加新的物理物体。当前 11 张墙面贴纸、3 张台阶侧面贴纸，加 1 张地毯印花。场景主体和灯光参数不变。

本轮只修改本地工程。项目拥有者确认完成前，不推送 GitHub、不创建或合并 PR、不刷新 Mac 或完整工程包。
