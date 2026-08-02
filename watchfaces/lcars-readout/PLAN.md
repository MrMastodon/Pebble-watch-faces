# LCARS Readout — design og status

![](screenshot.png)

Tatt rett fra emulatoren, med ekte vær hentet fra Open-Meteo. Puls og skritt
står som `--`/`0` fordi emulatoren ikke har helsedata; alt annet er live.

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
`resources/images/LCARS-readout_background_7.png`. Koden tegner bare verdiene
oppå. Alle koordinater i `src/c/lcars_theme.h` er målt mot det bildet, så de
må måles om hvis illustrasjonen endres.

| Felt | Posisjon |
|---|---|
| Klokkeslett | x 52–198, y 16–72 (Antonio 58) |
| Dato | x 52–198, y 109–141 (Antonio 30) |
| Batteri | x 0–50, y 182–197 (Antonio 16) |
| Værikon | x 55, y 160, 17×17 |
| Værtilstand | x 74–121, y 159–174 |
| Temperatur | x 74–121, y 200–216 |
| Puls | x 147–198, y 159–174 |
| Skritt | x 147–198, y 200–216 |
| LINK (BT) | x 0–50, y 4–19 (Antonio 16) |

Klokkeslett og dato er sentrert ved å måle faktisk avstand fra sifrene til
nærmeste grafikk over og under, ikke mot hullet i illustrasjonen: klokka står
5 px fra begge, datoen 6 over og 5 under (mellomrommet er 11 px, så det lar
seg ikke dele likt).

Batteriet ligger i den **røde** blokka (y 174–200), ikke den nederste — det er
langt lettere å lese mot den fargen. De to venstre verdiene deler x74 og de to
høyre deler x147, så kolonnene står i flukt selv om ikonene har ulik bredde.

Venstre kolonne slutter på x122 (høyre begynner på x127). Det gir værtilstand
og temperatur 48 px hver — `CLEAR` er den bredeste teksten og tegnes 47 px, så
marginen er 1 px. Nye forkortelser må derfor måles, ikke antas.

Værikonet er det eneste ikonet som tegnes i kode, siden det er det eneste som
bytter. Det er 17×17 for å matche hjertet (17×15) og fotsporene (17×16) i
illustrasjonen, og y-en settes slik at det står sentrert på hjertet. Når
ikonene i illustrasjonen flytter seg, følger derfor værikonet etter — mens
verdiene blir stående. Ikonrekka og tekstrekka er to uavhengige linjer.

Skillestreken mellom vær- og temperaturraden (y 185–190) er ren dekor. Den er
tegnet smalere enn kolonnen (x 55–119), så kolonnegrensen leses av
`SENSORS`-raden (endekapsel x 116–122), ikke av streken.

Bakgrunnen klargjøres av `tools/prep_background.py`, som selv plukker den
høyest nummererte `LCARS-readout_background*.png`. Alfa flates ut mot hvitt,
og hver piksel snappes til Pebble-64. Deretter skilles designfarger fra
antialiasing-frynser, slik at fargetallet holder seg under 16 og bitmapen kan
lagres som `4BitPalette` — 22 KB heap i stedet for 45 KB.

Skillet gjøres på **lengste sammenhengende stripe**, ikke på pikselantall
eller tetthet. Begge de enklere målene tar feil her: en liten solid blokk kan
være sjeldnere enn en frynse, og pills og endekapsler er solide men spredt
over hele flaten, så de ser like tynne ut som støy målt på omriss. Stripelengde
stiller spørsmålet som faktisk betyr noe — fyller fargen en solid strekning
noe sted, eller er den alltid en tynn kant?

Ekte gråtoner (`#555555`, `#AAAAAA`) håndteres for seg og løses mot svart
eller hvitt etter lyshet. Nærmeste-farge ville sendt dem et kromatisk sted:
mellomgrå ligger bare 85 fra `#005555` men 255 fra svart målt per kanal.

## Palett

Illustrasjonen er tegnet direkte på Pebble-paletten, så den overlever
konverteringen nesten intakt — bare 2,6 % av pikslene flytter seg, og det er
utelukkende antialiasing langs kanter. Sluttresultatet er 13 farger: åtte
designfarger pluss svart og hvit, og tre frynsefarger som er små nok til å
ikke telle visuelt.

