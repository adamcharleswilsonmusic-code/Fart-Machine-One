#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Rebuild hash-verified bank inputs from the preserved CC0 downloads.

Requires Python 3 and FFmpeg. No third-party Python modules are required.
Example, from this directory:
    python3 reproduce_audio.py --ffmpeg /path/to/ffmpeg
Outputs are created in reproduced-bank-inputs/ only after all hashes match.
The SOURCE-MANIFEST records the FFmpeg version used for the release; a different
decoder or resampler version may produce different bytes and fail verification.
"""
from __future__ import annotations

import argparse
import array
import hashlib
import json
import math
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import wave


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=Path(__file__).resolve().parent / 'SOURCE-MANIFEST.json')
    parser.add_argument('--output', type=Path, default=None)
    parser.add_argument('--ffmpeg', default=shutil.which('ffmpeg') or 'ffmpeg')
    args = parser.parse_args()
    manifest = args.manifest.resolve()
    base = manifest.parent
    output = args.output.resolve() if args.output else base / 'reproduced-bank-inputs'
    if output.exists():
        raise SystemExit(f'Refusing to overwrite existing output: {output}')
    data = json.loads(manifest.read_text())
    rows = data['samples']
    if len(rows) != 10 or len({x['filename'] for x in rows}) != 10:
        raise ValueError('Expected ten unique samples')
    for row in rows:
        source = base / row['original_file']
        if digest(source) != row['original_sha256']:
            raise ValueError(f'Original checksum mismatch: {source.name}')
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.audio-reproduction-', dir=output.parent) as directory:
        stage = Path(directory)
        for row in rows:
            source = base / row['original_file']
            raw = subprocess.check_output([
                args.ffmpeg, '-nostdin', '-v', 'error', '-i', str(source),
                '-map', '0:a:0', '-ar', '44100', '-f', 'f32le', '-'
            ])
            samples = array.array('f')
            samples.frombytes(raw)
            if sys.byteorder != 'little':
                samples.byteswap()
            channels = row['original_format']['channels']
            if len(samples) % channels:
                raise ValueError('Invalid source frame alignment')
            mono = [sum(samples[i:i+channels])/channels for i in range(0, len(samples), channels)]
            p = row['processing']
            excerpt = mono[p['trim_start_frame']:p['trim_end_frame_exclusive']]
            if len(excerpt) != row['frames'] or not all(math.isfinite(x) for x in excerpt):
                raise ValueError('Invalid excerpt')
            peak = max(abs(x) for x in excerpt)
            gain = min(10**(p['maximum_amplification_db']/20), 10**(p['peak_target_dbfs']/20)/peak) if peak else 1
            fade_in, fade_out = p['fade_in_frames'], p['fade_out_frames']
            pcm = array.array('h')
            for i, x in enumerate(excerpt):
                edge = min(1.0, i/fade_in if fade_in else 1, (len(excerpt)-1-i)/fade_out if fade_out else 1)
                value = round(x*gain*edge*32767)
                if not -32768 < value < 32767:
                    raise ValueError('Unexpected clipping')
                pcm.append(value)
            if sys.byteorder != 'little':
                pcm.byteswap()
            target = stage / row['filename']
            if target.parent != stage:
                raise ValueError('Invalid bank filename')
            with wave.open(str(target), 'wb') as w:
                w.setnchannels(1)
                w.setsampwidth(2)
                w.setframerate(44100)
                w.writeframes(pcm.tobytes())
            if digest(target) != row['sha256']:
                raise ValueError(f'Reproduced checksum mismatch for {row["filename"]}; use the recorded FFmpeg build: {data["decoder"]}')
        stage.rename(output)
    print(f'Reproduced all ten bank inputs with matching SHA-256 hashes: {output}')


if __name__ == '__main__':
    main()
