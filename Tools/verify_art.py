"""Read back imported assets; fail on missing models, animation, or dependencies."""
import unreal, os, json
root=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
report=[]
for cat in ['Ahuai','Jutuan','Wumei']:
    folder='/Game/Characters/'+cat
    assets={}
    for name in ['SM_'+cat,'SK_'+cat,'SK_'+cat+'_Skeleton','model','texture_diffuse','texture_normal','A_Idle','A_Run','A_Walk']:
        asset=unreal.load_asset(folder+'/'+name)
        assert asset is not None, folder+'/'+name
        item={'class':asset.get_class().get_name(),'path':asset.get_path_name()}
        if isinstance(asset,unreal.AnimSequence):
            item['duration_seconds']=asset.get_play_length()
            assert item['duration_seconds']>0
            item['skeleton']=asset.get_editor_property('skeleton').get_path_name()
            assert '/'+cat+'/' in item['skeleton']
        assets[name]=item
    report.append({'cat':cat,'assets':assets})
for name in ['SM_RoundedBlock','SM_Cup','SM_Book','SM_Tray','SM_Lamp','SM_TV','SM_Car','SM_Spring','SM_Speaker']:
    assert unreal.load_asset('/Game/Art/Props/'+name),name
with open(os.path.join(root,'Verification','art-manifest.json'),'w') as f:json.dump(report,f,indent=2)
unreal.log('BAD_CAT_ART_VERIFIED: three models, nine animation clips, nine props')
