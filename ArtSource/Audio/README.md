# 游戏声音

猫叫已替换为 Freesound 的两段真实猫叫录音，采用 CC0 1.0。源文件及处理记录见 FieldRecordings/LICENSE.md，署名信息随包放在 Content/Licenses/CatRecordings.txt。

主人对白使用 ElevenLabs 的 Jing - Natural, Bright and Conversational，Eleven v3，Stability Natural 0.5。此次账号为 Free，三次生成共 104 credits（10,000 → 9,896）。生成结果只保留在被 Git 忽略的 ElevenLabsLocal 文件夹，供本地试听；正式商业使用需在付费套餐下重新生成。官方说明 https://elevenlabs.io/docs/help-center/legal/can-i-publish-the-content-i-generate-on-the-platform

| 文件 | 生成台词 |
|---|---|
| OwnerCaught | [surprised] 哎，咪咪！爪子放下。[sighs] 我都看见了。 |
| OwnerGood | [warmly] 嗯，真乖。[chuckles] 是我错怪你啦。 |
| OwnerHome | [surprised] 我回来啦。哎？[sighs] 这才出去一会儿啊。 |

保留生成 MP3，导入版转为 44.1 kHz 单声道 PCM，85 Hz 高通、12 kHz 低通、-20 LUFS / -3 dBTP。Tools/import_voice_refresh.py 导入猫叫及存在的本地对白。公开源码没有旧机械人声回退，缺少本地主人音频时仍显示字幕。

主人对白每次只播一句；猫叫按片段时长限制重叠，装无辜的小叫声至少间隔十秒。回家对白延后到首次 OIIA 表演结束，以免同时抢声。暂停、重开和返回选猫会处理对应声音。

Clink、Thud、CameraOff 为原有短合成占位效果；Tools/prepare_party_audio.py 现在只生成这三项，不会覆盖猫叫或对白。不生成背景音乐。
