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

Rammen — former, overskrifter og de tre faste ikonene (termometer, hjerte,
fotspor) — er **ett bakgrunnsbilde**, håndtegnet av brukeren:
`resources/images/LCARS-readout_background.png`. Koden tegner bare verdiene
oppå. Alle koordinater i `src/c/lcars_theme.h` er målt mot det bildet, så de
må måles om hvis illustrasjonen endres.

| Felt | Posisjon |
|---|---|
| Klokkeslett | x 52–198, y 16–72 (Antonio 58) |
| Dato | x 52–198, y 109–141 (Antonio 30) |
| Batteri | x 0–50, y 203–228 (Antonio 16) |
| Værikon | x 52, y 158, 20×20 |
| Værtilstand | x 74–126, y 159–179 |
| Temperatur | x 70–122, y 194–214 |
| Puls | x 148–198, y 159–179 |
| Skritt | x 148–198, y 194–214 |

Værikonet er det eneste ikonet som tegnes i kode, siden det er det eneste som
bytter. Det ligger på x52 og ikke i flukt med termometeret på x57, fordi
`CLEAR`/`CLOUD` trenger 47 px og x57 bare levner 43.

Bakgrunnen klargjøres av `tools/prep_background.py`: alfa flates ut mot hvitt,
og hver piksel snappes til Pebble-64. Sjeldne farger fra antialiasing (under
32 px) foldes inn i nærmeste nabo, som får fargetallet ned til 9 — under 16, så
bitmapen kan lagres som `4BitPalette` og bruker 22 KB heap i stedet for 45 KB.

## Palett

Emery kan bare vise 64 farger (hver kanal 00/55/AA/FF), så 35,5 % av pikslene
i illustrasjonen flyttet seg under snappingen. Fire farger endte annerledes
enn antatt:

| Element | Tegnet | Blir på klokka | |
|---|---|---|---|
| Lyse barer | `#D6DBF0` | `#FFFFFF` | forsvinner mot hvit bakgrunn |
| Blå elbow | `#84B9E2` | `#AAAAFF` | identisk med barene |
| Pills/kapsler | ca. `#B0B0C8` | `#AAAAAA` | grå, ikke fiolett |
| Batteriblokk | `#F7B195` | `#FFAAAA` | rosa, ikke oransje |

Trygge alternativer som ligger *på* paletten: `#AAAAFF` lyse barer,
`#55AAFF` blå elbow, `#AA55FF` fiolette pills, `#AA5555` rose (traff riktig),
`#FF5555` rød (traff riktig), `#FFAA55` oransje batteri.

Unngå også mykt antialiasing: paletten har bare fire nivåer per kanal, så
myke kanter blir klumpete gråtoner i stedet for jevne overganger.

## Typografi

**Antonio** (Google Fonts, SIL Open Font License — `resources/fonts/OFL.txt`),
i fire størrelser. Den kondenserte formen er det som gjør at `22.06.2026` i det
hele tatt får plass i en 146 px kolonne. Hver størrelse er begrenset med
`characterRegex` til akkurat de tegnene den bruker, så alle fire til sammen
veier lite.

| Ressurs | Bruk | Tegn |
|---|---|---|
| `FONT_ANTONIO_58` | klokkeslett | `[0-9:]` |
| `FONT_ANTONIO_30` | dato | `[0-9.]` |
| `FONT_ANTONIO_22` | avlesningsverdier | `[0-9A-Z%°.:-]` |
| `FONT_ANTONIO_16` | batteriprosent | `[0-9%]` |

## Ikoner

Bare de fire værikonene ligger som ressurser nå — termometer, hjerte og
fotspor er malt inn i bakgrunnen, siden de aldri endrer seg. De fire som er
igjen er 20×20 px sort-på-transparent fra **Material Symbols** (Google,
**Apache-2.0** — lisenstekst i `resources/images/LICENSE-material-symbols.txt`).

Kilde-SVG-ene ligger i `resources/images/src/`, og PNG-ene bygges av
`tools/make_icons.py`.

## Datakilder

| Felt | Kilde | Status |
|---|---|---|
| Tid, dato | `tick_timer_service` (MINUTE_UNIT) | live |
| Batteri | `battery_state_service` | live |
| Puls, skritt | `HealthService` | live på klokka, tomt i emulator |
| Vær | `src/pkjs/index.js` → Open-Meteo | krever telefon med posisjon |

Illustrasjonen har ingen plass til Bluetooth-status, så den indikatoren er
tatt ut. De to øverste blokkene i venstre skinne er dekorative — hvis en av
dem skal vise tilkobling, er det bare å si.

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
Ressurser 13 998 B / 256 KB, statisk RAM 2 740 B / 128 KB
(pluss ~22 KB heap for bakgrunnsbitmapen).

### Fallgruve på Linux uten IPv6

`pebble install --emulator` feiler med `[Errno 111] Connection refused` fordi
`pypkjs` binder websocket-serveren til IPv6. Fiks ved å endre
`pywsgi.WSGIServer(("", self.port), ...)` til `("0.0.0.0", self.port)` i
`pypkjs/runner/websocket.py`. Må gjentas etter hver reinstallasjon av
`pebble-tool`.