| Element | Farge | Pebble-navn |
|---|---|---|
| Elbow | `#AA55AA` | Purpureus |
| Oransje blokker | `#FFAA55` | Rajah |
| Lyse barer | `#AAAAFF` | BabyBlueEyes |
| Pills og endekapsler | `#5555AA` | Liberty |
| Rød blokk | `#FF5555` | SunsetOrange |
| Rød aksent | `#FF0000` | Red |
| Rosa segment | `#FF55AA` | BrilliantRose |
| Lyse kapsler | `#FFAAFF` | RichBrilliantLavender |

Til sammenligning traff den første illustrasjonen ingen av palettfargene, og
33 % av pikslene flyttet seg — de lyse barene forsvant helt mot hvit bakgrunn.

## Typografi

**Antonio** (Google Fonts, SIL Open Font License — `resources/fonts/OFL.txt`),
i fire størrelser. Den kondenserte formen er det som gjør at `22.06.2026` i det
hele tatt får plass i en 146 px kolonne. Hver størrelse er begrenset med
`characterRegex` til akkurat de tegnene den bruker, så alle fire til sammen
veier lite.

| Ressurs | Bruk | Tegn |
|---|---|---|
| `FONT_ANTONIO_58` | klokkeslett | `[0-9:]` |
| `FONT_ANTONIO_30` | dato | `[0-9.-]` |
| `FONT_ANTONIO_22` | avlesningsverdier | `[0-9A-Z%°.:?-]` |
| `FONT_ANTONIO_16` | batteriprosent, LINK | `[0-9%LINK]` |

## Ikoner

Ni værikoner ligger som ressurser — termometer, hjerte og fotspor er malt inn
i bakgrunnen, siden de aldri endrer seg. Alle er 17×17 px sort-på-transparent
fra **Material Symbols** (Google, **Apache-2.0** — lisenstekst i
`resources/images/LICENSE-material-symbols.txt`).

Klart vær og delvis skyet har egne nattvarianter (måne i stedet for sol);
resten ser like ut etter mørkets frembrudd, så en måne ville ikke tilført noe.
Telefonen sender `is_day` fra Open-Meteo, som styrer valget.

Kilde-SVG-ene ligger i `resources/images/src/`, og PNG-ene bygges av
`tools/make_icons.py`.

## Datakilder

| Felt | Kilde | Status |
|---|---|---|
| Tid, dato | `tick_timer_service` (MINUTE_UNIT) | live |
| Batteri | `battery_state_service` | live |
| Puls, skritt | `HealthService` | live på klokka, tomt i emulator |
| Vær | `src/pkjs/index.js` → Open-Meteo | live, verifisert i emulator |

Bluetooth vises som `LINK` i den **øverste** blokka når telefonappen er
tilkoblet, og blokka står helt tom når den ikke er det — et brutt samband
leser da som et fravær i stedet for enda en etikett å tolke.

Ved brudd vibrerer klokka, hvis innstillingen tillater det. Den utløses bare
på overgangen tilkoblet → frakoblet, og `s_connected` seedes fra
`connection_service_peek_pebble_app_connection()` *før* abonnementet, slik at
urskiva ikke vibrerer når den lastes mens telefonen allerede er utenfor
rekkevidde.

Første forsøk brukte 60 ms, og brukeren merket ingenting. Antagelsen om at
motoren trengte tid på å komme opp i turtall gjaldt en roterende ubalansert
masse; Time 2 bruker en **lineær resonansaktuator**, som når full amplitude
langt raskere. 60 ms er derfor ikke umulig i prinsippet, men det gir bare noen
titalls millisekunder på full styrke — lett å gå glipp av på en løs reim.
`kort` er nå 150 ms og `lang` bruker `vibes_short_pulse()`, så brukeren kan
skru opp selv i stedet for at vi gjetter én riktig verdi.

`buzz()` er det ene stedet som avgjør om klokka får vibrere, så både
BT-varselet og timesignalet arver Quiet Time-regelen SDK-en ber om.

Det er ingen debounce: et kort brudd som straks kobler seg opp igjen gir en
vibrering. Blir det støyende i praksis er en AppTimer-debounce en liten
tilleggsendring.

