"""Download completed API output using provider-returned HTTPS links; never send API headers."""
import argparse,json,urllib.request,urllib.parse,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('state',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
s=json.loads(a.state.read_text());assert s['status']=='COMPLETED','Poll completion before download'
a.output.mkdir(parents=True,exist_ok=True);records=[]
for meta in s.get('metadata') or []:
 for d in meta.get('downloads') or []:
  uri=d.get('asset_path','');url=d.get('download_url','')
  if not url.startswith('https://'):continue
  name=Path(uri).name
  if not name or name in [r['file'] for r in records]:continue
  target=a.output/name
  if not target.exists():
   with urllib.request.urlopen(url,timeout=90) as r:target.write_bytes(r.read())
  records.append({'file':name,'asset_path':uri,'sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'bytes':target.stat().st_size})
report={'task_uuid':s['task_uuid'],'status':s['status'],'model':s.get('model'),'files':records,'mesh_metadata':[m.get('mesh_info') for m in s.get('metadata') or []]}
(a.output/'provenance.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
