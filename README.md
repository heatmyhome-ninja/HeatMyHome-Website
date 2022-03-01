# HeatMyHome.Ninja
www.heatmyhome.ninja is a web tool to allow consumers to simulate their home's heating for a range of heating systems.

# Table of Contents

- [Technologies Simulated](#technologies-simulated)
    - [Primary Heaters](#primary-heaters)
    - [Electrified Heating Technologies](#electrified-heating-technologies)
        - [Solar Ancillaries](solar-ancillaries)
        - [Thermal Energy Storage](thermal-energy-storage)

# Technologies Simulated

### Primary Heaters

The following heating systems are simulated:
1. Air Source Heat Pumps (ASHP)
2. Ground Source Heat Pumps (GSHP)
3. Electric Resistance Heating (ERH)
    *(also known as Electric Boilers or Direct Electrical heating (DEH))*
4. Hydrogen Boilers
5. Hydrogen Fuel Cells
6. Biomass Boilers
7. Natural Gas Boilers

### Electrified Heating Technologies
The electrified heating technologies (ASHPs, GSHPs and ERHs) are simulated with solar 
ancillaries and thermal energy storage.
These technologies are sized based on the peak hourly energy demand experience over the 
simulated year.

#### Solar Ancillaries

The following solar ancillaries (a mix of solar photovoltaic and solar thermal technologies)
 are simulated
1. Photovoltaic Panels (PV)
2. Evacuated Tube Solar Thermal Collectors (ET)
3. Flat Plate Solar Thermal Collectors (FP)
4. Photovoltaic-Thermal Hydrid Collectors

PV and ET, and PV and FP are also simulated in combination.
The solar ancillaries are sized from 2m<sup>2</sup> upto 1/4 the inputted floor area of 
the property (assume half the roof area), in 2m<sup>2</sup> increments.

#### Thermal Energy Storage

Water tanks are the only thermal energy storage technology currently simulated. The minimum
water tank volume is 0.1m<sup>3</sup> with a maximum of 3m<sup>3</sup>

# Core Simulation Stages


# Computational Complexity