## Innstillinger

Panelet er bygget med **Clay** (`pebble-clay`, `src/pkjs/config.js`), som
krever `enableMultiJS` — den var allerede på. Fire innstillinger:

| Innstilling | Valg | Virker |
|---|---|---|
| Datoformat | `31.12.2026` / `12.31.2026` / `2026-12-31` | klokka bytter `strftime`-mønster |
| Temperatur | Celsius / Fahrenheit | telefonen ber Open-Meteo om enheten |
| Vibrering ved brudd | av / kort / lang | `buzz()` på klokka |
| Timesignal | av / på | sjekk på `tm_min == 0` i tick-handleren |

Alle fire persisteres på klokka (`PKEY_DATE_FORMAT` og utover), så de
overlever en omstart uten å vente på telefonen. Clay sender select-verdier som
strenger og toggles som byte, så mottakssiden godtar begge former i stedet for
å anta én.

Temperaturen konverteres av Open-Meteo via `temperature_unit` i URL-en, ikke på
klokka — ett felt i stedet for regnestykke og avrundingsregel i C. Enheten
sendes med som egen nøkkel så klokka vet hvilken suffiks den skal skrive.

Timesignalet trenger ikke eget abonnement: `tick_handler()` kjører allerede på
`MINUTE_UNIT`.

Værhentingen bruker Open-Meteo, som ikke krever API-nøkkel, og henter hver
30. minutt. Siste måling lagres med `persist_write_int`, så den overlever en
omstart av urskiven i stedet for å blanke ut.

Værtilstanden vises som en forkortelse på maks fem tegn ved siden av ikonet.
Elleve tilstander skilles: `CLEAR`, `PTCLD`, `CLDY`, `FOG`, `DRIZL`, `RAIN`,
`FZRN`, `SNOW`, `SHWR`, `SNSH`, `STRM`. Ikonet er signalet man leser i et
øyekast; teksten er det som faktisk skiller yr fra underkjølt regn.

Forkortelsene er valgt etter **målt bredde, ikke tegnantall** — `W` er nesten
dobbelt så bred som `I` i Antonio, så `SHWRS` (53 px) og `SNSHW` (54 px) ble
avkuttet mens fem-tegns `CLEAR` (46 px) står fint. `tools/measure_text.py`
måler en kandidat mot 48 px-budsjettet før den tas i bruk.

WMO-kodene fra Open-Meteo mappes i `src/pkjs/index.js`, og
`tools/test_conditions.js` går gjennom alle 28 dokumenterte koder pluss
udokumenterte verdier. Mappingen er et dusin tallintervaller, altså akkurat
den formen der en av-med-én feiler stille: den krasjer ikke, den viser bare
regn når det snør.

Feilene er synlige på klokka i stedet for å se ut som «ingen data ennå»:
`GPS?` betyr at posisjon ikke kunne hentes, `NET?` at Open-Meteo ikke svarte,
og `--` at ingenting har kommet inn ennå. Feiltilstander lagres ikke, så de
kommer ikke tilbake etter en omstart og utgir seg for å være dagens vær.

Telefonsiden prøver på nytt tre ganger med ett minutts mellomrom før den gir
opp, og husker siste posisjon i `localStorage` — en halvtimegammel posisjon er
mer enn presis nok for vær, og slår å stå uten. Tidsavbruddet for posisjon er
30 s, siden en kald GPS-fiks innendørs sjelden rekker 15.

Været er nå verifisert ende-til-ende i emulatoren: `pypkjs` kjører
telefonkoden, henter posisjon og svar fra Open-Meteo, og verdiene lander på
skjermen. Puls og skritt er fortsatt tomme der, siden emulatoren ikke har
helsedata.

## Robusthet

Gjennomgått for stabilitet og sikkerhet. Det som ble funnet og fikset:

**Manglende bakgrunn tok ned urskiva.** `graphics_draw_bitmap_in_rect()` med
en `NULL`-bitmap krasjer — verifisert ved å tvinge `s_background` til `NULL`
og se at emulatoren sluttet å svare, mot en kontroll med ekte bakgrunn som
virket. Bakgrunnen er den største allokeringen på heapen (~22 KB) og dermed
den som først feiler under press. Nå tegnes hvit bunn i stedet, så verdiene
er fortsatt lesbare. Fontene har tilsvarende fallback til systemfonter.

