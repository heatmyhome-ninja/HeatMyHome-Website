# HeatMyHome.Ninja
www.heatmyhome.ninja is a web tool to allow consumers to simulate their home's heating for a range of heating systems.

# Table of Contents

- [Technologies Simulated](#technologies-simulated)
    - [Primary Heaters](#primary-heaters)
    - [Electrified Heating Technologies](#electrified-heating-technologies)
        - [Solar Ancillaries](#solar-ancillaries)
        - [Thermal Energy Storage](#thermal-energy-storage)
        - [Tariff options](#tariff-options)
    - [Hydrogen Sources](#hydrogen-sources)
- [Performance Metrics](#performance-metrics)
- [Core Simulation Stages](#core-simulation-stages)
    - [Inputs](#inputs)
    - [Estimating Thermal Transmittance](#estimating-thermal-transmittance)
    - [Estimating Space, Water and Total Heating Demand](#estimating-space-water-and-total-heating-demand)
    - [Applying Demand to Non-Electrified Heating Technologies](#applying-demand-to-non-electrified-heating-technologies)
    - [Applying Demand to Electrified Heating Technologies](#applying-demand-to-electrified-heating-technologies)
- [Computational Complexity](#computational-complexity)
- [The Code](#the-code)
    - [Native Code](#native-code)
    - [Website UI](#website-ui)
    - [API Server](#api-server)

# Technologies Simulated

### Primary Heaters

The following heating systems are simulated:
```
1. Air Source Heat Pumps (ASHP)
2. Ground Source Heat Pumps (GSHP)
3. Electric Resistance Heating (ERH)
    *(also known as Electric Boilers or Direct Electrical heating (DEH))*
4. Hydrogen Boilers
5. Hydrogen Fuel Cells
6. Biomass Boilers
7. Natural Gas Boilers
```
ERH is also known as Electric Boilers or Direct Electrical heating (DEH).

### Electrified Heating Technologies
The electrified heating technologies (ASHPs, GSHPs and ERHs) are simulated with solar 
ancillaries and thermal energy storage, along with different energy tariffs.
These technologies are sized based on the peak hourly energy demand experience over the 
simulated year.

#### Solar Ancillaries

The following solar ancillaries (a mix of solar photovoltaic and solar thermal technologies)
are simulated:
```
1. Photovoltaic Panels (PV)
2. Evacuated Tube Solar Thermal Collectors (ET)
3. Flat Plate Solar Thermal Collectors (FP)
4. Photovoltaic-Thermal Hydrid Collectors
```
PV and ET, and PV and FP are also simulated in combination.
The solar ancillaries are sized from 2m<sup>2</sup> upto 1/4 the inputted floor area of 
the property (assume half the roof area), in 2m<sup>2</sup> increments.

#### Thermal Energy Storage (TES)

Water tanks are the only thermal energy storage technology currently simulated. The minimum
water tank volume is 0.1m<sup>3</sup> with a maximum of 3m<sup>3</sup>, and is simulated in 
0.1m<sup>3</sup> increments.

#### Tariff options
The heating systems are simulated with five different tariff options to find the cheapest option.
The five tariffs are:
```
1. Flat-rate
2. Night off-peak
3. Evening-peak
4. EV off-peak
5. Variable. 
```

**The tariff are based off 2019 prior to the economic impact on the UK of Covid-19. 
Therefore expect current prices to vary from those estimated by the model.**

### Hydrogen Sources
For the hydrogen technologies (boiler and fuel cell) they are simulated using hydrogen from
three different sources:

**Grey**: Hydrogen produced from steam methane reforming.

**Blue**: Hydrogen produced from steam methane reforming, but with the addition of Carbon 
Capture and Storage (CCS).

**Green**: Hydrogen produced from the electrolysis of water, with the electroylsis powered 
using grid electricity.

# Performance Metrics
The simulator calaculates the following metrics for each of the systems modeled:
```
1. Capital Expenditure (Upfront Cost)
2. Operational Expenditure (Yearly Cost)
3. Net Present Cost (Lifetime Cost / Cost over 20 years)
```

Additionally for the electrified heating technologies the solar ancillery type and size,
along with TES size that achieves the lowest lifetime cost is outputted.

# Core Simulation Stages

## Inputs

The model requires six inputs from the user, being the:
```
1. Dwelling’s postcode
2. Number of occupants
3. Thermostat temperature
4. Floor area
5. Yearly space heating energy consumption
6. Maximum TES volume. 
```
From the postcode, the longitude and latitude of the home can be determined, which are rounded to the nearest half a degree. The rounded position is used to load the hourly outside temperature and solar radiation datasets for the location. While the user may not know their home’s floor area and yearly space heating energy consumption, this information can usually be found on their Energy Performance Certificate (EPC), a legal requirement for any building sold, let or constructed in the UK since 2007. As the simulator only outputs the optimal TES size for a given system an upper TES size limit is required as input from the user to ensure the model does not optimise for a TES size that could not feasibly fit into the user’s home.

## Estimating Thermal Transmittance

The first step of the model is to estimate the thermal transmittance (U value) of the 
property. This is done by simulating the home’s yearly space heating demand at an hourly
resolution using a range of U values from 0.5 to 3.0 in 0.01 W/m<sup>2</sup>K step 
increments and selecting the U value which results in a yearly space heating demand closest
to that specified by the user. 
 
The hourly space heating demand is taken as the energy required to raise the inside 
temperature to the desired temperature. The inside temperature is a function of heat loss 
to the environment, and gain from body heat and solar radiation.

## Estimating Space, Water and Total Heating Demand
With the U value calculated, the space heating demand is recalculated, in addition to 
hot water demand, using the inputted thermostat temperature. Hot water energy consumption
is a function of inlet cold water temperature and hot water consumption per occupant, 
accounting for daily and monthly variation. 
 
### Applying Demand to Non-Electrified Heating Technologies
The total demand is then used to calculate the operational costs and emissions of the 
hydrogen, biomass and gas heating systems, by multiplying the energy demand with operating 
costs and emissions per unit energy respectively, whilst adjusting for system efficiencies. 

### Applying Demand to Non-Electrified Heating Technologies
As for the electrified heating technologies, the model is more complex, being simulated 
for a range of solar thermal and photovoltaic ancillary technology sizes and combinations,
TES sizes and tariffs. For each hour over a year, the following steps occur.

1.	Calculate the change in inside temperature due to heat loss to the environment and 
heat gain from body heat, solar radiation and heat loss from the TES into the living space.
2.	Update the TES state of charge due to heat loss, solar thermal generation, space and
 hot water usage, and heat produced from the electrified heating technology, powered 
 either from grid import or solar photovoltaics.
3.	Calculate operational costs from electricity import for the hour using the selected
 tariff and operational emissions associated with the solar ancillaries and grid import.

The optimal solar ancillary size, TES size and tariff is then outputted to the user for
each electrified heat technology - solar ancillary combination, where the optimal 
system is considered that with the lowest lifetime cost.

# Computational Complexity

Due to each of the 3 electrified heating systems being simulated at an hourly resolution 
over a year, for each combination and size of solar ancillary, TES sizes and tariff options,
del has a considerable computational cost. 

The number of solar ancillary combinations tested
 per heating technology, O_solar, given by Eq. 1, is dependent on the dwelling’s floor area,
  A. The number of iterations required to go through all TES sizes is given by Eq. 2, 
  and the total number of hours simulated for all electrified heating system combinations 
  is given by Eq. 3.

The number of solar ancillary combinations tested per heating technology if given by:

```
solar_combinations = 6 * floor( floor_area / 8) - 1
tes_combinations = tes_max / 10
total_hours_simulated = 8670*3*5*solar_combinations*tes_combinations
total_hours_simulated = 130,050*solar_combinations*tes_combinations
```
The number of tes combinations tested per solar ancillary if given by:
```
tes_combinations = tes_max / 10
```
The total number of hours simulated for each heating technology, solar ancillary, TES 
and tariff option.
```
total_hours_simulated = 8670 * 3 * 5 * solar_combinations * tes_combinations
total_hours_simulated = 130,050 * solar_combinations * tes_combinations
```

# The code

## Native Code

The simulator was originally developed in Python, but was migrated first to C++ 
and then Rust. This was to achieve improved performance and utilisation of WASM. 
Note while both version should output identical results the Rust version contains 
an improved global optimisation algorithm (although the C++ version still performs better).

Links to Relevant Source Code:
- [Native Rust code](./docs)
- [WASM code compiled from Rust](./docs)
- [Native C++ code](./docs)
- [WASM code compiled from C++](./docs)

Please contact Jack Kirby via [GitHub](https://github.com/Jackrekirby) if you wish to
discuss implementation aspects fo the code.

## Website UI

The website UI was developed with pure HTML, CSS and Javascript. 

[Chart.js](https://www.chartjs.org) was used to create the output graph.

Simulations are by default run on a server using Rust compiled WASM, but one can select 
to run the simulations client-side with Rust or C++ compiled WASM.

The structure of the website pages are outlined below:

- Home
- About Us
    - Our Team
    - Privacy Policy
    - Motivation
- Education
    - Heating Technologies
    - Solar Technologies
    - Why Switch?
- Documentation
    - Simulator Inputs
    - Simulator Outputs
    - How-It-Works
- Simulate

# API / Server

The server is split into two major parts:
1. For fetching EPC certificates.
2. For computing the heating simulations.

The API is currently unavailable for public use due to limited compute hours. 
However the code is publicly available if you wish to view it or host the server yourself.

For further information please see the API specific [repository](https://github.com/heatmyhome-ninja/epc-api). 
