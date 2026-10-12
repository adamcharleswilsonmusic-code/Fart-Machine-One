// SPDX-License-Identifier: GPL-3.0-only
// The real bank + actual C UI in WebAssembly, including short repeated taps.
import fs from 'node:fs';
import assert from 'node:assert/strict';
let mem;
const wasi=new Proxy({clock_time_get:(id,p,out)=>{new DataView(mem.buffer).setBigUint64(out,0n,true);return 0;},fd_write:(fd,iov,n,out)=>{new DataView(mem.buffer).setUint32(out,0,true);return 0;}},{get:(t,k)=>k in t?t[k]:()=>0});
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})});
const e=instance.exports;mem=e.memory;if(e._initialize)e._initialize();e.web_boot();e.web_master(4096);
function render(frames){let peak=0;while(frames>0){let n=Math.min(256,frames);e.web_render(n);for(const x of new Float32Array(mem.buffer,e.web_out_l(),n)){assert(Number.isFinite(x));peak=Math.max(peak,Math.abs(x));}frames-=n;}return peak;}
function area(y,h){return Array.from(new Uint16Array(mem.buffer,e.web_fb()+y*240*2,240*h));}
render(2205);const idle=area(30,120),header=area(0,30),controls=area(178,62);
for(let k=0;k<12;k++){e.web_keys(1<<(k%27));e.web_keys(0);render(441);}
assert.notDeepEqual(area(30,120),idle);assert.deepEqual(area(0,30),header);assert.deepEqual(area(178,62),controls);
assert(render(4410)>0.001);render(44100);assert.deepEqual(area(30,120),idle);
for(let k=0;k<27;k++){e.web_keys(1<<k);e.web_keys(0);let peak=render(48510);assert(peak>0.00001 && peak<1);}
console.log('WASM: real-bank audio on 27 keys; rapid-tap puff animation; fixed controls; expiry; finite bounded audio PASS');
