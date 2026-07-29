# Pebble-watch-faces

Samling av watchfaces for Pebble (primært testet mot Pebble Time 2 /
Emery-plattformen, 200×228 px farge), utviklet med Pebble SDK 4.x via
[`pebble-tool`](https://developer.rebble.io/developer.pebble.com/sdk/index.html)
(Rebble).

## Struktur

Hvert watchface bor i sin egen mappe under `watchfaces/`, som et
selvstendig, byggbart Pebble-prosjekt:

```
watchfaces/
  <kebab-case-navn>/
    PLAN.md            # design- og implementasjonsplan
    package.json        # Pebble-prosjektmanifest
    wscript              # build-script
    src/c/               # watchface-kode (C)
    src/pkjs/             # ev. PebbleKit JS (telefon-side, f.eks. vær)
    resources/            # fonter, bilder
```

## Watchfaces

- [`watchfaces/lcars-readout`](watchfaces/lcars-readout) — LCARS-inspirert
  urskive med lyse avlesningspaneler (svart tekst på lys bunn) for bedre
  lesbarhet enn klassisk LCARS lys-på-mørk.

## Bygg

Hvert prosjekt bygges med `pebble-tool`:

```bash
cd watchfaces/<navn>
pebble build
pebble install --emulator emery   # eller til fysisk klokke via telefon-appen
```

Se [developer.rebble.io](https://developer.rebble.io/) for installasjon av
SDK/`pebble-tool`, og `CLAUDE.md` i dette repoet for kjente fallgruver ved
oppsett av utviklingsmiljøet.
