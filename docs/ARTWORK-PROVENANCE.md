# Character and puff artwork provenance

Created from scratch for Adam’s Fart Machine One on 2026-10-11.

The round cartoon man and puff are original procedural drawings authored directly
in `firmware/src/app/cartoon.h` during this project. They use hand-chosen ellipse,
rectangle and line coordinates and colours, rendered by the firmware’s canvas.
The idle and reacting faces, body, clothes, shoes, cloud lobes and drifting wisps
were composed in that file. No character image, sprite sheet, icon pack, photograph,
traced artwork, image-generation reference or downloaded illustration was used.
No particular existing character or artist was requested or used as a reference.

Provenance inspection:

- `cartoon.h` is a new project file, absent from the pinned FoMni foundation.
- The drawing calls use `cv_rect`, `cv_line`, `cv_begin`, `cv_blit` and a new
  integer ellipse helper. There are no image-file reads, embedded raster images,
  external sprite data or network asset references in the character drawing.
- `ui.c` calls those drawings directly; the puff timestamps drive the animation.
- Screenshots and the animated GIF are renders of this same code, not stock art
  or separate artist mockups.

The artwork code is included with this GPL-3.0-only firmware source. This statement
records how the artwork was made; it is not a claim that no independently created
cartoon could ever look similar.

## Separate inherited material

The hardware platform, drawing primitives, loader, and plate reverb originate from
FoMni/Felucca and retain their original notices under GPL-3.0-only. The screen font
is Terminus, by Dimitar Toshkov Zhekov, under SIL OFL 1.1; its licence is retained in
`assets/fonts/Terminus-LICENSE.txt`. Neither the character nor its puff comes from
that font or the upstream firmware’s artwork. The browser’s generic FM-1 panel
illustration is inherited from FoMni, distinct from the new on-device character.

The eleven audio recordings were supplied separately by Adam. Their public-domain
provenance has not been verified; see `samples/ATTRIBUTION.md`. This artwork audit
does not assert ownership or licensing of those recordings.
