# LCARS Readout — design og status

| Slik den ser ut nå (emulator) | Med vær- og helsedata til stede |
|---|---|
| ![](screenshot.png) | ![](screenshot-with-data.png) |

Venstre bilde er tatt rett fra emulatoren. Emulatoren har ingen posisjons-
kilde og ingen helsedata, så vær, puls og skritt står som `--`/`0`. Høyre
bilde er samme bygg med verdiene fylt inn manuelt, for å vise hvordan det ser
ut på klokka når telefonen er tilkoblet.

## Konsept

LCARS-inspirert urskive for Pebble Time 2 (Emery, 200×228 px, farge), bygget
etter en mockup fra brukeren. Lys bakgrunn, svarte tall og pastellfargede
LCARS-former — det motsatte av klassisk LCARS, og hele poenget: tallene skal
være lette å lese i dagslys på en reflektiv skjerm.

Inspirert av LCARS-tradisjonen generelt og av layout-ideene i
`AKlitbo/pebble-watchfaces`. All kode her er egenutviklet — det prosjektet er
lisensiert PolyForm Noncommercial og ingenting derfra er kopiert.

## Layout (200×228)

Venstre skinne `x 2..48`, innholdskolonne `x 52..198`.

```
 y   2..16   [lys blokk]           │ ▐ TIME ▬▬▬▬▬▬▬▬▬▬▬ ▌
 y  18..90   [blå elbow, L]        │ 17:54            (Antonio 58)
             arm y 78..90 → x 96   │ segmentskinne y 80..90
 y  94..138  [rose elbow, Γ]       │ STARDATE ▮ ▬▬▬▬▬ ▌
             arm y 94..106 → x 66  │ 29.07.2026       (Antonio 30)
 y 140..172  [LINK ACTIVE]         │ ▐SENSORS▬▌   ▐VITALS▬▌
                                   │ ☀ CLEAR      ♥ 64
 y 174..196  [rød dekorblokk]      │              ▐TRAVERSAL▌
 y 198..226  [batteri %]           │ 🌡 18°C      👣 8432
```

Begge elbows tegnes av `lcars_elbow_top()` / `lcars_elbow_bottom()` i
`src/c/lcars_draw.c`: vertikal arm + horisontal arm, der den konkave
innerkurven skjæres ut ved å male bakgrunnsfargen tilbake med en avrundet
rektangel i innerhjørnet.

Alle fem header-rader (`TIME`, `STARDATE`, `SENSORS`, `VITALS`, `TRAVERSAL`)
kommer fra én `lcars_header()`. Den måler etikettbredden med
`graphics_text_layout_get_content_size()` og fyller resten med bar + endekapsel
— og dropper baren automatisk når etiketten ikke levner plass (som på
`TRAVERSAL`). `STARDATE` bruker varianten uten ledende pill, så den rose
elbow-armen får løpe inn i etiketten slik mockupen viser.

## Palett

| Element | GColor |
|---|---|
| Bakgrunn | `GColorWhite` |
| Lyse fyllbarer, toppblokk | `GColorBabyBlueEyes` |
| Pills og endekapsler | `GColorLavenderIndigo` |
| Øvre elbow | `GColorPictonBlue` |
| Nedre elbow, LINK-blokk | `GColorRoseVale` |
| Rød dekorblokk | `GColorSunsetOrange` |
| Batteriblokk | `GColorRajah` |
| All tekst | `GColorBlack` |

Fargene ser mer dempet ut på klokka enn hex-verdiene tilsier — Emery-skjermen
er reflektiv, og emulatoren simulerer det.

## Typografi

**Antonio** (Google Fonts, SIL Open Font License — `resources/fonts/OFL.txt`),
i fem størrelser. Den kondenserte formen er det som gjør at `22.06.2026` i det
hele tatt får plass i en 146 px kolonne. Hver størrelse er begrenset med
`characterRegex` til akkurat de tegnene den bruker, så alle fem til sammen
veier under 15 KB.

| Ressurs | Bruk | Tegn |
|---|---|---|
| `FONT_ANTONIO_58` | klokkeslett | `[0-9:]` |
| `FONT_ANTONIO_30` | dato | `[0-9.]` |
| `FONT_ANTONIO_22` | avlesningsverdier | `[0-9A-Z%°.:-]` |
| `FONT_ANTONIO_16` | batteriprosent | `[0-9%]` |
| `FONT_ANTONIO_14` | etiketter | `[A-Z ]` |

## Ikoner

Sju 20×20 px sort-på-transparent PNG-er fra **Material Symbols** (Google,
**Apache-2.0** — lisenstekst i `resources/images/LICENSE-material-symbols.txt`).
Apache-2.0 krever bare at lisensteksten følger med, ikke synlig kreditering i
appen.

Kilde-SVG-ene ligger i `resources/images/src/`, og PNG-ene bygges av
`tools/make_icons.py`: hver SVG rasteriseres stort, beskjæres til selve
figuren og skaleres ned med hard terskel — nødvendig fordi Pebble tegner dem
med `GCompOpSet`, så antialiaserte gråtoner ville blitt støy.

Termometeret bruker `device_thermostat`, ikke `thermostat`: sistnevnte har
skalastreker som utvider omrisset og dytter kolben ut av senter i denne
størrelsen.

## Datakilder

| Felt | Kilde | Status |
|---|---|---|
| Tid, dato | `tick_timer_service` (MINUTE_UNIT) | live |
| Batteri | `battery_state_service` | live |
| LINK ACTIVE / LOST | `connection_service` | live |
| Puls, skritt | `HealthService` | live på klokka, tomt i emulator |
| Vær | `src/pkjs/index.js` → Open-Meteo | krever telefon med posisjon |

Værhentingen bruker Open-Meteo, som ikke krever API-nøkkel, og henter hver
30. minutt. Siste måling lagres med `persist_write_int`, så den overlever en
omstart av urskiven i stedet for å blanke ut. Felter uten data viser `--`.

**Ikke verifisert på fysisk klokke:** vær-, puls- og skrittverdiene er kun
testet i emulatoren, der de to siste er tomme og været ikke kan hentes (ingen
posisjonskilde). Selve mottakssiden i C og render-veien er verifisert ved å
fylle inn verdier manuelt — se høyre skjermbilde over.

## Bygg

```bash
cd watchfaces/lcars-readout
pebble build                      # -> build/lcars-readout.pbw
pebble install --emulator emery   # krever X-display
```

Sist bygde `.pbw` ligger i `dist/lcars-readout.pbw` og kan installeres direkte
på klokka via Pebble-telefonappen.

Bygget med `pebble-tool` 5.0.39 og Pebble SDK 4.17, target `emery`.
Ressurser 14 459 B / 256 KB, RAM 4 713 B / 128 KB.

### Fallgruve på Linux uten IPv6

`pebble install --emulator` feiler med `[Errno 111] Connection refused` fordi
`pypkjs` binder websocket-serveren til IPv6. Fiks ved å endre
`pywsgi.WSGIServer(("", self.port), ...)` til `("0.0.0.0", self.port)` i
`pypkjs/runner/websocket.py`. Må gjentas etter hver reinstallasjon av
`pebble-tool`.
