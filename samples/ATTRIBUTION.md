# Audio provenance

Eleven user-supplied recordings are included in `samples/originals/` as source
assets for building Fart Machine One. They were imported on 2026-10-11.
SHA-256 checksums and format/duration are in `SOURCE-MANIFEST.json`.
Total source duration: 6.312585 seconds. All are mono PCM16, 44,100 Hz.

Adam identified Pixabay as the source on 2026-10-11 and described the sounds
as royalty-free. Specific item URLs, contributor names and download certificates
are unavailable. This is user-reported provenance, not an independently verified
licence for each recording. Do not label these recordings public domain, CC0 or
GPL solely on that basis.

Pixabay's current general terms allow creative uses and restrict standalone
redistribution. These files are included for reproducing the instrument, not offered as a
separate stock sound library. The terms do not explicitly address this firmware
sample-instrument use, and inclusion is not a claim of verified permission for
each recording. The GPL code licence does not relicense the audio.

References checked 2026-10-11:
- https://pixabay.com/service/license-summary/
- https://pixabay.com/service/terms/

Key order cycles through 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, then repeats.
There is no 8.wav in the supplied set; eleven files are intentional.
The importer makes mono 22,050 Hz PCM with DC removal and brief fades, without
truncation. Original recording files are unmodified.

The separate TEST fixture bank is generated locally for regression tests and
is CC0 test audio. It is not included in the actual-sample firmware bank.
