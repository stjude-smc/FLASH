# SM-FIRE (formerly Flash Gordon)

**Authors:** Daniel Terry, Manuel Juette, and Roman Kiselev
**Institution:** St. Jude Children's Research Hospital  
**Contact:** [scott.blanchard@stjude.org](mailto:scott.blanchard@stjude.org)  
**License:** See [`license.txt`](./license.txt)  
**GitHub:** [stjude-smc/SM-FIRE](https://github.com/stjude-smc/SM-FIRE)

---

## Overview

**SM-FIRE** is a high-performance instrument control and image acquisition platform for **single-molecule TIRF microscopy**. Built for precision, speed, and flexibility, it enables synchronized control of cameras, shutters, lasers, stages, and other devices — with seamless downstream compatibility with the SPARTAN analysis suite.

> **WARNING**  
> Flash Gordon does **not** include integrated laser safety mechanisms. Proper external safety systems and procedures are the responsibility of the user to maintain.

---

## Features

- Up to 4 synchronized cameras with synchronized acquisition
- Supports continuous, ALEX, and stroboscopic illumination
- Real-time Live Viewer with particle alignment and particle preview
- Auto-focus and staged scan automation
- Integrated support for laser power control and photobleaching routines
- Metadata-rich BigTIFF (.tif) output for analysis with [SPARTAN](https://github.com/stjude-smc/SPARTAN)
- Built-in configuration management for multi-instrument labs

---

## System Requirements (compiled version)

- **OS:** Windows 10/11 (64-bit)
- **Dependencies:** (depending on installed devices)
  - [LabVIEW Run-Time 2023-SP3](https://www.ni.com/en/support/downloads/software-products/download.labview-runtime.html#484336)
  - [DCAM](https://dcam-api.com/) / [PVCAM](https://www.photometrics.com/support/download/pvcam)
  - [NI DAQmx](https://www.ni.com/en-us/support/downloads/drivers/download.ni-daqmx.html#445931)
  - [Nikon Ti2 SDK](https://nisdk.recollective.com/microscopes)
  - [Thorlabs Kinesis](https://www.thorlabs.com/software_pages/ViewSoftwarePage.cfm?Code=Motion_Control)
  - Coherent, LaserQuantum, or Cobolt laser drivers (if applicable)

---

## Installation

1. Install all required drivers and runtimes listed above.
2. Download the [latest stable release](https://github.com/stjude-smc/SM-FIRE/releases).
2. Unzip the Flash Gordon package to `C:\FlashGordon`.
3. Create a desktop shortcut to `FlashGordon.exe`.
4. Power on and initialize all hardware devices.
5. Launch the application and customize the configuration to match your instrument.

---

## Quick Start

| Task                | How-To                                                                 |
|---------------------|------------------------------------------------------------------------|
| Launch Software     | Run `FlashGordon.exe`                                                  |
| Select Configuration| Choose your instrument setup from the list                             |
| Live View           | Click **Show Live** to preview images and align channels               |
| Start Acquisition   | Use **Stream Acquisition** to record movies to disk                    |

Movies are saved as **16-bit BigTIFF** stacks with aligned montage layout and structured metadata.

---

## Documentation

The full user manual is included in [`Flash Gordon documentation.pdf`](./Flash%20Gordon%20documentation.pdf), covering:

- Hardware setup and supported devices
- Configuration management
- User interface overview
- Acquisition modes and file formats
- Troubleshooting, limitations, and known issues

---

## Citation

If you use Flash Gordon in your research, please cite:

> Juette MF, Terry DS, Wasserman MR, et al.  
> *Single-molecule imaging of non-equilibrium molecular ensembles on the millisecond timescale.*  
> **Nature Methods**. 2016;13(4):341-344.  
> [doi:10.1038/nmeth.3769](https://doi.org/10.1038/nmeth.3769)

---

## Support & Contact

For support, bug reports, or feature requests, please contact:  
[scott.blanchard@stjude.org](mailto:scott.blanchard@stjude.org)

When reporting issues, include:

- Your institution, PI, and Flash Gordon version
- Your hardware setup and configuration file
- Any error messages or screenshots
- Details on reproduction steps

---

## Disclaimer

This software is **intended solely for academic research use**. Users are responsible for validating safety and compliance in their labs. Use of this software is at your own risk.

---