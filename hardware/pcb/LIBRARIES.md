# Project libraries

Open `pcb.kicad_pro`. Keep both schematic sheets, the PCB, the project library tables, and the local libraries together.

The board and both schematic sheets were copied unchanged from `ECE362PCBfiles (2).zip`. The ZIP did not include standalone custom libraries. `RPI_DB.kicad_sym`, `RPI - Power.kicad_sym`, and `RPI - BOM Directives.kicad_sym` were recovered from the schematic's embedded symbol definitions (23 symbols total). `RPI_COMPILED.pretty` contains the 13 footprint types recovered from the 47 RPI_COMPILED instances on the routed PCB. These are the project-used definitions, not the complete original Raspberry Pi libraries. For types with multiple instances, the first PCB instance supplies the library copy; instance properties and keepout coordinates can differ on the board. No board instance was updated from these libraries.

Existing TFT symbol, footprint, and STEP assets are retained. Both `myparts` (used by the imported project) and the existing `z_Devices` footprint aliases resolve to `TFT-320x240.pretty`. Local library paths use `${KIPRJMOD}`. The TFT library footprint's model path also uses `${KIPRJMOD}`. Project text variable `KICAD6_USER_3DMOD_DIR=${KIPRJMOD}` resolves the routed board's historical TFT model path without editing the PCB.

The ZIP references Raspberry Pi 3D models using `kicad-embed://` URLs, but contains no embedded model payloads or corresponding standalone STEP files. Those optional 3D models remain unavailable; symbol and footprint geometry is present. Original Raspberry Pi `file:///C:/rpi-pdb/...` datasheet links also remain as provenance, with no matching local datasheets supplied.

## Preserved schematic/PCB differences

J8 is a 10-pin MPU-9250 breakout. J7 is a four-signal microSD header. Their GPIO labels agree with `firmware/include/pins.h`, but the imported netlist leaves J8 GPIO5/6, J7 GPIO0/18/19/22, and D-pad GPIO31 disconnected from the MCU. J7 and J8 have no assigned footprints and are absent from the routed PCB. These source discrepancies are intentionally preserved. The existing TFT library also differs from its embedded schematic copy, producing a KiCad library-symbol mismatch warning.

Do not run Update PCB from Schematic as part of this import. The routing and schematic connectivity have not been changed.
