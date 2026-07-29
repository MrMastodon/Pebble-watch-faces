# Pebble-watch-faces — notater for Claude Code

Dette repoet samler *alle* Pebble-watchfaces brukeren lager, ikke bare ett
prosjekt. Hold konvensjonen nedenfor når du legger til nye watchfaces.

## Konvensjon

- Ett watchface = én mappe under `watchfaces/<kebab-case-navn>/`, og er et
  helt selvstendig Pebble SDK-prosjekt (eget `package.json`, `wscript`,
  `src/`, `resources/`).
- Target-plattform er som hovedregel `emery` (Pebble Time 2, 200×228 px,
  farge, avrundede hjørner). Vurder ekstra plattformer
  (`basalt`/`chalk`/`diorite`) per prosjekt ved behov for
  bakoverkompatibilitet.
- Ikke kopiér kode/CSS/assets fra andre sin Pebble-watchface-repoer uten å
  sjekke lisens først — flere populære referanseprosjekter (f.eks.
  `AKlitbo/pebble-watchfaces`) er lisensiert PolyForm Noncommercial og er
  *ikke* fritt gjenbrukbare i andre prosjekter. Bruk dem kun som visuell
  inspirasjon, ikke som kildekode.

## Byggverktøy

- Bygges med [`pebble-tool`](https://github.com/pebble-dev/pebble-tool)
  (Rebble-forket), SDK 4.17.
- Standard flyt: `pebble build`, deretter `pebble install --emulator emery`
  eller installasjon på fysisk klokke via Pebble-telefonappen.

### Kjente fallgruver ved oppsett (Linux)

- `pebble sdk install latest` kan rapportere feil-exit-kode selv om
  installasjonen faktisk lyktes ("already installed"-tilfellet) — sjekk
  faktisk SDK-status før du konkluderer med feil.
- Ufullstendige SDK-installasjoner (manglende toolchain-mappe) krever
  avinstallering + reinstallering.
- `pebble sdk activate <versjon>` skjer ikke alltid automatisk etter
  installasjon — kjør eksplisitt.
- Telefonsimulatoren (`pypkjs`) binder til IPv6 som standard, og
  `pebble install --emulator <platform>` feiler med "connection refused" på
  verter/kjerner uten IPv6. Må evt. patches til å binde `0.0.0.0`.
- Emulatoren krever et aktivt X-display — headless miljøer kan bygge
  (`pebble build`), men ikke starte emulatorvinduet.

## Valgfritt akselerasjonsverktøy

[`coredevices/pebble-watchface-agent-skill`](https://github.com/coredevices/pebble-watchface-agent-skill)
er en offisiell Claude Code-skill fra Core Devices (Pebble) for å generere
komplette watchface-prosjekter fra en tekstbeskrivelse, bygge med QEMU og ta
skjermbilder automatisk. Den er *ikke* vendoret inn i dette repoet (mangler
eksplisitt lisensfil) — klon den separat i et eget miljø med SDK/QEMU
installert hvis du vil bruke den til å generere eller bygge et watchface,
og kopiér kun det ferdige, egenproduserte resultatet inn i
`watchfaces/<navn>/` her.
