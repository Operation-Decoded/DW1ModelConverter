# DW1ModelConverter

A tool that converts Digimon models from the game "Digimon World 1" into gltf files, complete with texture and animation.

This is a specialist tool and users are expected to have at least an idea of what they're doing. It is not supposed to be user friendly but to get the job done.

# Usage

Run the tool in a command line like this:

```
DW1ModelConverter <pathToGameFiles>
```

The path must point to a folder containing the contents of the ISO. This can be obtained in a number of ways, for example:

* put the original CD in a CD drive and either pass the path to the drive, or copy the contents into a folder of your choice
  * for CD images, like ISO, you can mount them to a virtual CD drive and do the same
* extract the ISO using tools like [dumpsxiso](https://github.com/Lameguy64/mkpsxiso/releases)

Any PSX release of the game *should* be supported, but so far only US, JP (Version 1.1) and JP (BonBon) have been tested.

The tool will extract all Digimon models into an `output` folder created in the current working directory.

## Output Caveats
Not every property of the original TMD files could be translated properly into gltf. As much as possible of that information has been placed into the "extras" fields.

This includes:
- animation endless loop start time
- animation sound effects
- texture animations (e.g. blinking)
- TMD translucency blend modes

Animations aren't named yet, that may change in future versions.

### Name Mapping
The created files will use the internal file names of the game. To find the Digimon you want, use the following mapping:

Duplicates marked with (NPC) are typically versions of a model with fewer animations, intended to save on memory.

```
BOYS -> Player Character
BOTA -> Botamon
KORO -> Koromon
AGUM -> Agumon
BETA -> Betamon
GREY -> Greymon
DEVI -> Devimon
AIRD -> Airdramon
TYRA -> Tyrannomon
MERA -> Meramon
SEAD -> Seadramon
NUME -> Numemon
MTGR -> MetalGreymon
MAME -> Mamemon
MONZ -> Monzaemon
PUNI -> Punimon
TUNO -> Tsunomon
GABU -> Gabumon
ELEC -> Elecmon
KABU -> Kabuterimon
ANGE -> Angemon
BIRD -> Bidramon
GARU -> Garurumon
YUKI -> Frigimon
HOEE -> Whamon
VEGI -> Vegiemon
SKUL -> SkullGreymon
MTMA -> MetalMamemon
VEDA -> Vademon
POYO -> Poyomon
TOKO -> Tokomon
PATA -> Patamon
KUNE -> Kunemon
UNIM -> Unimon
OGRE -> Ogremon
SHEL -> Shellmon
CENT -> Centarumon
BAKE -> Bakemon
DORI -> Drimogemon
SCUM -> Sukamon
ANDR -> Andromon
GIRO -> Giromon
ETEM -> Etemon
YURA -> Yuramon
TANE -> Tanemon
PIYO -> Biyomon
PALM -> Palmon
MONO -> Monochromon
LEOM -> Leomon
SIRA -> Coelamon
COCA -> Kokatorimon
KUWA -> Kuwagamon
MOJA -> Mojyamon
NANI -> Nanimon
MGDR -> Megadramon
PICC -> Piximon
DIGI -> Digitamamon
PENM -> Penguinmon
IGAM -> Ninjamon
HOUO -> Phoenixmon
HKAB -> H-Kabuterimon
MGSD -> MegaSeadramon
PANJ -> Panjyamon
GGDR -> Gigadramon
MTET -> MetalEtemon
VAND -> Myotismon
YANM -> Yanmamon
GOTU -> Gotsumon
FLAR -> Flarizamon
WARU -> WaruMonzaemon
YKAG -> SnowAgumon
HYOG -> Hyogamon
PCSC -> PlatinumSukamon
DOKU -> Dokunemon
SIMA -> ShimaUnimon
TANK -> Tankmon
REDV -> RedVegiemon
JMOJ -> J-Mojyamon
NISE -> NiseDrimogemon
GOBR -> Goburimon
TUTI -> MudFrigimon
PSYC -> Psychemon
MODO -> ModokiBetamon
TOYA -> ToyAgumon
PIDD -> Piddomon
ARUR -> Aruramon
GERE -> Geremon
VARM -> Vermillimon
FUGA -> Fugamon
TKKA -> Tekkamon
MRIS -> MoriShellmon
GARD -> Guardromon
MCHO -> Muchomon
ICEM -> Icemon
AKAT -> Akatorimon
TUKA -> Tsukaimon
SHAM -> Sharmamon
CLEA -> ClearAgumon
ZASS -> Weedmon
ICDV -> IceDevimon
DKRZ -> Darkrizamon
SNDY -> SandYanmamon
SNGB -> SnowGoburimon
BLMR -> BlueMeramon
GRUR -> Gururumon
SABD -> Saberdramon
SOUL -> Souldmon
GOLE -> Rockmon
OTAM -> Otamamon
GECO -> Gekomon
TENT -> Tentomon
WRSE -> WaruSeadramon
INSE -> Meteormon
MUGE -> Machinedramon
ANLG -> Analogman
JIJI -> Jijimon
TENS -> Market Manager
TONO -> ShogunGekomon
SCUD -> King Sukamon
JURE -> Cherrymon
HAGU -> Hagurumon
BRIK -> Tinmon
TIRS -> Master Tyrannomon
EGOB -> Goburimon (NPC)
BRAK -> Brachiomon
PUTI -> DemoMeramon
EBET -> Betamon (NPC)
EGRE -> Greymon (NPC)
EDEV -> Devimon (NPC)
EAIR -> Airdramon (NPC)
ETYR -> Tyrannomon (NPC)
EMER -> Meramon (NPC)
ESEA -> Seadramon (NPC)
ENUM -> Numemon (NPC)
EMTG -> MetalGreymon (NPC)
EMAM -> Mamemon (NPC)
EMON -> Monzaemon (NPC)
EGAB -> Gabumon (NPC)
EELE -> Elecmon (NPC)
EKAB -> Kabuterimon (NPC)
EANG -> Angemon (NPC)
EBIR -> Bidramon (NPC)
EGAR -> Garurumon (NPC)
EYUK -> Frigimon (NPC)
EHOE -> Whamon (NPC)
EVEG -> Vegiemon (NPC)
ESKU -> SkullGreymon (NPC)
EMTM -> MetalMamemon (NPC) ("MetalGreymon")
EVED -> Vademon (NPC)
EPAT -> Patamon (NPC)
EKUN -> Kunemon (NPC)
EUNI -> Unimon (NPC)
EOGR -> Ogremon (NPC)
ESHE -> Shellmon (NPC)
ECEN -> Centarumon (NPC)
EBAK -> Bakemon (NPC)
EDOR -> Drimogemon (NPC)
ESCU -> Sukamon (NPC)
EAND -> Andromon (NPC)
EGIR -> Giromon (NPC)
EETE -> Etemon (NPC)
EPIY -> Biyomon (NPC)
EPAL -> Palmon (NPC)
EMNO -> Monochromon (NPC)
ELEO -> Leomon (NPC)
ESIR -> Coelamon (NPC)
ECOC -> Kokatorimon (NPC)
EKUW -> Kuwagamon (NPC)
EMOJ -> Mojyamon (NPC)
ENAN -> Nanimon (NPC)
EMGD -> Megadramon (NPC)
EPIC -> Piximon (NPC)
EDIG -> Digitamamon (NPC)
EIGA -> Ninjamon (NPC)
EPEN -> Penguinmon (NPC)
EVAN -> Myotismon (NPC)
CEGR -> Greymon (NPC)
CEMG -> MetalGreymon (NPC)
```

# Building

This project uses CMake in combination CPM.cmake for dependency management.

Building the project should be a simple

```
$ git clone git@github.com:Operation-Decoded/DW1ModelConverter.git
$ cd <project dir>
$ cmake . -DCMAKE_BUILD_TYPE=Release
$ cmake --build . --config Release
```

Or you just open the folder with a CMake enabled IDE like VS Code.

# Contact

* Discord: SydMontague, or in either the [Digimon Modding Community](https://discord.gg/cb5AuxU6su) or [Digimon Discord Community](https://discord.gg/0VODO3ww0zghqOCO)
* directly on GitHub
* E-Mail: sydmontague@web.de
* Reddit: [/u/Sydmontague](https://reddit.com/u/sydmontague)
* if you find a SydMontague somewhere else chances are high that's me, too. ;)