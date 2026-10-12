// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
let mem;const wasi=new Proxy({clock_time_get:(id,p,out)=>{new DataView(mem.buffer).setBigUint64(out,0n,true);return 0;},fd_write:(fd,iov,n,out)=>{new DataView(mem.buffer).setUint32(out,0,true);return 0;}},{get:(t,k)=>k in t?t[k]:()=>0});
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})});
const e=instance.exports;mem=e.memory;if(e._initialize)e._initialize();e.web_boot();e.web_master(4096);
const PLAY=1<<10,REC=1<<11;
function render(n){let peak=0;while(n){const k=Math.min(256,n);e.web_render(k);for(const x of new Float32Array(mem.buffer,e.web_out_l(),k)){assert(Number.isFinite(x));peak=Math.max(peak,Math.abs(x));}n-=k;}return peak;}
function button(bit){e.web_buttons(bit);render(882);e.web_buttons(0);render(882);}
function area(y,h){return Array.from(new Uint16Array(mem.buffer,e.web_fb()+y*240*2,240*h));}
e.web_enc(3,-100);render(4410);const controls=area(178,46),idle=area(30,120);
button(REC);assert.equal(e.web_lit_buttons()&(PLAY|REC),PLAY|REC);
e.web_keys(3);render(441);e.web_keys(0);render(441);button(REC);
assert.equal(e.web_lit_buttons()&(PLAY|REC),PLAY);
render(44100);assert.deepEqual(area(30,120),idle);
assert(render(44100)>0.001); /* a recorded chord sounds again without touching keys */
assert.deepEqual(area(178,46),controls);
button(PLAY);assert.equal(e.web_lit_buttons()&(PLAY|REC),0);assert.equal(render(4410),0);
button(PLAY);assert(render(4410)>0.001);
e.web_buttons(REC);render(48510);e.web_buttons(0);render(882);
assert.equal(e.web_lit_buttons()&(PLAY|REC),0);button(PLAY);assert.equal(render(132300),0); /* cleared loop is empty */
console.log('WASM SEQ PASS: REC chord/overdub-off, loop audio without input, puffs return to idle, stable effect cards, stop/restart, long-REC clear with no release toggle');
