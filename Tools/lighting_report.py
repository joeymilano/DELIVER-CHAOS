# -*- coding: utf-8 -*-
"""Create a local comparison report from actual per-stage native screenshots and frame samples."""
from pathlib import Path
import json,html,csv
root=Path(__file__).resolve().parents[1];out=root/'Verification/lighting'
steps=[('stage0_before','step0-before.png','基线'),('stage1_lumen','step1-lumen.png','① 软件 Lumen'),('stage2_lights','step2-lights.png','② 暖色灯光'),('stage3_grade','step3-grade.png','③ 后期调色'),('stage4_fog','step4-fog.png','④ 淡暖色雾')]
rows=[]
for label,picture,title in steps:
 p=out/(label+'.json')
 if not p.exists():continue
 row=json.loads(p.read_text());row.update(title=title,picture=picture);rows.append(row)
with (out/'fps.csv').open('w',newline='',encoding='utf-8-sig') as f:
 w=csv.writer(f);w.writerow(['阶段','分辨率','平均FPS','最低5秒FPS','P99帧耗时ms','采样秒','Lumen档位'])
 for r in rows:w.writerow([r['title'],f"{r['width']}×{r['height']}",r['average_fps'],r['minimum_5s_fps'],r['p99_frame_ms'],r['sample_seconds'],r['lumen_quality']])
body=''
for i,r in enumerate(rows):
 before=rows[max(0,i-1)]
 body+=f'''<section>
<h2>{html.escape(r['title'])}</h2>
<p>平均 <b>{r['average_fps']:.2f} FPS</b> · 最低 5 秒 {r['minimum_5s_fps']:.2f} FPS · P99 {r['p99_frame_ms']:.2f} ms</p>
<div class="pair">
<figure>
<img src="{before['picture']}">
<figcaption>{html.escape(before['title'])}</figcaption>
</figure>
<figure>
<img src="{r['picture']}">
<figcaption>{html.escape(r['title'])}</figcaption>
</figure>
</div>
</section>'''
page='''<!doctype html>
<meta charset="utf-8">
<title>猫手猫脚 · 灯光逐步对比</title>
<style>body{background:#f4eee2;color:#262725;font:16px/1.6 system-ui;margin:0;padding:4vw}main{max-width:1600px;margin:auto}h1{font-size:38px}section{padding:28px 0;border-top:1px solid #c9c3b6}.pair{display:grid;grid-template-columns:1fr 1fr;gap:16px}figure{margin:0}img{width:100%;border-radius:10px}figcaption{padding:8px}b{color:#d35e28}@media(max-width:800px){.pair{grid-template-columns:1fr}}</style>
<main>
<h1>猫手猫脚 · 灯光逐步对比</h1>
<p>本机 Mac 实机 · 1920×1080 · 100% 渲染比例 · 软件 Lumen，无硬件光追。每阶段预热 15 秒，前台采样 30 秒；采样显示正常 HUD，截图隐藏 HUD。NPC 保持巡逻，场景机位固定。关闭垂直同步与帧率上限后测量；正式游戏保留 60 FPS 上限。</p>
<p>每一步与前一步并排展示。最低 5 秒平均帧率用于 45 FPS 门槛判断，P99 记录偶发慢帧。本报告只代表本机此机位；移动玩法另测，不推断 Windows 表现。</p>'''+body+'''<p>原始 JSON、PNG 与 fps.csv 同目录保存。Lumen 设置参考 <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine">Epic 性能指南</a>。</p>
</main>'''
(out/'comparison.html').write_text(page)
print(json.dumps([{k:r[k] for k in ['title','average_fps','minimum_5s_fps','p99_frame_ms']} for r in rows],ensure_ascii=False,indent=2))
