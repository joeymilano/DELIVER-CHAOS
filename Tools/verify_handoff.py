"""Verify the private full-project archive after extraction (no network required)."""
from pathlib import Path
import json,hashlib,sys
root=Path(__file__).resolve().parents[1]
manifest=root/'HANDOFF_MANIFEST.json'
if not manifest.exists():sys.exit('HANDOFF_MANIFEST.json is present in the full project handoff archive only.')
rows=json.loads(manifest.read_text(encoding='utf-8'));bad=[]
for name,row in rows.items():
    p=root/name
    if not p.is_file() or p.stat().st_size!=row['size'] or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:bad.append(name)
print('Checked',len(rows),'files; mismatches:',len(bad))
for name in bad:print(name)
sys.exit(bool(bad))
