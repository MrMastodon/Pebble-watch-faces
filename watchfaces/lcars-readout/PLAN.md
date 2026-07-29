# LCARS Readout — design og status

![Skjermbilde fra emery-emulatoren](screenshot.png)

## Konsept

Et LCARS-inspirert (Star Trek) watchface for Pebble Time 2 (Emery,
200×228 px, farge). Rammen — elbow-kolonnen til venstre og de fargede
etikettbarene — beholder den klassiske, mørke, fargerike LCARS-stilen, men
alle **avlesningsfelt** vises som **svart tekst på lyse paneler**. Det gir
langt bedre lesbarhet enn tradisjonell LCARS (lys tekst på mørk bunn), som
var hele motivasjonen for dette watchfacet.

Inspirert av LCARS-designtradisjonen generelt og av layout-ideene i
`AKlitbo/pebble-watchfaces` sin "LCARS Stardate" — men all kode her er
egenutviklet. Det prosjektet er lisensiert PolyForm Noncommercial og er ikke
fritt gjenbrukbart, så ingenting derfra er kopiert.

## Layout (200×228)

```
┌────────┬──────────────────┐
│▓▓▓▓▓▓▓▓│  ONS 29 JUL      │ y 0–22    elbow-arm + gull datobar
│▓▓┐     └──────────────────┤
│▓▓│  ┌────────────────────┐│
│▓▓│  │      16:38         ││ y 26–84   lyst panel, stor svart tekst
│▓▓│  └────────────────────┘│
│▓▓│  ┌SENSORS─────────────┐│ y 88–130  reservert (vær)
│▓▓│  │        --          ││
│  │  ├VITALS──────────────┤│ y 134–176 reservert (puls/skritt)
│░░│  │        --          ││
│░░│  ├SYSTEMS─────────────┤│ y 180–222 LIVE (batteri + BT)
│░░│  │  100%   LINK       ││
│▒▒│  └────────────────────┘│
└──┴────────────────────────┘
 ▓ oransje   ░ fiolett   ▒ rose
```

- **Elbow-kolonnen** (40 px bred) er en ekte LCARS-elbow: vertikal arm pluss
  en horisontal arm øverst, der den konkave innerkurven "skjæres ut" ved å
  male bakgrunnsfargen tilbake over hjørnet med en avrundet rektangel.
  Kolonnen er segmentert nedover i oransje → fiolett → rose.
- **Tre readout-slots**, definert av `READOUT_COUNT` og posisjonert med
  `READOUT_Y(i)`-makroen i `lcars_theme.h`. Alle tre er fullt wiret opp med
  egen fargekodet etikettbar, lyst datapanel, `TextLayer` og tekstbuffer.
  Slot 0 og 1 viser `--` inntil de får data — å ta dem i bruk er da bare å
  fylle riktig buffer.

## Fargepalett

Alle farger er fra Pebbles 64-fargepalett. Merk at Emery-skjermen er
reflektiv, så fargene fremstår mer dempede på klokka (og i emulatoren) enn
hex-verdiene tilsier.

| Bruk                 | GColor                  |
|----------------------|--------------------------|
| Elbow øvre arm         | `GColorOrange`          |
| Elbow midtseksjon       | `GColorVividViolet`     |
| Elbow fot                | `GColorRoseVale`       |
| Datobar                   | `GColorChromeYellow`  |
| Etikett SENSORS            | `GColorRoseVale`     |
| Etikett VITALS              | `GColorLavenderIndigo` |
| Etikett SYSTEMS              | `GColorChromeYellow` |
| Alle datapaneler              | `GColorWhite`       |
| All tekst                      | `GColorBlack`      |

## Status

**Implementert og bygget (MVP):**
- Klokkeslett, oppdatert per minutt (`MINUTE_UNIT` — ikke sekund, av hensyn
  til batteri). Respekterer 12/24-timers systeminnstilling.
- Dato i toppbaren (ukedag, dag, måned).
- SYSTEMS-slot: batteriprosent og Bluetooth-status (`LINK` / `NO LINK`),
  oppdatert via `battery_state_service` og `connection_service`.

**Reservert, ikke implementert ennå:**
- SENSORS-slot: værdata via PebbleKit JS (Open-Meteo, ingen API-nøkkel).
  Krever `src/pkjs/index.js` og `messageKeys` i `package.json`.
- VITALS-slot: puls og/eller skritt via `HealthService`
  (`HealthMetricHeartRateBPM`, `HealthMetricStepCount`).
- Egen font (Antonio, SIL OFL) i stedet for systemfontene.
- Flere fargetema via Clay-innstillinger.
- Vibrasjon på hel time.

## Typografi

Bruker foreløpig Pebbles innebygde fonter: `BITHAM_42_BOLD` for klokkeslett,
`GOTHIC_18_BOLD` for dato, `GOTHIC_24_BOLD` for verdier og `GOTHIC_14` for
etiketter. Antonio (Google Fonts, SIL OFL) er et aktuelt bytte senere — se
`resources/README.md`.

## Bygg

```bash
cd watchfaces/lcars-readout
pebble build                      # -> build/lcars-readout.pbw
pebble install --emulator emery   # krever X-display
```

Sist bygde `.pbw` er sjekket inn under `dist/lcars-readout.pbw` og kan
installeres direkte på klokka via Pebble-telefonappen.

Bygget med `pebble-tool` 5.0.39 og Pebble SDK 4.17, target `emery`.
Minneforbruk: 2332 byte RAM, 4092 byte ressurser — god plass til utvidelser.

### Fallgruve på Linux uten IPv6

`pebble install --emulator` feiler med `[Errno 111] Connection refused` fordi
`pypkjs` binder websocket-serveren til IPv6. Fiks ved å endre
`pypkjs/runner/websocket.py` slik at `pywsgi.WSGIServer` binder `"0.0.0.0"` i
stedet for `""`. Må gjentas etter hver reinstallasjon av `pebble-tool`.