**Urimelig temperatur tømte feltet.** `°` er to byte i UTF-8, så en sekssifret
verdi ble avkuttet midt i tegnet og etterlot ugyldig UTF-8 — som renderen
dropper stille, slik at feltet ble stående tomt. Verdien klemmes nå til
−99…199. Grensen er satt etter hva som faktisk rendres, ikke etter `int32`:
`100°C` er 48 px av 49, mens `999°C` flyter over, fordi `1` er en smal glyf i
Antonio og `9` ikke er det.

**`layer_mark_dirty()` uten `NULL`-sjekk** i tre av fem tilbakekall. Vanskelig
å nå i praksis, men inkonsekvent med de to som sjekket. Nå sjekker alle.

**AppMessage-innboksen** er hevet fra 128 til 256 byte. 128 holdt til dagens
meldinger, men Clay sender alle innstillingene i én ordbok, så et større
panel ville begynt å droppe meldinger stille i stedet for å feile høylytt.
`app_message_deregister_callbacks()` er lagt til i `deinit()`.

Sikkerhetsmessig er angrepsflaten liten: URL-en bygges av tall som er
`parseFloat`-et og `isNaN`-sjekket, så ingen injeksjon; trafikken går over
HTTPS; det finnes ingen API-nøkkel å lekke. Innstillinger fra telefonen
`atoi`-es og faller tilbake til trygge standardverdier ved søppel. Posisjonen
lagres i `localStorage` på telefonen og sendes kun til Open-Meteo — det er
iboende i det å ha vær i det hele tatt, men verdt å vite.

## Bygg

```bash
cd watchfaces/lcars-readout
pebble build                      # -> build/lcars-readout.pbw
pebble install --emulator emery   # krever X-display
```

Sist bygde `.pbw` ligger i `dist/lcars-readout.pbw` og kan installeres direkte
på klokka via Pebble-telefonappen.

Bygget med `pebble-tool` 5.0.39 og Pebble SDK 4.17, target `emery`.
Ressurser 14 444 B / 256 KB, statisk RAM 4 508 B / 128 KB
(pluss ~22 KB heap for bakgrunnsbitmapen).

### Fallgruve: `enableMultiJS` og navnet på JS-bunten

Uten `"enableMultiJS": true` i `package.json` — og et `wscript` som sender
`js_entry_file` til `pbl_bundle` — havner telefonkoden i `.pbw`-en som
`src/pkjs/index.js`. Telefonen ser etter `pebble-js-app.js`, finner den ikke,
og **kjører aldri JS-en i det hele tatt**. Alt annet fungerer, så symptomet er
at bare vær mangler mens klokke, batteri og helse er i orden.

Byggeloggen sier det rett ut — `enableMultiJS is not enabled for this project
and pebble-js-app.js does not exist` — men den drukner blant andre advarsler.
Sjekk `unzip -l build/*.pbw`: ligger det en `pebble-js-app.js` der, kjører
JS-en; ligger det en `index.js`, gjør den det ikke.

`wscript` må være SDK-ens JS-variant (`pebble new-project --javascript`), som
bruker `pbl_build(..., bin_type='app')` og `js_entry_file='src/pkjs/index.js'`.

### Fallgruve: `characterRegex` og waf-cachen

Endrer du `characterRegex` på en font i `package.json`, ser ikke waf det som
en grunn til å bygge fonten på nytt — TTF-en er jo uendret. Bygget lykkes, men
med den gamle glyffsamlingen, og tegn du nettopp la til rendres som ingenting.
Kjør `rm -rf build` etter slike endringer. Her gikk ressursbudsjettet fra
13 688 til 13 797 B først etter et rent bygg, som var beviset på at fire nye
glyffer faktisk kom med.

### Fallgruve på Linux uten IPv6

`pebble install --emulator` feiler med `[Errno 111] Connection refused` fordi
`pypkjs` binder websocket-serveren til IPv6. Fiks ved å endre
`pywsgi.WSGIServer(("", self.port), ...)` til `("0.0.0.0", self.port)` i
`pypkjs/runner/websocket.py`. Må gjentas etter hver reinstallasjon av
`pebble-tool`.
