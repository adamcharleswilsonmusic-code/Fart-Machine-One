# SPDX-License-Identifier: GPL-3.0-only
import importlib.util, tempfile, wave, struct, math, subprocess, sys, json, hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('bank',root/'tools/import_samples.py');bank=importlib.util.module_from_spec(spec);spec.loader.exec_module(bank)
with tempfile.TemporaryDirectory() as td:
    folder=Path(td); wav=folder/'01-example.wav'
    def write(freq,width=2):
        with wave.open(str(wav),'wb') as w:
            w.setparams((2,width,44100,4410,'NONE','not compressed'))
            for i in range(4410):
                v=int(10000*math.sin(2*math.pi*freq*i/44100))
                w.writeframesraw(struct.pack('<hh',v,v) if width==2 else bytes([128,128]))
    write(440);x=bank.read_wav(wav);assert len(x)==2205 and x[0]==x[-1]==0
    low=sum(v*v for v in x)
    write(16000);high=sum(v*v for v in bank.read_wav(wav));assert high<low*.01
    write(440)
    out=folder/'bank.h'; subprocess.run([sys.executable,str(root/'tools/import_samples.py'),str(folder),'--output',str(out)],check=True)
    before=out.read_bytes();subprocess.run([sys.executable,str(root/'tools/import_samples.py'),str(folder),'--output',str(out)],check=True);assert out.read_bytes()==before
    meta=json.loads(out.with_suffix('.json').read_text());assert meta['mapping']==[0]*27 and not meta['placeholder']
    assert meta['samples'][0]['license']=='USER TO PROVIDE SOURCE/LICENCE'
    provenance=folder/'provenance.json'
    row={'filename':wav.name,'sha256':hashlib.sha256(wav.read_bytes()).hexdigest(),
         'license':'CC0-1.0','source_url':'https://example.invalid/recording','creator':'test author'}
    provenance.write_text(json.dumps({'samples':[row]}))
    command=[sys.executable,str(root/'tools/import_samples.py'),str(folder),'--output',str(out),'--provenance',str(provenance)]
    subprocess.run(command,check=True)
    meta=json.loads(out.with_suffix('.json').read_text())
    assert meta['samples'][0]['license']=='CC0-1.0' and meta['samples'][0]['provenance']==row
    assert len(meta['samples'][0]['pcm_sha256'])==64
    approved_header=out.read_bytes();approved_meta=out.with_suffix('.json').read_bytes()
    for invalid in ({**row,'sha256':'0'*64},{**row,'filename':'different.wav'}):
        provenance.write_text(json.dumps({'samples':[invalid]}))
        result=subprocess.run(command,capture_output=True)
        assert result.returncode!=0 and out.read_bytes()==approved_header
        assert out.with_suffix('.json').read_bytes()==approved_meta
    provenance.write_text(json.dumps({'samples':[row,row]}))
    assert subprocess.run(command,capture_output=True).returncode!=0
    write(440,1)
    result=subprocess.run([sys.executable,str(root/'tools/import_samples.py'),str(folder),'--output',str(out)],capture_output=True)
    assert result.returncode!=0 and out.read_bytes()==before
print('Importer: PCM validation, antialiasing, fades, deterministic output, mapping, generic licence fallback, hash-bound provenance, stale/duplicate rejection and failed import preservation PASS')
