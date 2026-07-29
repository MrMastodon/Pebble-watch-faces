# LCARS Readout — design- og implementasjonsplan

## Konsept

Et LCARS-inspirert (Star Trek) watchface for Pebble Time 2 (Emery,
200×228 px, farge, avrundede hjørner). Rammen rundt kanten beholder den
klassiske, fargerike, mørke LCARS-stilen (elbow-paneler, stolper), men alle
**avlesningsfelt** — klokkeslett, dato, batteri, Bluetooth-status — vises som
**svart tekst på lyse paneler** for bedre lesbarhet enn tradisjonell LCARS
(lys tekst på mørk bunn). Dette er en bevisst "delvis invertering": rammen
er mørk og fargerik som forbildet, innholdet er lyst og lesbart.

Inspirert av den generelle LCARS-designtradisjonen og layout-ideer fra
`AKlitbo/pebble-watchfaces` sitt "LCARS Stardate"-watchface — men koden her
er egenutviklet fra bunnen av, ikke kopiert (det prosjektet er lisensiert
PolyForm Noncommercial og ikke fritt gjenbrukbart).

## Layout (200×228 px)

```
┌──┬──────────────────────────┐
│▓▓│  DAG · DATO   (gull bar) │  <- toppstolpe, ~20px høy, svart tekst
├──┤                          │
│▓▓│  ┌────────────────────┐  │
│▓▓│  │                    │  │
│▓▓│  │      12:34         │  │  <- hovedpanel: lyst, stor svart tekst
│▓▓│  │                    │  │
│░░│  └────────────────────┘  │
│░░│  ┌────────┬───────────┐  │
│░░│  │ BATT   │ BT        │  │  <- to mindre lyse paneler i rad
│░░│  │ 82%    │ ●         │  │     m/ fargede LCARS-etiketter over
│░░│  └────────┴───────────┘  │
└──┴──────────────────────────┘
  ^ venstre "elbow"-kolonne, ~40px bred
    ▓▓ = oransje (øvre halvdel)
    ░░ = fiolett-blå (nedre halvdel)
```

- **Venstre kolonne** (~40 px bred, full høyde): klassisk LCARS elbow —
  avrundet øvre venstre hjørne som kurver ned i toppstolpen. Stables i to
  fargeblokker: oransje øverst (~40 % av høyden), fiolett-blå resten.
- **Toppstolpe** (~20 px høy, fra elbow og bortover): `GColorChromeYellow`
  (gull/rav), med liten svart tekst for ukedag + dato (f.eks. "MON 29 JUL").
- **Hovedpanel** (klokkeslett): lyst panel (`GColorWhite`), avrundede
  hjørner, stor fet svart tekst, sentrert i den øvre halvdelen av
  hovedfeltet.
- **Databaner** (batteri, Bluetooth): to mindre lyse paneler
  (`GColorLightGray` eller hvit) side ved side under klokkeslettet, hver med
  en smal fargestolpe-etikett (rødlig rose / oransje) med svart tekst rett
  over ("BATT", "BT"), og selve verdien i svart tekst i det lyse feltet
  under.
- **Høyre/nedre kant**: valgfrie små dekorative fargeblokker ("sp"-elementer
  i LCARS-stil) for å ramme inn hovedpanelet visuelt, i samme palett som
  elbow-kolonnen.

## Fargepalett (Pebble `GColor`, alle støttet på Emery)

| Bruk                          | Farge                                    |
|-------------------------------|-------------------------------------------|
| Elbow øvre blokk               | `GColorOrange`                            |
| Elbow nedre blokk               | `GColorVividViolet` (evt. `GColorLavenderIndigo`) |
| Toppstolpe (dato)               | `GColorChromeYellow`                      |
| Etikett-stolper (BATT/BT)        | `GColorRoseVale`                          |
| Avlesningspaneler (bakgrunn)      | `GColorWhite`                             |
| Sekundærpaneler (bakgrunn)         | `GColorLightGray`                       |
| All tekst i lyse paneler            | `GColorBlack`                          |
| All tekst i fargede stolper/etiketter | `GColorBlack`                       |

## Typografi

- Forslag: **Antonio** (Google Fonts, SIL Open Font License), kondensert
  sans-serif som gir en god LCARS-følelse. Lastes ned separat under
  implementasjon og legges i `resources/fonts/antonio-bold.ttf`, bygges inn
  via `package.json` sin `media`-liste.
- Fallback uten custom font (raskere å komme i gang med): Pebbles innebygde
  `FONT_KEY_BITHAM_42_BOLD` for klokkeslett og `FONT_KEY_GOTHIC_18_BOLD` /
  `FONT_KEY_GOTHIC_14` for øvrig tekst.

## Funksjonsomfang

**MVP (denne mappen sitt kodeskjelett dekker strukturen for):**
- Klokkeslett (time:minutt, oppdateres hvert minutt via
  `tick_timer_service_subscribe(MINUTE_UNIT, ...)` — ikke sekund, for
  batterisparing).
- Dato (ukedag + dag + måned).
- Batteriprosent (`battery_state_service_subscribe`).
- Bluetooth-tilkoblingsstatus (`connection_service_subscribe`).

**Fase 2 (senere, ikke del av skjelettet nå):**
- Værdata via PebbleKit JS (Open-Meteo, uten API-nøkkel — som i forbildet).
- Flere fargetema-varianter (f.eks. "Classic" / "Voyager" / "Mono") valgbare
  via Clay-innstillinger.
- Vibrasjon på hel time (valgfri innstilling).
- Skrittdata / helsedata fra Pebble Health API.

## Teknisk tilnærming

- Ren C, ingen bitmap-ressurser for selve LCARS-formene — alt tegnes med
  `Layer` + `layer_set_update_proc` og `graphics_fill_rect(...,
  GCornerMask, radius)` for avrundede paneler. Holder appen lett og gjør
  fargejustering enkelt (kun konstanter i `lcars_theme.h`).
- `TextLayer` for all tekst, plassert oppå de tegnede panelene.
- Layout-mål og farger samles i `src/c/lcars_theme.h` slik at hele
  fargepaletten/layouten kan justeres ett sted.

## Byggverktøy

- [`pebble-tool`](https://github.com/pebble-dev/pebble-tool) (Rebble),
  SDK 4.17.
- Target-plattform: `emery` (Pebble Time 2). Vurder å legge til
  `basalt`/`chalk`/`diorite` i `targetPlatforms` senere for
  bakoverkompatibilitet — krever da fallback-layout for mindre skjermer.

## Verifisering (i et miljø med SDK installert)

1. `cd watchfaces/lcars-readout && pebble build` — bekreft at prosjektet
   kompilerer uten feil.
2. `pebble install --emulator emery` — visuell sjekk i emulator (krever
   X-display).
3. Sjekk kontrast/lesbarhet: klokkeslett og databaner skal være lett
   leselige i vanlig innendørsbelysning og direkte sollys (Pebble Time 2 har
   reflekterende LCD, så mørk tekst på lys bunn bør faktisk gi bedre
   utendørslesbarhet enn originalens lys-på-mørk).
4. Installer på fysisk Pebble Time 2 og sammenlign side om side med
   "LCARS Stardate" for å bekrefte at det oppleves mer lesbart, som var
   hele motivasjonen for dette watchfacet.
