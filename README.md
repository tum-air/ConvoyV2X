# ConvoyV2X Simulation Framework

An open source framework for the simulation of moving-network-convoy based cellular V2X communication architectures in intelligent transportation systems equipped with real-time sensing and detection of traffic participants. The convoy-based approach, briefly illustrated below, provides an alternate cellular deployment architecture taking into consideration the increased mobility and the ever-changing nature of network topology seen in V2X scenarios.

![moving_convoy_architecture](https://github.com/tum-air/ConvoyV2X/assets/128901542/e5351b9e-65b1-4368-ad23-54e6adb1bb64)

Conceptual information and initial evaluations about the potential benefits of such an architecture can be found in [this](https://doi.org/10.1109/VTC2020-Spring48590.2020.9128410) paper.

## Installation
The framework uses the [OMNeT++](https://omnetpp.org) discrete event simulator, the [INET](https://github.com/inet-framework/inet) networking framework, the [VEINS](https://veins.car2x.org) V2X framework, the [Simu5G](https://github.com/Unipisa/Simu5G) LTE and 5G-NR library, and the [SUMO](https://eclipse.dev/sumo/) mobility simulator. In order to implement the moving-network-convoy architecture, the Simu5G library has been extended to support convoy broadcast and control messages.

Please follow the installation instructions for the OMNeT++ discrete event simulator provided [here](https://omnetpp.org/download/), and subsequently import the folders `/convoy-architecture` `/inet4.4` `/simu5G` `/veins-veins-5.2` and `/veins_inet` as separate projects into the omnet++ workspace as shown below. Please also install the SUMO library, for which the instructions can be found [here](https://sumo.dlr.de/docs/Downloads.php). The framework has been tested for OMNeT++ v6.0 and SUMO v1.11.0

![omnetpp_workspace_edit](https://github.com/tum-air/ConvoyV2X/assets/128901542/351ab53f-c73b-4b58-8add-bd84ebf61ce9)

## Simulation Runs
The executable simulation runs are available in the folder `/convoy-architecture/simulations`. The scenario configuration files and the corresponding documentation are currently being updated and will be uploaded shortly.

<img width="1043" alt="omnetpp_workspace_run" src="https://github.com/tum-air/ConvoyV2X/assets/128901542/0ed4b895-6082-4e89-b0e8-dd9e5110ad42">

## License
The usage of [OMNeT++](https://omnetpp.org), [Simu5G](https://github.com/Unipisa/Simu5G), [INET](https://github.com/inet-framework/inet), [VEINS](https://veins.car2x.org), and [SUMO](https://eclipse.dev/sumo/) libraries are governed by their licensing terms. Users are requested to read the terms and assure conformance before using these libraries.

The remaining contents of this repository outside of the above library dependencies are provided for use under the GNU General Public License v3.0.

## Disclaimer
While the authors have taken utmost care to provide a reliable and accurate simulation framework, the software is provided "as is" and without any assurances. The authors are also not liable for any loss, expense or damage of any type that may arise by using this software.
