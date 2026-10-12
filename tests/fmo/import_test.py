# SPDX-License-Identifier: GPL-3.0-only
import importlib.util, tempfile, wave, struct, math, subprocess, sys, json
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
    write(440,1)
    result=subprocess.run([sys.executable,str(root/'tools/import_samples.py'),str(folder),'--output',str(out)],capture_output=True)
    assert result.returncode!=0 and out.read_bytes()==before
print('Importer: PCM validation, stereo downmix, resampling/antialiasing, fades, deterministic output, repeated mapping, failed import preservation PASS')
