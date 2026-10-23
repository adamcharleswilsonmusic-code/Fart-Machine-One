# Fart Machine One — recorded CC0 audio

These ten samples derive from creator-described real recordings published under CC0 on their individual Freesound pages. Attribution is retained for provenance. Original downloaded bytes are preserved in `source-originals/`. Selected WAVs listed below are under `bank-inputs/`.

License: [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/). Verified on 2026-10-22.

| File | Freesound recording | Creator | Original preview | Selected seconds |
|---|---|---|---|---|
| 01.wav | [FART - 2](https://freesound.org/people/SamuelGremaud/sounds/518699/) | SamuelGremaud | 01 | 1.135 |
| 02.wav | [Fart 2](https://freesound.org/people/Sadiquecat/sounds/790551/) | Sadiquecat | 03 | 0.620 |
| 03.wav | [Short fart](https://freesound.org/people/Sadiquecat/sounds/787755/) | Sadiquecat | 04 | 0.270 |
| 04.wav | [04 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803563/) | anndszjuvupftbim | 06 | 1.150 |
| 05.wav | [16 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803574/) | anndszjuvupftbim | 07 | 0.332 |
| 06.wav | [18 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803576/) | anndszjuvupftbim | 08 | 0.620 |
| 07.wav | [24 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803695/) | anndszjuvupftbim | 09 | 0.437 |
| 08.wav | [25 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803696/) | anndszjuvupftbim | 10 | 0.391 |
| 09.wav | [26 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803697/) | anndszjuvupftbim | 11 | 0.279 |
| 10.wav | [20 - fart](https://freesound.org/people/anndszjuvupftbim/sounds/803578/) | anndszjuvupftbim | New | 0.464 |

Previous preview 02 (874487) and 05 (135716) are excluded by user choice.

Processing: decode to 44.1 kHz, average source channels, select a contiguous event excerpt, peak level to −6 dBFS (gain capped at +12 dB), apply 3 ms edge fades, encode mono PCM16. No pitch/speed changes, synthesis, denoising, or internal event splicing. Exact source-relative trim frames, gain, original format and hashes are in SOURCE-MANIFEST.json.

Two retained sources contain separated events: 803563 uses the first event; 803576 uses the strongest middle event. The complete original downloads are retained.

Quality assessment used source statements and technical waveform analysis. It is not a claim that the assistant heard and judged the recordings. MP3/M4A source quality and incidental recording noise remain.

To reproduce all ten selected WAVs from these originals, run `python3 reproduce_audio.py --ffmpeg /path/to/ffmpeg` from this directory. Output is a separate `reproduced-bank-inputs/` folder and all hashes must match. This helper is GPL-3.0-only code; the recordings remain CC0.
