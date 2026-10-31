"""Create a private portable full-project archive, including licensed local content."""
from pathlib import Path
import hashlib,json,subprocess,zipfile,datetime,argparse
parser=argparse.ArgumentParser();parser.add_argument("--fast",action="store_true",help="Compress quickly and defer archive CRC/hash re-read");args=parser.parse_args()
root=Path(__file__).resolve().parents[1];stamp=datetime.datetime.now().strftime('%Y%m%d-%H%M%S');dest=root/'Builds'/('Delivery-Chaos-Windows-Project-'+stamp+'.zip')
dest.parent.mkdir(exist_ok=True)
folders=['Source','Config','Content','ArtSource','Tools','Docs','Verification']
files=[]
for folder in folders:
    files.extend(p for p in (root/folder).rglob('*') if p.is_file() and p.name!='.DS_Store' and '__pycache__' not in p.parts)
files.extend(p for p in root.iterdir() if p.is_file() and (p.suffix.lower() in ['.md','.uproject'] or p.name in ['.gitignore','.gitattributes']))
files=sorted(set(files));rows={}
for p in files:
    rows[p.relative_to(root).as_posix()]={'size':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
try:revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
except (OSError,subprocess.CalledProcessError):revision='unavailable'
version={'source_commit':revision,'engine':'5.8.4','created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'kind':'complete-project-source-not-a-playable-Windows-build','includes_local_licensed_assets':True,'excluded':['.git','Binaries','Intermediate','Saved','DerivedDataCache','Builds','Unreal Engine'],'windows_build_verified':False}
prefix='Delivery-Chaos-Project/'
with zipfile.ZipFile(dest,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=1 if args.fast else 4) as z:
    for p in files:z.write(p,prefix+p.relative_to(root).as_posix())
    z.writestr(prefix+'HANDOFF_MANIFEST.json',json.dumps(rows,ensure_ascii=False,indent=2))
    z.writestr(prefix+'HANDOFF_VERSION.json',json.dumps(version,indent=2))
if not args.fast:
    with zipfile.ZipFile(dest) as z:
        bad=z.testzip()
        if bad:raise RuntimeError('Corrupt archive member: '+bad)
        manifest=json.loads(z.read(prefix+'HANDOFF_MANIFEST.json'))
        for name,row in manifest.items():
            data=z.read(prefix+name)
            if len(data)!=row['size'] or hashlib.sha256(data).hexdigest()!=row['sha256']:raise RuntimeError(name)
sha=hashlib.sha256(dest.read_bytes()).hexdigest();dest.with_suffix('.zip.sha256').write_text(sha+'  '+dest.name+'\n')
report=dict(version,archive=dest.name,files_verified=0 if args.fast else len(rows),files_manifested=len(rows),size_bytes=dest.stat().st_size,sha256=sha,crc_ok=None if args.fast else True,all_file_hashes_verified=not args.fast)
(root/'Builds/windows-handoff.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
