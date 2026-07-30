# Resources

## fonts/

**Antonio** (`Antonio-Bold.ttf`) — Google Fonts, SIL Open Font License.
Lisensteksten ligger i `fonts/OFL.txt`. Bygges inn i fire størrelser, se
`package.json`.

## images/ — bakgrunnen

`LCARS-readout_background_2.png` er kilden, håndtegnet. Den bygges om til
`background.png`, som er filen som faktisk kompileres inn:

```bash
python3 tools/prep_background.py
```

Steget flater alfa ut mot hvitt og snapper hver piksel til Pebble-64, så
`background.png` er nøyaktig det klokka viser. Rediger alltid kilden, aldri
`background.png` direkte — den blir overskrevet.

## images/ — ikoner

Ikonene er hentet fra **Material Symbols** (Google), lisensiert
**Apache-2.0** — se `images/LICENSE-material-symbols.txt`. Apache-2.0 krever
kun at lisensteksten følger med, ikke synlig kreditering i appen.

Kilde-SVG-ene ligger i `images/src/` slik at ikonene kan bygges på nytt uten
nett. PNG-ene genereres av `../tools/make_icons.py`:

```bash
python3 tools/make_icons.py            # bygg PNG-er fra src/
python3 tools/make_icons.py --download # hent SVG-ene på nytt først
```

Bare værikonene ligger her — termometer, hjerte og fotspor er malt inn i
bakgrunnen, siden de aldri endrer seg.

| PNG | Material Symbols-glyf |
|---|---|
| `icon_clear.png` | `sunny` |
| `icon_cloud.png` | `cloud` |
| `icon_rain.png` | `rainy` |
| `icon_snow.png` | `weather_snowy` |

Scriptet rasteriserer hver SVG stort, beskjærer til selve figuren og skalerer
ned til 20×20 med hard terskel. Terskelen er nødvendig: Pebble tegner disse
med `GCompOpSet`, så antialiaserte gråtoner ville blitt til støy på klokka.
