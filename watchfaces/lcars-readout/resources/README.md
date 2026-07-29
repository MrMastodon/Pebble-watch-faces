# Resources

- `fonts/` — legg Antonio (Google Fonts, SIL Open Font License) TTF-filer
  her under implementasjon, f.eks. `antonio-bold.ttf`. Registrer dem i
  `package.json` sin `pebble.resources.media`-liste og generer en
  `ResourceId` for bruk med `fonts_load_custom_font()`.
- `images/` — reservert for eventuelle bitmap-ressurser (ikke brukt av
  MVP-skjelettet, som tegner alle LCARS-former direkte med `graphics_fill_rect`).
