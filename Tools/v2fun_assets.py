"""Offline production client. Key is read from environment or explicit --clipboard, never saved.
No automatic POST retries: an ambiguous submission must be reconciled before spending again.
"""
import argparse,base64,json,os,subprocess,urllib.request,urllib.error
from pathlib import Path
BASE='https://api.v2fun.art/api/v1'
p=argparse.ArgumentParser();p.add_argument('action',choices=['balance','submit','poll']);p.add_argument('--clipboard',action='store_true');p.add_argument('--spec',type=Path);p.add_argument('--state',type=Path);args=p.parse_args()
key=subprocess.check_output(['pbpaste']).decode().strip() if args.clipboard else os.environ.get('V2FUN_API_KEY','').strip()
if not key.startswith('ak_live_') or any(c.isspace() for c in key):raise SystemExit('No valid V2Fun credential supplied; key was not displayed.')
def request(path,payload=None):
 req=urllib.request.Request(BASE+path,data=json.dumps(payload).encode() if payload else None,headers={'Authorization':'Bearer '+key,'Content-Type':'application/json'})
 try:
  with urllib.request.urlopen(req,timeout=55) as r:return json.load(r)
 except urllib.error.HTTPError as e:raise SystemExit('V2Fun HTTP '+str(e.code)+'; request was not retried.')
if args.action=='balance': print(json.dumps(request('/balance')))
elif args.action=='submit':
 if args.state.exists():raise SystemExit('State already exists; poll it instead of submitting twice.')
 spec=json.loads(args.spec.read_text());payload=dict(spec['request'])
 if 'reference_file' in spec:payload['input_image']='data:image/png;base64,'+base64.b64encode(Path(spec['reference_file']).read_bytes()).decode()
 if 'model_file' in spec:payload['input_model']='data:application/octet-stream;base64,'+base64.b64encode(Path(spec['model_file']).read_bytes()).decode()
 args.state.parent.mkdir(parents=True,exist_ok=True)
 # Pre-submit sentinel prevents accidental retries after timeouts.
 args.state.write_text(json.dumps({'status':'SUBMITTING','asset':spec['asset']}))
 endpoint=spec.get('endpoint','/3d_models/meshes')
 result=request(endpoint,payload)
 result['_endpoint']=endpoint
 args.state.write_text(json.dumps(result,indent=2))
 print(json.dumps({k:result.get(k) for k in ['task_uuid','status','queue_position','estimated_queue_time']}))
else:
 prior=json.loads(args.state.read_text());task=prior.get('task_uuid')
 if not task:raise SystemExit('Submission outcome is ambiguous; reconcile with provider. No retry performed.')
 endpoint=prior.get('_endpoint','/3d_models/meshes');result=request(endpoint+'/'+task);result['_endpoint']=endpoint;args.state.write_text(json.dumps(result,indent=2))
 print(json.dumps({k:result.get(k) for k in ['task_uuid','status','queue_position','estimated_queue_time']}))
