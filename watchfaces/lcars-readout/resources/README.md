# Resources

## fonts/

**Antonio** (`Antonio-Bold.ttf`) — Google Fonts, SIL Open Font License.
Lisensteksten ligger i `fonts/OFL.txt`. Bygges inn i fem størrelser, se
`package.json`.

## images/

Ikonene er hentet fra **Material Symbols** (Google), lisensiert
**Apache-2.0** — se `images/LICENSE-material-symbols.txt`. Apache-2.0 krever
kun at lisensteksten følger med, ikke synlig kreditering i appen.

Kilde-SVG-ene ligger i `images/src/` slik at ikonene kan bygges på nytt uten
nett. PNG-ene genereres av `../tools/make_icons.py`:

```bash
python3 tools/make_icons.py            # bygg PNG-er fra src/
python3 tools/make_icons.py --download # hent SVG-ene på nytt først
```

| PNG | Material Symbols-glyf |
|---|---|
| `icon_clear.png` | `sunny` |
| `icon_cloud.png` | `cloud` |
| `icon_rain.png` | `rainy` |
| `icon_snow.png` | `weather_snowy` |
| `icon_temp.png` | `device_thermostat` |
| `icon_heart.png` | `favorite` |
| `icon_steps.png` | `footprint` |

Scriptet rasteriserer hver SVG stort, beskjærer til selve figuren og skalerer
ned til 20×20 med hard terskel. Terskelen er nødvendig: Pebble tegner disse
med `GCompOpSet`, så antialiaserte gråtoner ville blitt til støy på klokka.
