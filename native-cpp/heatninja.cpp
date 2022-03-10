#include "heatninja.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

#ifndef EM_COMPATIBLE
    #include <execution>
#endif

namespace heatninja {
    // key terms
    // erh = electric resistance heating
    // hp = heat pump
    // dhw = domestic hot water

    constexpr float PI = 3.14159265358979323846f;

    int file_index;
    SimulationOptions simulation_options;
    std::ofstream all_specs_file;

    // DEFINTIONS

    // tools
    std::string printArray(const auto& arr) {
        std::stringstream ss;
        for (const auto& element : arr) {
            ss << element << ", ";
        }
        return ss.str();
    }

    std::string float_to_string(const float value, const int precision)
    {
        std::ostringstream out;
        out.precision(precision);
        out << std::fixed << value;
        return out.str();
    }

    float ax2bxc(float a, float b, float c, float x) {
        return a * x * x + b * x + c;
    }

    float ax3bx2cxd(float a, float b, float c, float d, float x) {
        const float x2 = x * x;
        const float x3 = x2 * x;
        return a * x3 + b * x2 + c * x + d;
    }

    // simulation

    std::string run_simulation(const float thermostat_temperature, const float latitude, const float longitude, const int num_occupants, const float house_size, const std::string& postcode, const int epc_space_heating, const float tes_volume_max, const SimulationOptions& simulation_options) {

        heatninja::simulation_options = simulation_options;
        constexpr int float_print_precision = 2;
        std::cout << "===== Simulation Started =====\n";
        std::cout << "--- Input Parameters ---\n";
        std::cout << "thermostat_temperature: " << float_to_string(thermostat_temperature, float_print_precision) << '\n';
        std::cout << "latitude: " << float_to_string(latitude, float_print_precision) << '\n';
        std::cout << "longitude: " << float_to_string(longitude, float_print_precision) << '\n';
        std::cout << "num_occupants: " << num_occupants << '\n';
        std::cout << "house_size: " << float_to_string(house_size, float_print_precision) << '\n';
        std::cout << "postcode: " << postcode << '\n';
        std::cout << "epc_space_heating: " << epc_space_heating << '\n';
        std::cout << "tes_volume_max: " << tes_volume_max << '\n';
        std::cout << "\n";
        std::cout << "--- Simulation Options ---\n";

        std::cout << "use_optimisation_surfaces: " << simulation_options.use_optimisation_surfaces << '\n';
#ifndef EM_COMPATIBLE
        std::cout << "use_multithreading: " << simulation_options.use_multithreading << '\n';
        std::cout << "output_file_index: " << simulation_options.output_file_index << '\n';
        std::cout << "output_demand: " << simulation_options.output_demand << '\n';
        std::cout << "output_optimal_specs: " << simulation_options.output_optimal_specs << '\n';
        std::cout << "output_all_specs: " << simulation_options.output_all_specs << '\n';
#endif

#ifndef EM_COMPATIBLE
        if (simulation_options.output_all_specs) {
            all_specs_file.open("debug_data/all_specs_" + std::to_string(simulation_options.output_file_index) + ".csv");
        }
#endif

        const std::array<float, 24> erh_hourly_temperatures_over_day = calculate_erh_hourly_temperature_profile(thermostat_temperature);
        const std::array<float, 24> hp_hourly_temperatures_over_day = calculate_hp_hourly_temperature_profile(thermostat_temperature);

        constexpr std::array<float, 12> monthly_solar_declinations = { -20.7f, -12.8f, -1.8f, 9.8f, 18.8f, 23.1f, 21.2f, 13.7f, 2.9f, -8.7f, -18.4f, -23.0f };
        const std::array<float, 12> monthly_solar_height_factors = calculate_monthly_solar_height_factors(latitude, monthly_solar_declinations);

        constexpr std::array<float, 12> dhw_monthly_factors = { 1.10f, 1.06f, 1.02f, 0.98f, 0.94f, 0.90f, 0.90f, 0.94f, 0.98f, 1.02f, 1.06f, 1.10f };
        const std::array<float, 12> monthly_cold_water_temperatures = calculate_monthly_cold_water_temperatures(latitude);

        const std::array<float, 12> monthly_solar_gain_ratios_north = calculate_monthly_solar_gain_ratios_north(monthly_solar_height_factors);
        const std::array<float, 12> monthly_solar_gain_ratios_south = calculate_monthly_solar_gain_ratios_south(monthly_solar_height_factors);

        constexpr std::array<float, 24> hot_water_hourly_ratios = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f, 0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };

        const std::vector<float> hourly_outside_temperatures_over_year = import_weather_data("outside_temps", latitude, longitude);
        const std::vector<float> hourly_solar_irradiances_over_year = import_weather_data("solar_irradiances", latitude, longitude);

        const float average_daily_hot_water_volume = calculate_average_daily_hot_water_volume(num_occupants);

        constexpr int hot_water_temperature = 51;

        const float solar_gain_house_factor = calculate_solar_gain_house_factor(house_size);

        const float epc_body_gain = calculate_epc_body_gain(house_size);

        const int region_identifier = calculate_region_identifier(postcode);
        const std::array<float, 12> monthly_epc_outside_temperatures = calculate_monthly_epc_outside_temperatures(region_identifier);
        const std::array<int, 12> monthly_epc_solar_irradiances = calculate_monthly_epc_solar_irradiances(region_identifier);

        const std::array<float, 12> monthly_incident_irradiance_solar_gains_north = calculate_monthly_incident_irradiance_solar_gains_north(monthly_solar_gain_ratios_north, monthly_epc_solar_irradiances);
        const std::array<float, 12> monthly_incident_irradiance_solar_gains_south = calculate_monthly_incident_irradiance_solar_gains_south(monthly_solar_gain_ratios_south, monthly_epc_solar_irradiances);

        const std::array<float, 12> monthly_solar_gains_south = calculate_monthly_solar_gains_south(monthly_incident_irradiance_solar_gains_south, solar_gain_house_factor);
        const std::array<float, 12> monthly_solar_gains_north = calculate_monthly_solar_gains_north(monthly_incident_irradiance_solar_gains_north, solar_gain_house_factor);
        const float heat_capacity = calculate_heat_capacity(house_size);
        const float body_heat_gain = calculate_body_heat_gain(num_occupants);

        std::cout << "\n--- Energy Performance Certicate Demand ---" << '\n';
        const auto [dwelling_thermal_transmittance, optimised_epc_demand] = calculate_dwellings_thermal_transmittance(house_size, epc_body_gain, monthly_epc_outside_temperatures, monthly_epc_solar_irradiances, monthly_solar_height_factors, monthly_solar_declinations, monthly_solar_gains_south, monthly_solar_gains_north, heat_capacity, epc_space_heating);

        std::cout << "\n--- Electric Resistance Heating Yearly Demand ---" << '\n';
        // structured bindings c++17 https://www.educative.io/edpresso/how-to-return-multiple-values-from-a-function-in-cpp17 https://en.cppreference.com/w/cpp/language/structured_binding
        const auto [yearly_erh_demand, maximum_hourly_erh_demand, yearly_erh_space_demand, yearly_erh_hot_water_demand] = calculate_yearly_space_and_hot_water_demand(erh_hourly_temperatures_over_day, thermostat_temperature, dhw_monthly_factors, monthly_cold_water_temperatures, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, hot_water_hourly_ratios, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, average_daily_hot_water_volume, hot_water_temperature, solar_gain_house_factor, house_size, dwelling_thermal_transmittance, heat_capacity, body_heat_gain);

        std::cout << "\n--- Heat Pump Yearly Demand ---" << '\n';
        const auto [yearly_hp_demand, maximum_hourly_hp_demand, yearly_hp_space_demand, yearly_hp_hot_water_demand] = calculate_yearly_space_and_hot_water_demand(hp_hourly_temperatures_over_day, thermostat_temperature, dhw_monthly_factors, monthly_cold_water_temperatures, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, hot_water_hourly_ratios, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, average_daily_hot_water_volume, hot_water_temperature, solar_gain_house_factor, house_size, dwelling_thermal_transmittance, heat_capacity, body_heat_gain);

        // Output results to JSON
        std::stringstream ss;
        // {"demand":{"boiler":{"hot-water":2309,"space":872,"total":3109,"peak-hourly":178},"heat-pump":{"hot-water":2309,"space":872,"total":3109,"peak-hourly":178}}
        ss << "{\"demand\":{";
        ss << "\"boiler\":" << "{\"hot-water\":" << yearly_erh_hot_water_demand << ",\"space\":" << yearly_erh_space_demand << ",\"total\":" << yearly_erh_demand << ",\"peak-hourly\":" << maximum_hourly_erh_demand << "},";
        ss << "\"heat-pump\":" << "{\"hot-water\":" << yearly_hp_hot_water_demand << ",\"space\":" << yearly_hp_space_demand << ",\"total\":" << yearly_hp_demand << ",\"peak-hourly\":" << maximum_hourly_hp_demand << "}},";
        ss << "\"systems\":{";

#ifndef EM_COMPATIBLE
        if (simulation_options.output_demand) write_demand_data("debug_data/demand_" + std::to_string(simulation_options.output_file_index) + ".csv", dwelling_thermal_transmittance, optimised_epc_demand, yearly_erh_demand, maximum_hourly_erh_demand, yearly_erh_space_demand, yearly_erh_hot_water_demand, yearly_hp_demand, maximum_hourly_hp_demand, yearly_hp_space_demand, yearly_hp_hot_water_demand);
#endif
        // specification optimisation

        const float discount_rate = 1.035f; // 3.5% standard for UK HMRC
        const int npc_years = 20;

        const float coldest_outside_temperature_of_year = calculate_coldest_outside_temperature_of_year(latitude, longitude);
        const float ground_temp = calculate_ground_temperature(latitude);
        const int tes_range = calculate_tes_range(tes_volume_max);
        const int solar_maximum = calculate_solar_maximum(house_size);
        const float house_size_thermal_transmittance_product = calculate_house_size_thermal_transmittance_product(house_size, dwelling_thermal_transmittance);
        const float cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);
        const std::array<float, 12> monthly_roof_ratios_south = calculate_roof_ratios_south(monthly_solar_declinations, latitude);
        constexpr float u_value = 1.30f / 1000; // 0.00130 kW / m2K linearised from https ://zenodo.org/record/4692649#.YQEbio5KjIV &
        const std::vector<float> agile_tariff_per_hour_over_year = import_per_hour_of_year_data("assets/agile_tariff.csv");
        constexpr int grid_emissions = 212; // Current UK 212gCO2e/kWh electricity
        // https://www.gov.uk/government/publications/greenhouse-gas-reporting-conversion-factors-2021

        std::array<HeatSolarSystemSpecifications, 21> optimal_specifications;
        
        if (simulation_options.use_multithreading) {
            #ifndef EM_COMPATIBLE
            constexpr std::array<int, 21> heat_solar_indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
            std::for_each(std::execution::par_unseq, heat_solar_indices.begin(), heat_solar_indices.end(), [&](int i) {
                simulate_heat_solar_combination(static_cast<HeatOption>(i / 7), static_cast<SolarOption>(i % 7), solar_maximum, tes_range, ground_temp, optimal_specifications.at(i), erh_hourly_temperatures_over_day, hp_hourly_temperatures_over_day, hot_water_temperature, coldest_outside_temperature_of_year, maximum_hourly_erh_demand, maximum_hourly_hp_demand, thermostat_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                });
            #endif
        }
        else {
            for (int i = 0; i < 21; ++i) {
                simulate_heat_solar_combination(static_cast<HeatOption>(i / 7), static_cast<SolarOption>(i % 7), solar_maximum, tes_range, ground_temp, optimal_specifications.at(i), erh_hourly_temperatures_over_day, hp_hourly_temperatures_over_day, hot_water_temperature, coldest_outside_temperature_of_year, maximum_hourly_erh_demand, maximum_hourly_hp_demand, thermostat_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
            }
        }
        const std::array<std::string, 3> heat_options_json = { "electric-boiler", "air-source-heat-pump", "ground-source-heat-pump" };
        const std::array<std::string, 7> solar_options_json = { "none", "photovoltaic", "flat-plate", "evacuated-tube", "flat-plate-and-photovoltaic", "evacuated-tube-and-photovoltaic", "photovoltaic-thermal-hybrid" };
        
        for (int i = 0; i < 21; ++i) {
            if (i % 7 == 0) {
                if (i / 7 > 0) {
                    ss << "},";
                }
                ss << "\"" << heat_options_json.at(i / 7) << "\":{";
            }
            const auto& s = optimal_specifications.at(i);
            ss << "\"" << solar_options_json.at(i % 7) << "\":{\"pv-size\":" << s.pv_size << ",\"solar-thermal-size\":" << s.solar_thermal_size << ",\"thermal-energy-storage-volume\":" << s.tes_volume << ",\"operational-expenditure\":" << s.operational_expenditure << ",\"capital-expenditure\":" << s.capital_expenditure << ",\"net-present-cost\":" << s.net_present_cost << ",\"operational-emissions\":" << s.operation_emissions << "}";
            if (i % 7 < 6) {
                ss << ",";
            }
        }
        ss << "},";

        print_optimal_specifications(optimal_specifications, float_print_precision);

        #ifndef EM_COMPATIBLE
        if (simulation_options.output_optimal_specs) write_optimal_specifications(optimal_specifications, "debug_data/optimal_specs_" + std::to_string(simulation_options.output_file_index) + ".csv");
        #endif

        std::string json_systems = calculate_hydrogen_gas_biomass_systems(yearly_erh_demand, yearly_hp_demand, epc_space_heating, cumulative_discount_rate, npc_years, grid_emissions);

        ss << json_systems << "}}";
        //std::cout << ss.str() << "\n";
        // output_to_javascript(optimal_specifications);
        return ss.str();
    }

    std::string calculate_hydrogen_gas_biomass_systems(const float yearly_erh_demand, const float yearly_hp_demand, const int epc_space_heating, const float cumulative_discount_rate, const int npc_years, const int grid_emissions) {
        const float yearly_boiler_demand = yearly_erh_demand / 0.9f;
        const float yearly_fuel_cell_demand = yearly_hp_demand / 0.94f;
        
        // Hydrogen Boiler OPEX, CAPEX, NPC
        const float grey_hydrogen_cost = 0.049f; // 4.9p / kWh A greener gas grid : What are the options, lowest cost also more expensive than gas
        const float blue_hydrogen_cost = 0.093f; // # 9.3p / kWh A greener gas grid : What are the options, average cost
        const float green_hydrogen_cost = 0.184f; // # 18.4p / kWh A greener gas grid : What are the options, highest cost
        // Green cost could also be considered as low cost electricity(across more than 5 or 7 hours of the day) / 0.6
        // 60 % efficient from potentials and risk of H2
        const float grey_hydrogen_boiler_opex = yearly_boiler_demand * grey_hydrogen_cost; // 90 % Boiler efficiency
        const float blue_hydrogen_boiler_opex = yearly_boiler_demand * blue_hydrogen_cost; // 90 % Boiler efficiency
        const float green_hydrogen_boiler_opex = yearly_boiler_demand * green_hydrogen_cost; // 90 % Boiler efficiency
        float hydrogen_boiler_capex = 2000.0f + static_cast<float>(epc_space_heating) / 25.0f;  // £2200 - 3000 from A greener gas grid : What are the options
        if (hydrogen_boiler_capex > 3000) {
            hydrogen_boiler_capex = 3000;
        }
        const float grey_hydrogen_boiler_npc = hydrogen_boiler_capex + cumulative_discount_rate * grey_hydrogen_boiler_opex;
        const float blue_hydrogen_boiler_npc = hydrogen_boiler_capex + cumulative_discount_rate * blue_hydrogen_boiler_opex;
        const float green_hydrogen_boiler_npc = hydrogen_boiler_capex + cumulative_discount_rate * green_hydrogen_boiler_opex;
        
        // Gas Boiler OPEX, CAPEX, NPC
        const float gas_boiler_opex = yearly_boiler_demand * 0.04f; // 90 % Boiler efficiency 4p / kWh
        // https://www.gov.uk/government/statistical-data-sets/annual-domestic-energy-price-statistics
        // Avg gas bills £557, for 13, 600kWh = 4.09p / kWh includes equivalent standing charge
        const float gas_boiler_capex = hydrogen_boiler_capex - 500; // Estimated 500 less
        const float gas_boiler_npc = gas_boiler_capex + cumulative_discount_rate * gas_boiler_opex;

        // Hydrogen Fuel Cell OPEX, CAPEX, NPC
        const float grey_hydrogen_fuel_cell_opex = yearly_fuel_cell_demand * grey_hydrogen_cost; // 55 % thermal efficiency, continuous profile
        const float blue_hydrogen_fuel_cell_opex = yearly_fuel_cell_demand * blue_hydrogen_cost; // 39 % electrical efficiency
        const float green_hydrogen_fuel_cell_opex = yearly_fuel_cell_demand * green_hydrogen_cost; // Equivalent 94 % total efficiency
        // https://www.sciencedirect.com/science/article/pii/S0360319914031383#bib14
        // With all electrical energy used via direct electrical heater, comparable to electricity bill reductions method
        const float hydrogen_fuel_cell_capex = (12000 + 2068.3f * std::powf(0.1f, 0.553f)) * npc_years / 10; // 12000 fuel cell + min TES size, CAPEX of 10 yr life adjusted for npc_years
        // https://www.sciencedirect.com/science/article/pii/S0360319914031383#bib14 lowest cost
        const float grey_hydrogen_fuel_cell_npc = hydrogen_fuel_cell_capex + cumulative_discount_rate * grey_hydrogen_fuel_cell_opex;
        const float blue_hydrogen_fuel_cell_npc = hydrogen_fuel_cell_capex + cumulative_discount_rate * blue_hydrogen_fuel_cell_opex;
        const float green_hydrogen_fuel_cell_npc = hydrogen_fuel_cell_capex + cumulative_discount_rate * green_hydrogen_fuel_cell_opex;

        // Biomass Boiler OPEX, CAPEX, NPC
        // Biomass source https ://www.greenmatch.co.uk/blog/2015/02/how-much-does-a-biomass-boiler-cost
        const float biomass_boiler_fuel_cost = 0.0411f; // 4.11p / kWh
        const float biomass_boiler_opex = yearly_boiler_demand * biomass_boiler_fuel_cost; // 90 % Boiler efficiency
        float biomass_boiler_capex = 9000.0f + static_cast<float>(epc_space_heating) / 4.0f; // £10 - 19k for automatically fed biomass boilers
        if (biomass_boiler_capex > 19000) {
            biomass_boiler_capex = 19000;
        }
        const float biomass_boiler_npc = biomass_boiler_capex + cumulative_discount_rate * biomass_boiler_opex;

        const float boiler_demand_npc = yearly_erh_demand * cumulative_discount_rate; // £s

        // Emissions
        const float gas_emissions_per_kwh = 183; // // 183 gCO2e / kWh for UK natural gas
        const float grey_hydrogen_emissions_per_kwh = 382; // gCO2e / kWh, 382 middle value from SMR w / o CCS in parliament post
        const float blue_hydrogen_emissions_per_kwh = 60; // gCO2e / kWh, 60 middle value from SMR with CCS in parliament post
        const float green_hydrogen_emissions_per_kwh = 1875.0f * ( static_cast<float>(grid_emissions) / 1000.0f); // gCO2e / kWh,  1875 gCO2 / kWhe of grid in parliament post
        const float biomass_boiler_emissions_per_khw = 90; // 90gCO2 / kWh middle value from parliament post

        const float gas_boiler_emissions = yearly_boiler_demand * gas_emissions_per_kwh; 
        const float grey_hydrogen_boiler_emissions = yearly_boiler_demand * grey_hydrogen_emissions_per_kwh;
        const float blue_hydrogen_boiler_emissions = yearly_boiler_demand * blue_hydrogen_emissions_per_kwh;
        const float green_hydrogen_boiler_emissions = yearly_boiler_demand * green_hydrogen_emissions_per_kwh;

        const float grey_hydrogen_fuel_cell_emissions = yearly_fuel_cell_demand * grey_hydrogen_emissions_per_kwh;
        const float blue_hydrogen_fuel_cell_emissions = yearly_fuel_cell_demand * blue_hydrogen_emissions_per_kwh;
        const float green_hydrogen_fuel_cell_emissions = yearly_fuel_cell_demand * green_hydrogen_emissions_per_kwh;

        const float biomass_boiler_emissions = yearly_boiler_demand * biomass_boiler_emissions_per_khw; // 90gCO2 / kWh middle value from parliament post

        // format OPEX, CAPEX & NPC for hydrogen, gas & biomass boilers, and hydrogen fuel cells
        std::stringstream ss;
        ss << "\"hydrogen-boiler\":{";
        ss << "\"grey\":" << "{\"operational-expenditure\":" << grey_hydrogen_boiler_opex << ",\"capital-expenditure\":" << hydrogen_boiler_capex << ",\"net-present-cost\":" << grey_hydrogen_boiler_npc << ",\"operational-emissions\":" << grey_hydrogen_boiler_emissions << "},";
        ss << "\"blue\":" << "{\"operational-expenditure\":" << blue_hydrogen_boiler_opex << ",\"capital-expenditure\":" << hydrogen_boiler_capex << ",\"net-present-cost\":" << blue_hydrogen_boiler_npc << ",\"operational-emissions\":" << blue_hydrogen_boiler_emissions << "},";
        ss << "\"green\":" << "{\"operational-expenditure\":" << green_hydrogen_boiler_opex << ",\"capital-expenditure\":" << hydrogen_boiler_capex << ",\"net-present-cost\":" << green_hydrogen_boiler_npc << ",\"operational-emissions\":" << green_hydrogen_boiler_emissions << "}},";

        ss << "\"hydrogen-fuel-cell\":{";
        ss << "\"grey\":" << "{\"operational-expenditure\":" << grey_hydrogen_fuel_cell_opex << ",\"capital-expenditure\":" << hydrogen_fuel_cell_capex << ",\"net-present-cost\":" << grey_hydrogen_fuel_cell_npc << ",\"operational-emissions\":" << grey_hydrogen_fuel_cell_emissions << "},";
        ss << "\"blue\":" << "{\"operational-expenditure\":" << blue_hydrogen_fuel_cell_opex << ",\"capital-expenditure\":" << hydrogen_fuel_cell_capex << ",\"net-present-cost\":" << blue_hydrogen_fuel_cell_npc << ",\"operational-emissions\":" << blue_hydrogen_fuel_cell_emissions << "},";
        ss << "\"green\":" << "{\"operational-expenditure\":" << green_hydrogen_fuel_cell_opex << ",\"capital-expenditure\":" << hydrogen_fuel_cell_capex << ",\"net-present-cost\":" << green_hydrogen_fuel_cell_npc << ",\"operational-emissions\":" << green_hydrogen_fuel_cell_emissions << "}},";

        ss << "\"gas-boiler\":" << "{\"operational-expenditure\":" << gas_boiler_opex << ",\"capital-expenditure\":" << gas_boiler_capex << ",\"net-present-cost\":" << gas_boiler_npc << ",\"operational-emissions\":" << gas_boiler_emissions << "},";
        ss << "\"biomass-boiler\":" << "{\"operational-expenditure\":" << biomass_boiler_opex << ",\"capital-expenditure\":" << biomass_boiler_capex << ",\"net-present-cost\":" << biomass_boiler_npc << ",\"operational-emissions\":" << biomass_boiler_emissions << "}";

        // {"operational-expenditure":232,"capital-expenditure":679,"net-present-cost":4093,"operational-emissions":214}
        //std::cout << ss.str() << "\n";
        return ss.str();
    }

    float round_coordinate(const float coordinate) {
        float rounded_coordinate = std::roundf(coordinate * 2) / 2;
        // if lat or lon are -0.0f set to +0.0f (important for string conversion)
        if (rounded_coordinate == 0.0f) rounded_coordinate = 0.0f;
        return rounded_coordinate;
    }

    std::vector<float> import_weather_data(const std::string& data_type, const float latitude, const float longitude) {
        // using vector instead of array because: function exceeds stack size, consider moving some data to heap (C6262)
        // data_type = "outside_temps" or "solar_irradiances"
        const float rounded_latitude = round_coordinate(latitude);
        const float rounded_longtitude = round_coordinate(longitude);
        const std::string filename = "assets/" + data_type + "/lat_" + float_to_string(rounded_latitude, 1) + "_lon_" + float_to_string(rounded_longtitude, 1) + ".csv";
        return import_per_hour_of_year_data(filename);
    }

    std::vector<float> import_per_hour_of_year_data(const std::string& filename) {
        std::ifstream infile(filename);
        std::vector<float> data;
        data.reserve(8760);
        int i = 0;
        while (i < 8760)
        {
            std::string line;
            std::getline(infile, line);
            std::stringstream ss(line);
            float x;
            while (ss >> x) {
                data.push_back(x);
            }
            ++i;
        }
        //fmt::print("\n");
        infile.close();
        return data;
    }

    std::array<float, 24> calculate_erh_hourly_temperature_profile(const float t) {
        const float t2 = t - 2;
        return { t2, t2, t2, t2, t2, t2, t2, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t2, t2 };
    }

    std::array<float, 24> calculate_hp_hourly_temperature_profile(const float t) {
        return { t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t };
    }

    std::array<float, 12> calculate_monthly_cold_water_temperatures(const float latitude) {
        if (latitude < 52.2f) { // South of England
            return { 12.1f, 11.4f, 12.3f, 15.2f, 16.1f, 19.3f, 21.2f, 20.1f, 19.5f, 16.8f, 13.7f, 12.4f };
        }
        else if (latitude < 53.3f) { // Middle of England and Wales
            return { 12.9f, 13.3f, 14.4f, 16.3f, 17.7f, 19.7f, 21.8f, 20.1f, 20.3f, 17.8f, 15.3f, 14.0f };
        }
        else if (latitude < 54.95f) { // North of England and Northern Ireland
            return { 9.6f, 9.3f, 10.7f, 13.7f, 15.3f, 17.3f, 19.3f, 18.6f, 17.9f, 15.5f, 12.3f, 10.5f };
        }
        else { // Scotland
            return { 9.6f, 9.2f, 9.8f, 13.2f, 14.5f, 16.8f, 19.4f, 18.5f, 17.5f, 15.1f, 13.7f, 12.4f };
        }
    }

    std::array<float, 12> calculate_monthly_solar_height_factors(const float latitude, const std::array<float, 12>& monthly_solar_declination) {
        std::array<float, 12> solar_height_factors = {};
        size_t i = 0;
        for (float solar_declination : monthly_solar_declination) {
            solar_height_factors.at(i) = std::cosf((PI / 180.0f) * (latitude - solar_declination));
            ++i;
        }
        return solar_height_factors;
    }

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_south(const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<int, 12>& monthly_epc_solar_irradiances) {
        std::array<float, 12> monthly_incident_irradiances_solar_gains_south;
        for (size_t i = 0; i < 12; ++i) {
            monthly_incident_irradiances_solar_gains_south.at(i) = monthly_epc_solar_irradiances.at(i) * monthly_solar_gain_ratios_south.at(i);
        }
        return monthly_incident_irradiances_solar_gains_south;
    }

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_north(const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<int, 12>& monthly_epc_solar_irradiances) {
        std::array<float, 12> monthly_incident_irradiances_solar_gains_north;
        for (size_t i = 0; i < 12; ++i) {
            monthly_incident_irradiances_solar_gains_north.at(i) = monthly_epc_solar_irradiances.at(i) * monthly_solar_gain_ratios_north.at(i);
        }
        return monthly_incident_irradiances_solar_gains_north;
    }

    std::array<float, 12> calculate_monthly_solar_gain_ratios_south(const std::array<float, 12>& monthly_solar_height_factors) {
        const float pf_sg = std::sin(PI / 180 * 90 / 2); // Assume windows are vertical, so no in roof windows
        const float asg_s = ax3bx2cxd(-0.66f, -0.106f, 2.93f, 0, pf_sg);
        const float bsg_s = ax3bx2cxd(3.63f, -0.374f, -7.4f, 0, pf_sg);
        const float csg_s = ax3bx2cxd(-2.71f, -0.991f, 4.59f, 1, pf_sg);

        std::array<float, 12> monthly_solar_gain_ratios_south = {};
        size_t i = 0;
        for (float solar_height_factor : monthly_solar_height_factors) {
            monthly_solar_gain_ratios_south.at(i) = ax2bxc(asg_s, bsg_s, csg_s, solar_height_factor);
            ++i;
        }
        return monthly_solar_gain_ratios_south;
    }

    std::array<float, 12> calculate_monthly_solar_gain_ratios_north(const std::array<float, 12>& monthly_solar_height_factors) {
        const float pf_sg = std::sin(PI / 180 * 90 / 2); // Assume windows are vertical, so no in roof windows
        const float asg_n = ax3bx2cxd(26.3f, -38.5f, 14.8f, 0, pf_sg);
        const float bsg_n = ax3bx2cxd(-16.5f, 27.3f, -11.9f, 0, pf_sg);
        const float csg_n = ax3bx2cxd(-1.06f, -0.0872f, -0.191f, 1, pf_sg);

        std::array<float, 12> monthly_solar_gain_ratios_north = {};
        size_t i = 0;
        for (float solar_height_factor : monthly_solar_height_factors) {
            monthly_solar_gain_ratios_north.at(i) = ax2bxc(asg_n, bsg_n, csg_n, solar_height_factor);
            ++i;
        }
        return monthly_solar_gain_ratios_north;
    }

    std::array<float, 12> calculate_monthly_solar_gains_south(const std::array<float, 12>& incident_irradiance_solar_gains_south, const float solar_gain_house_factor) {
        std::array<float, 12> monthly_solar_gains_south = {};
        size_t i = 0;
        for (float incident_irradiance_solar_gain_south : incident_irradiance_solar_gains_south) {
            monthly_solar_gains_south.at(i) = solar_gain_house_factor * incident_irradiance_solar_gain_south;
            ++i;
        }
        return monthly_solar_gains_south;
    }

    std::array<float, 12> calculate_monthly_solar_gains_north(const std::array<float, 12>& incident_irradiance_solar_gains_north, const float solar_gain_house_factor) {
        std::array<float, 12> monthly_solar_gains_north = {};
        size_t i = 0;
        for (float incident_irradiance_solar_gain_north : incident_irradiance_solar_gains_north) {
            monthly_solar_gains_north.at(i) = solar_gain_house_factor * incident_irradiance_solar_gain_north;
            ++i;
        }
        return monthly_solar_gains_north;
    }

    float calculate_average_daily_hot_water_volume(const int num_occupants) {
        const float showers_vol = (0.45f * num_occupants + 0.65f) * 28.8f;  // Litres, 28.8 equivalent of Mixer with TES
        const float bath_vol = (0.13f * num_occupants + 0.19f) * 50.8f;  // Assumes shower is present
        const float other_vol = 9.8f * num_occupants + 14;
        return showers_vol + bath_vol + other_vol;
    }

    float calculate_solar_gain_house_factor(const float house_size) {
        return (house_size * 0.15f / 2) * 0.77f * 0.7f * 0.76f * 0.9f / 1000;
    }

    float calculate_epc_body_gain(const float house_size) {
        const float epc_num_occupants = 1 + 1.76f * (1 - std::exp(-0.000349f *
            std::powf((house_size - 13.9f), 2))) + 0.0013f * (house_size - 13.9f);
        return (epc_num_occupants * 60) / 1000;
    }

    float calculate_heat_capacity(const float house_size) {
        return (250 * house_size) / 3600;
    }

    float calculate_body_heat_gain(const int num_occupants) {
        return (num_occupants * 60) / 1000.0f;
    }

    struct PostcodeRegion {
        std::string postcode;
        int mininum;
        int maximum;
        int region;
    };

    int calculate_region_identifier(const std::string& postcode) {
        // regioncodes from https://www.bre.co.uk/filelibrary/SAP/2012/SAP-2012_9-92.pdf p177
        const std::array<PostcodeRegion, 169> regioncodes = { { { "ZE", 0, 0, 20 }, { "YO25", 0, 0, 11 }, { "YO", 15, 16, 11 }, { "YO", 0, 0, 10 }, { "WV", 0, 0, 6 }, { "WS", 0, 0, 6 }, { "WR", 0, 0, 6 }, { "WN", 0, 0, 7 }, { "WF", 0, 0, 11 }, { "WD", 0, 0, 1 }, { "WC", 0, 0, 1 }, { "WA", 0, 0, 7 }, { "W", 0, 0, 1 }, { "UB", 0, 0, 1 }, { "TW", 0, 0, 1 }, { "TS", 0, 0, 10 }, { "TR", 0, 0, 4 }, { "TQ", 0, 0, 4 }, { "TN", 0, 0, 2 }, { "TF", 0, 0, 6 }, { "TD15", 0, 0, 9 }, { "TD12", 0, 0, 9 }, { "TD", 0, 0, 9 }, { "TA", 0, 0, 5 }, { "SY", 15, 25, 13 }, { "SY14", 0, 0, 7 }, { "SY", 0, 0, 6 }, { "SW", 0, 0, 1 }, { "ST", 0, 0, 6 }, { "SS", 0, 0, 12 }, { "SR", 7, 8, 10 }, { "SR", 0, 0, 9 }, { "SP", 6, 11, 3 }, { "SP", 0, 0, 5 }, { "SO", 0, 0, 3 }, { "SN7", 0, 0, 1 }, { "SN", 0, 0, 5 }, { "SM", 0, 0, 1 }, { "SL", 0, 0, 1 }, { "SK", 22, 23, 6 }, { "SK17", 0, 0, 6 }, { "SK13", 0, 0, 6 }, { "SK", 0, 0, 7 }, { "SG", 0, 0, 1 }, { "SE", 0, 0, 1 }, { "SA", 61, 73, 13 }, { "SA", 31, 48, 13 }, { "SA", 14, 20, 13 }, { "SA", 0, 0, 5 }, { "S", 40, 45, 6 }, { "S", 32, 33, 6 }, { "S18", 0, 0, 6 }, { "S", 0, 0, 11 }, { "RM", 0, 0, 12 }, { "RH", 10, 20, 2 }, { "RH", 0, 0, 1 }, { "RG", 21, 29, 3 }, { "RG", 0, 0, 1 }, { "PR", 0, 0, 7 }, { "PO", 18, 22, 2 }, { "PO", 0, 0, 3 }, { "PL", 0, 0, 4 }, { "PH50", 0, 0, 14 }, { "PH49", 0, 0, 14 }, { "PH", 30, 44, 17 }, { "PH26", 0, 0, 16 }, { "PH", 19, 25, 17 }, { "PH", 0, 0, 15 }, { "PE", 20, 25, 11 }, { "PE", 9, 12, 11 }, { "PE", 0, 0, 12 }, { "PA", 0, 0, 14 }, { "OX", 0, 0, 1 }, { "OL", 0, 0, 7 }, { "NW", 0, 0, 1 }, { "NR", 0, 0, 12 }, { "NP8", 0, 0, 13 }, { "NP", 0, 0, 5 }, { "NN", 0, 0, 6 }, { "NG", 0, 0, 11 }, { "NE", 0, 0, 9 }, { "N", 0, 0, 1 }, { "ML", 0, 0, 14 }, { "MK", 0, 0, 1 }, { "ME", 0, 0, 2 }, { "M", 0, 0, 7 }, { "LU", 0, 0, 1 }, { "LS24", 0, 0, 10 }, { "LS", 0, 0, 11 }, { "LN", 0, 0, 11 }, { "LL", 30, 78, 13 }, { "LL", 23, 27, 13 }, { "LL", 0, 0, 7 }, { "LE", 0, 0, 6 }, { "LD", 0, 0, 13 }, { "LA", 7, 23, 8 }, { "LA", 0, 0, 7 }, { "L", 0, 0, 7 }, { "KY", 0, 0, 15 }, { "KW", 15, 17, 19 }, { "KW", 0, 0, 17 }, { "KT", 0, 0, 1 }, { "KA", 0, 0, 14 }, { "IV36", 0, 0, 16 }, { "IV", 30, 32, 16 }, { "IV", 0, 0, 17 }, { "IP", 0, 0, 12 }, { "IG", 0, 0, 12 }, { "HX", 0, 0, 11 }, { "HU", 0, 0, 11 }, { "HS", 0, 0, 18 }, { "HR", 0, 0, 6 }, { "HP", 0, 0, 1 }, { "HG", 0, 0, 10 }, { "HD", 0, 0, 11 }, { "HA", 0, 0, 1 }, { "GU", 51, 52, 3 }, { "GU46", 0, 0, 3 }, { "GU", 30, 35, 3 }, { "GU", 28, 29, 2 }, { "GU14", 0, 0, 3 }, { "GU", 11, 12, 3 }, { "GU", 0, 0, 1 }, { "GL", 0, 0, 5 }, { "G", 0, 0, 14 }, { "FY", 0, 0, 7 }, { "FK", 0, 0, 14 }, { "EX", 0, 0, 4 }, { "EN9", 0, 0, 12 }, { "EN", 0, 0, 1 }, { "EH", 43, 46, 9 }, { "EH", 0, 0, 15 }, { "EC", 0, 0, 1 }, { "E", 0, 0, 1 }, { "DY", 0, 0, 6 }, { "DT", 0, 0, 3 }, { "DN", 0, 0, 11 }, { "DL", 0, 0, 10 }, { "DH", 4, 5, 9 }, { "DH", 0, 0, 10 }, { "DG", 0, 0, 8 }, { "DE", 0, 0, 6 }, { "DD", 0, 0, 15 }, { "DA", 0, 0, 2 }, { "CW", 0, 0, 7 }, { "CV", 0, 0, 6 }, { "CT", 0, 0, 2 }, { "CR", 0, 0, 1 }, { "CO", 0, 0, 12 }, { "CM", 21, 23, 1 }, { "CM", 0, 0, 12 }, { "CH", 5, 8, 7 }, { "CH", 0, 0, 7 }, { "CF", 0, 0, 5 }, { "CB", 0, 0, 12 }, { "CA", 0, 0, 8 }, { "BT", 0, 0, 21 }, { "BS", 0, 0, 5 }, { "BR", 0, 0, 2 }, { "BN", 0, 0, 2 }, { "BL", 0, 0, 7 }, { "BH", 0, 0, 3 }, { "BD", 23, 24, 10 }, { "BD", 0, 0, 11 }, { "BB", 0, 0, 7 }, { "BA", 0, 0, 5 }, { "B", 0, 0, 6 }, { "AL", 0, 0, 1 }, { "AB", 0, 0, 16 } } };

        // extract the digit from the outcode
        std::string digits_str = "";
        for (size_t i = 0; i < postcode.size(); ++i) {
            if (isdigit(postcode.at(i))) {
                digits_str += postcode.substr(i, 1);
                if (digits_str.length() > 1) break;
            }
            else if (digits_str.length() > 0) {
                break;
            }
        }
        int digits = std::stoi(digits_str);
        //std::cout << digits << '\n';

        const std::array<std::string, 4> sub_postcodes = { postcode.substr(0, 1), postcode.substr(0, 2), postcode.substr(0, 3), postcode.substr(0, 4) };
        for (auto& regioncode : regioncodes) {
            // search for the outcode in the list of regioncodes
            if (regioncode.postcode == sub_postcodes.at(regioncode.postcode.length() - 1)) {
                // if the region code has a minimum & maximum, check the postcode lies within that range (inclusively).
                if (regioncode.maximum == 0 || (digits >= regioncode.mininum && digits <= regioncode.maximum)) {
                    //std::cout << regioncode.postcode << ' ' << regioncode.mininum << ' ' << regioncode.maximum << ' ' << regioncode.region << ' ' << '\n';
                    return regioncode.region - 1;
                }
            }
        }
        return -1;
    }

    std::array<float, 12> calculate_monthly_epc_outside_temperatures(int region_identifier) {
        constexpr std::array<std::array<float, 12>, 21> monthly_epc_outside_temperatures_per_region = { {
                { 5.1f, 5.6f, 7.4f, 9.9f, 13.0f, 16.0f, 17.9f, 17.8f, 15.2f, 11.6f, 8.0f, 5.1f },
                { 5.0f, 5.4f, 7.1f, 9.5f, 12.6f, 15.4f, 17.4f, 17.5f, 15.0f, 11.7f, 8.1f, 5.2f },
                { 5.4f, 5.7f, 7.3f, 9.6f, 12.6f, 15.4f, 17.3f, 17.3f, 15.0f, 11.8f, 8.4f, 5.5f },
                { 6.1f, 6.4f, 7.5f, 9.3f, 11.9f, 14.5f, 16.2f, 16.3f, 14.6f, 11.8f, 9.0f, 6.4f },
                { 4.9f, 5.3f, 7.0f, 9.3f, 12.2f, 15.0f, 16.7f, 16.7f, 14.4f, 11.1f, 7.8f, 4.9f },
                { 4.3f, 4.8f, 6.6f, 9.0f, 11.8f, 14.8f, 16.6f, 16.5f, 14.0f, 10.5f, 7.1f, 4.2f },
                { 4.7f, 5.2f, 6.7f, 9.1f, 12.0f, 14.7f, 16.4f, 16.3f, 14.1f, 10.7f, 7.5f, 4.6f },
                { 3.9f, 4.3f, 5.6f, 7.9f, 10.7f, 13.2f, 14.9f, 14.8f, 12.8f, 9.7f, 6.6f, 3.7f },
                { 4.0f, 4.5f, 5.8f, 7.9f, 10.4f, 13.3f, 15.2f, 15.1f, 13.1f, 9.7f, 6.6f, 3.7f },
                { 4.0f, 4.6f, 6.1f, 8.3f, 10.9f, 13.8f, 15.8f, 15.6f, 13.5f, 10.1f, 6.7f, 3.8f },
                { 4.3f, 4.9f, 6.5f, 8.9f, 11.7f, 14.6f, 16.6f, 16.4f, 14.1f, 10.6f, 7.1f, 4.2f },
                { 4.7f, 5.2f, 7.0f, 9.5f, 12.5f, 15.4f, 17.6f, 17.6f, 15.0f, 11.4f, 7.7f, 4.7f },
                { 5.0f, 5.3f, 6.5f, 8.5f, 11.2f, 13.7f, 15.3f, 15.3f, 13.5f, 10.7f, 7.8f, 5.2f },
                { 4.0f, 4.4f, 5.6f, 7.9f, 10.4f, 13.0f, 14.5f, 14.4f, 12.5f, 9.3f, 6.5f, 3.8f },
                { 3.6f, 4.0f, 5.4f, 7.7f, 10.1f, 12.9f, 14.6f, 14.5f, 12.5f, 9.2f, 6.1f, 3.2f },
                { 3.3f, 3.6f, 5.0f, 7.1f, 9.3f, 12.2f, 14.0f, 13.9f, 12.0f, 8.8f, 5.7f, 2.9f },
                { 3.1f, 3.2f, 4.4f, 6.6f, 8.9f, 11.4f, 13.2f, 13.1f, 11.3f, 8.2f, 5.4f, 2.7f },
                { 5.2f, 5.0f, 5.8f, 7.6f, 9.7f, 11.8f, 13.4f, 13.6f, 12.1f, 9.6f, 7.3f, 5.2f },
                { 4.4f, 4.2f, 5.0f, 7.0f, 8.9f, 11.2f, 13.1f, 13.2f, 11.7f, 9.1f, 6.6f, 4.3f },
                { 4.6f, 4.1f, 4.7f, 6.5f, 8.3f, 10.5f, 12.4f, 12.8f, 11.4f, 8.8f, 6.5f, 4.6f },
                { 4.8f, 5.2f, 6.4f, 8.4f, 10.9f, 13.5f, 15.0f, 14.9f, 13.1f, 10.0f, 7.2f, 4.7f }
        } };
        return monthly_epc_outside_temperatures_per_region.at(region_identifier);
    }

    std::array<int, 12> calculate_monthly_epc_solar_irradiances(int region_identifier) {
        constexpr std::array<std::array<int, 12>, 21> monthly_epc_solar_irradiances_per_region = { {
            { 30, 56, 98, 157, 195, 217, 203, 173, 127, 73, 39, 24 },
            { 32, 59, 104, 170, 208, 231, 216, 182, 133, 77, 41, 25 },
            { 35, 62, 109, 172, 209, 235, 217, 185, 138, 80, 44, 27 },
            { 36, 63, 111, 174, 210, 233, 204, 182, 136, 78, 44, 28 },
            { 32, 59, 105, 167, 201, 226, 206, 175, 130, 74, 40, 25 },
            { 28, 55, 97, 153, 191, 208, 194, 163, 121, 69, 35, 23 },
            { 24, 51, 95, 152, 191, 203, 186, 152, 115, 65, 31, 20 },
            { 23, 51, 95, 157, 200, 203, 194, 156, 113, 62, 30, 19 },
            { 23, 50, 92, 151, 200, 196, 187, 153, 11, 61, 30, 18 },
            { 25, 51, 95, 152, 196, 198, 190, 156, 115, 64, 32, 20 },
            { 26, 54, 96, 150, 192, 200, 189, 157, 115, 66, 33, 21 },
            { 30, 58, 101, 165, 203, 220, 206, 173, 128, 74, 39, 24 },
            { 29, 57, 104, 164, 205, 220, 199, 167, 120, 68, 35, 22 },
            { 19, 46, 88, 148, 196, 193, 185, 150, 101, 55, 25, 15 },
            { 21, 46, 89, 146, 198, 191, 183, 150, 106, 57, 27, 15 },
            { 19, 45, 89, 143, 194, 188, 177, 144, 101, 54, 25, 14 },
            { 17, 43, 85, 145, 189, 185, 170, 139, 98, 51, 22, 12 },
            { 16, 41, 87, 155, 205, 206, 185, 148, 101, 51, 21, 11 },
            { 14, 39, 84, 143, 205, 201, 178, 145, 100, 50, 19, 9 },
            { 12, 34, 79, 135, 196, 190, 168, 144, 90, 46, 16, 7 },
            { 24, 52, 96, 155, 201, 198, 183, 150, 107, 61, 30, 18 }
        } };
        return monthly_epc_solar_irradiances_per_region.at(region_identifier);
    }

    const std::array<float, 24>& select_hourly_epc_temperature_profile(const size_t month, const size_t day, const std::array<float, 24>& summer_profile, const std::array<float, 24>& weekend_profile, const std::array<float, 24>& default_profile) {
        if (month >= 5 && month <= 8) { // summer no heating
            return summer_profile;
        }
        else if (day % 7 >= 5) { // weekend not summer
            return weekend_profile;
        }
        else { // weekday not summer
            return default_profile;
        }
    }

    ThermalTransmittanceAndOptimisedEpcDemand calculate_dwellings_thermal_transmittance(const float house_size, const float epc_body_gain, const std::array<float, 12>& monthly_epc_outside_temperatures, const std::array<int, 12>& monthly_epc_solar_irradiances, const std::array<float, 12>& monthly_solar_height_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_solar_gains_south, const std::array<float, 12>& monthly_solar_gains_north, const float heat_capacity, const int epc_space_heating) {
        float thermal_transmittance = 0.5;
        float optimised_epc_demand = 0;

        constexpr std::array<size_t, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        static constexpr std::array<float, 24> summer_temperature_profile = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
        static constexpr std::array<float, 24> weekend_temperature_profile = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 };
        static constexpr std::array<float, 24> default_temperature_profile = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20 };

        // thermal_transmittance_current == ttc
        for (float thermal_transmittance_current = 0.5f; thermal_transmittance_current < 3.0f; thermal_transmittance_current += 0.01f) {
            int month = 0;
            float inside_temperature_current = 20;  // Initial temperature
            float epc_demand = 0;

            for (size_t days_in_month : days_in_months) {
                const float outside_temperature_current = monthly_epc_outside_temperatures.at(month);
                //const int solar_irradiance_current = monthly_epc_solar_irradiances.at(month);
                //const float solar_height_factor = monthly_solar_height_factors.at(month);
                //const float solar_declination_current = monthly_solar_declinations.at(month);
                const float solar_gain_south = monthly_solar_gains_south.at(month);
                const float solar_gain_north = monthly_solar_gains_north.at(month);

                for (size_t day = 0; day < days_in_month; ++day) {
                    const std::array<float, 24>& hourly_temperature_profile = select_hourly_epc_temperature_profile(month, day, summer_temperature_profile, weekend_temperature_profile, default_temperature_profile);

                    for (size_t hour = 0; hour < 24; ++hour) {
                        const float desired_temperature_current = hourly_temperature_profile.at(hour);
                        const float heat_flow_out = (house_size * thermal_transmittance_current * (inside_temperature_current - outside_temperature_current)) / 1000;

                        // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
                        inside_temperature_current += (-heat_flow_out + solar_gain_south + solar_gain_north + epc_body_gain) / heat_capacity;
                        if (inside_temperature_current < desired_temperature_current) {  //  Requires heating
                            const float space_hr_demand = (desired_temperature_current - inside_temperature_current) * heat_capacity;
                            inside_temperature_current = desired_temperature_current;
                            epc_demand += space_hr_demand / 0.9f;
                        }
                    }
                }
                ++month;
            }

            const float epc_optimal_heating_demand_diff = std::abs(epc_space_heating - optimised_epc_demand);
            const float epc_heating_demand_diff = std::abs(epc_space_heating - epc_demand);

            //std::cout << epc_space_heating << ' ' << optimised_epc_demand << ' ' << epc_demand << '\n';

            if (epc_heating_demand_diff < epc_optimal_heating_demand_diff) {
                optimised_epc_demand = epc_demand;
                thermal_transmittance = thermal_transmittance_current;
            }
            else { // if the epc heating demand difference is increasing the most optimal has already been found
                break;
            }
        }

        std::cout << "Dwelling Thermal Transmittance: " << thermal_transmittance << '\n';
        std::cout << "Optimised EPC Demand: " << optimised_epc_demand << '\n';
        return { thermal_transmittance, optimised_epc_demand };
    }

    Demand calculate_yearly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, const float thermostat_temperature, const std::array<float, 12>& hot_water_monthly_factors, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain) {
        size_t hour_year_counter = 0;

        float max_hourly_demand = 0;
        float demand_total = 0;

        float inside_temperature_current = thermostat_temperature;
        float hot_water_total = 0;

        constexpr std::array<size_t, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        size_t month = 0;
        for (size_t days_in_month : days_in_months) {
            const float hot_water_monthly_factor = hot_water_monthly_factors.at(month);
            const float cold_water_temperature = monthly_cold_water_temperatures.at(month);
            const float ratio_solar_gain_south = monthly_solar_gain_ratios_south.at(month);
            const float ratio_solar_gain_north = monthly_solar_gain_ratios_north.at(month);
            for (size_t day = 0; day < days_in_month; ++day) {
                for (size_t hour = 0; hour < 24; ++hour) {
                    calculate_hourly_space_and_hot_water_demand(hourly_temperatures_over_day, inside_temperature_current, ratio_solar_gain_south, ratio_solar_gain_north, cold_water_temperature, hot_water_monthly_factor, demand_total, hot_water_total, max_hourly_demand, hour_year_counter, hour, dhw_hourly_ratios, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, average_daily_hot_water_volume, hot_water_temperature, solar_gain_house_factor, house_size, dwelling_thermal_transmittance, heat_capacity, body_heat_gain);
                    ++hour_year_counter;
                }
            }
            ++month;
        }

        const float space_demand = demand_total - hot_water_total;
        std::cout << "Yearly Hot Water Demand: " + float_to_string(hot_water_total, 4) << +" kWh\n";
        std::cout << "Yearly Space demand: " + float_to_string(space_demand, 4) << +" kWh\n";
        std::cout << "Yearly Total demand: " + float_to_string(demand_total, 4) << +" kWh\n";
        std::cout << "Max hourly demand: " + float_to_string(max_hourly_demand, 4) << +" kWh\n";
        //fmt::print("Space demand: {:.2f} kWh\n", space_demand);
        //fmt::print("Yearly total thermal demand: {:.2f} kWh\n", demand_total);
        //fmt::print("Max hourly demand: {:.2f} kWh\n", max_hourly_demand);
        return { demand_total , max_hourly_demand, space_demand, hot_water_total};
    }

    void calculate_hourly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, float& inside_temp_current, const float ratio_solar_gain_south, const float ratio_solar_gain_north, const float cwt_current, const float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, const size_t hour_year_counter, const size_t hour, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain) {
        const float desired_temp_current = hourly_temperatures_over_day.at(hour);
        const float dhw_hr_current = dhw_hourly_ratios.at(hour);
        const float outside_temp_current = hourly_outside_temperatures_over_year.at(hour_year_counter);
        const float solar_irradiance_current = hourly_solar_irradiances_over_year.at(hour_year_counter);

        const float dhw_hr_demand = (average_daily_hot_water_volume * 4.18f * (hot_water_temperature - cwt_current) / 3600) * dhw_mf_current * dhw_hr_current;

        const float incident_irradiance_solar_gain_south = solar_irradiance_current * ratio_solar_gain_south;
        const float incident_irradiance_solar_gain_north = solar_irradiance_current * ratio_solar_gain_north;
        const float solar_gain_south = incident_irradiance_solar_gain_south * solar_gain_house_factor;
        const float solar_gain_north = incident_irradiance_solar_gain_north * solar_gain_house_factor;

        //float solar_irradiance_current = Solar_Irradiance[Weather_Count]
        const float heat_loss = (house_size * dwelling_thermal_transmittance * (inside_temp_current - outside_temp_current)) / 1000;

        // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
        inside_temp_current += (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain) / heat_capacity;

        float space_hr_demand = 0;
        if (inside_temp_current < desired_temp_current) {  //  Requires heating
            space_hr_demand = (desired_temp_current - inside_temp_current) * heat_capacity;
            inside_temp_current = desired_temp_current;
            //fmt::print("i{}\n", inside_temp_current);
        }

        //std::cout << hour << ' ' << space_hr_demand << ' ' << demand_total - dhw_hr_demand << '\n';
        const float hourly_demand = dhw_hr_demand + space_hr_demand;
        max_hourly_demand = std::max(max_hourly_demand, hourly_demand);
        demand_total += hourly_demand;
        dhw_total += dhw_hr_demand;
    }

    void write_demand_data(const std::string filename, const float dwelling_thermal_transmittance, const float optimised_epc_demand, const float yearly_erh_demand, const float maximum_hourly_erh_demand, const float yearly_erh_space_demand, const float yearly_erh_hot_water_demand, const float yearly_hp_demand, const float maximum_hourly_hp_demand, const float yearly_hp_space_demand, const float yearly_hp_hot_water_demand) {
        std::ofstream file(filename);
        file << dwelling_thermal_transmittance << "," << optimised_epc_demand << "\n";
        file << yearly_erh_demand << "," << maximum_hourly_erh_demand << "," << yearly_erh_space_demand << "," << yearly_erh_hot_water_demand << "\n";
        file << yearly_hp_demand << "," << maximum_hourly_hp_demand << "," <<  yearly_hp_space_demand << "," << yearly_hp_hot_water_demand;
        file.close();
    }

    // OPTIMAL SPECIFICATIONS

    float calculate_coldest_outside_temperature_of_year(const float latitude, const float longitude) {
        const std::map<std::string, float> coldest_outside_temperatures_of_year_per_region = { { "50.0_-3.5", 4.61f }, { "50.0_-4.0", 4.554f }, { "50.0_-4.5", 4.406f }, { "50.0_-5.0", 4.017f }, { "50.0_-5.5", 4.492f }, { "50.5_-0.5", 3.02f }, { "50.5_-1.0", 3.188f }, { "50.5_-1.5", 2.812f }, { "50.5_-2.0", 2.583f }, { "50.5_-2.5", 2.774f }, { "50.5_-3.0", 2.697f }, { "50.5_-3.5", 1.744f }, { "50.5_-4.0", 0.854f }, { "50.5_-4.5", 1.27f }, { "50.5_-5.0", 2.708f }, { "50.5_0.0", 2.886f }, { "50.5_0.5", 2.764f }, { "51.0_-0.5", -3.846f }, { "51.0_-1.0", -4.285f }, { "51.0_-1.5", -4.421f }, { "51.0_-2.0", -4.274f }, { "51.0_-2.5", -3.764f }, { "51.0_-3.0", -2.635f }, { "51.0_-3.5", -1.712f }, { "51.0_-4.0", -0.232f }, { "51.0_-4.5", 1.638f }, { "51.0_0.0", -3.344f }, { "51.0_0.5", -2.101f }, { "51.0_1.0", 0.307f }, { "51.0_1.5", 1.271f }, { "51.5_-0.5", -5.969f }, { "51.5_-1.0", -5.673f }, { "51.5_-1.5", -5.09f }, { "51.5_-2.0", -4.292f }, { "51.5_-2.5", -3.039f }, { "51.5_-3.0", -1.591f }, { "51.5_-3.5", 0.221f }, { "51.5_-4.0", 1.249f }, { "51.5_-4.5", 2.001f }, { "51.5_-5.0", 2.948f }, { "51.5_0.0", -5.628f }, { "51.5_0.5", -4.165f }, { "51.5_1.0", -1.369f }, { "51.5_1.5", 1.813f }, { "52.0_-0.5", -5.601f }, { "52.0_-1.0", -5.283f }, { "52.0_-1.5", -4.854f }, { "52.0_-2.0", -4.37f }, { "52.0_-2.5", -3.7f }, { "52.0_-3.0", -3.597f }, { "52.0_-3.5", -3.13f }, { "52.0_-4.0", -2.297f }, { "52.0_-4.5", -0.642f }, { "52.0_-5.0", 2.044f }, { "52.0_-5.5", 3.622f }, { "52.0_0.0", -5.439f }, { "52.0_0.5", -4.533f }, { "52.0_1.0", -2.836f }, { "52.0_1.5", 0.146f }, { "52.5_-0.5", -4.979f }, { "52.5_-1.0", -4.814f }, { "52.5_-1.5", -4.451f }, { "52.5_-2.0", -3.991f }, { "52.5_-2.5", -3.603f }, { "52.5_-3.0", -3.359f }, { "52.5_-3.5", -3.007f }, { "52.5_-4.0", -0.479f }, { "52.5_-4.5", 2.769f }, { "52.5_0.0", -4.845f }, { "52.5_0.5", -4.0f }, { "52.5_1.0", -3.96f }, { "52.5_1.5", -1.778f }, { "52.5_2.0", 1.576f }, { "53.0_-0.5", -4.434f }, { "53.0_-1.0", -4.51f }, { "53.0_-1.5", -4.234f }, { "53.0_-2.0", -3.806f }, { "53.0_-2.5", -3.409f }, { "53.0_-3.0", -2.964f }, { "53.0_-3.5", -2.419f }, { "53.0_-4.0", -0.304f }, { "53.0_-4.5", 1.987f }, { "53.0_-5.0", 3.827f }, { "53.0_0.0", -4.07f }, { "53.0_0.5", -1.754f }, { "53.0_1.0", 0.277f }, { "53.0_1.5", 1.709f }, { "53.0_2.0", 2.397f }, { "53.5_-0.5", -4.156f }, { "53.5_-1.0", -4.141f }, { "53.5_-1.5", -3.834f }, { "53.5_-2.0", -3.492f }, { "53.5_-2.5", -2.729f }, { "53.5_-3.0", -1.344f }, { "53.5_-3.5", 0.446f }, { "53.5_-4.0", 1.524f }, { "53.5_-4.5", 2.578f }, { "53.5_0.0", -2.173f }, { "53.5_0.5", 1.351f }, { "54.0_-0.5", -2.622f }, { "54.0_-1.0", -3.424f }, { "54.0_-1.5", -3.834f }, { "54.0_-2.0", -3.837f }, { "54.0_-2.5", -2.766f }, { "54.0_-3.0", -0.56f }, { "54.0_-3.5", 1.22f }, { "54.0_-5.5", 3.297f }, { "54.0_-6.0", 1.151f }, { "54.0_-6.5", -1.496f }, { "54.0_-7.0", -3.164f }, { "54.0_-7.5", -3.294f }, { "54.0_-8.0", -2.848f }, { "54.0_0.0", 0.231f }, { "54.5_-0.5", 0.579f }, { "54.5_-1.0", -1.903f }, { "54.5_-1.5", -4.414f }, { "54.5_-2.0", -5.579f }, { "54.5_-2.5", -5.161f }, { "54.5_-3.0", -2.187f }, { "54.5_-3.5", -0.424f }, { "54.5_-4.0", 1.047f }, { "54.5_-4.5", 2.244f }, { "54.5_-5.0", 2.994f }, { "54.5_-5.5", 1.337f }, { "54.5_-6.0", -0.575f }, { "54.5_-6.5", -2.338f }, { "54.5_-7.0", -3.041f }, { "54.5_-7.5", -2.662f }, { "54.5_-8.0", -1.808f }, { "55.0_-1.5", -0.996f }, { "55.0_-2.0", -4.155f }, { "55.0_-2.5", -6.204f }, { "55.0_-3.0", -4.514f }, { "55.0_-3.5", -2.703f }, { "55.0_-4.0", -1.58f }, { "55.0_-4.5", -0.407f }, { "55.0_-5.0", 0.806f }, { "55.0_-5.5", 2.081f }, { "55.0_-6.0", 0.887f }, { "55.0_-6.5", -0.469f }, { "55.0_-7.0", -0.993f }, { "55.0_-7.5", -0.77f }, { "55.5_-1.5", 0.873f }, { "55.5_-2.0", -2.474f }, { "55.5_-2.5", -5.702f }, { "55.5_-3.0", -5.566f }, { "55.5_-3.5", -4.895f }, { "55.5_-4.0", -4.132f }, { "55.5_-4.5", -2.358f }, { "55.5_-5.0", -0.579f }, { "55.5_-5.5", 1.338f }, { "55.5_-6.0", 2.057f }, { "55.5_-6.5", 2.505f }, { "56.0_-2.0", 1.815f }, { "56.0_-2.5", 0.195f }, { "56.0_-3.0", -2.189f }, { "56.0_-3.5", -4.626f }, { "56.0_-4.0", -5.49f }, { "56.0_-4.5", -4.919f }, { "56.0_-5.0", -3.499f }, { "56.0_-5.5", -1.181f }, { "56.0_-6.0", 1.063f }, { "56.0_-6.5", 2.977f }, { "56.5_-2.5", -0.305f }, { "56.5_-3.0", -3.11f }, { "56.5_-3.5", -5.41f }, { "56.5_-4.0", -6.757f }, { "56.5_-4.5", -7.005f }, { "56.5_-5.0", -5.879f }, { "56.5_-5.5", -3.253f }, { "56.5_-6.0", 0.046f }, { "56.5_-6.5", 2.699f }, { "56.5_-7.0", 4.242f }, { "57.0_-2.0", 1.061f }, { "57.0_-2.5", -4.347f }, { "57.0_-3.0", -6.774f }, { "57.0_-3.5", -8.256f }, { "57.0_-4.0", -8.531f }, { "57.0_-4.5", -8.952f }, { "57.0_-5.0", -7.613f }, { "57.0_-5.5", -4.211f }, { "57.0_-6.0", -0.368f }, { "57.0_-6.5", 2.421f }, { "57.0_-7.0", 3.249f }, { "57.0_-7.5", 4.066f }, { "57.5_-2.0", 0.562f }, { "57.5_-2.5", -2.636f }, { "57.5_-3.0", -3.24f }, { "57.5_-3.5", -3.825f }, { "57.5_-4.0", -4.351f }, { "57.5_-4.5", -5.412f }, { "57.5_-5.0", -7.049f }, { "57.5_-5.5", -3.771f }, { "57.5_-6.0", 0.002f }, { "57.5_-6.5", 2.105f }, { "57.5_-7.0", 2.649f }, { "57.5_-7.5", 3.287f }, { "58.0_-3.5", 1.614f }, { "58.0_-4.0", -0.872f }, { "58.0_-4.5", -2.392f }, { "58.0_-5.0", -2.029f }, { "58.0_-5.5", 0.609f }, { "58.0_-6.0", 2.139f }, { "58.0_-6.5", 2.056f }, { "58.0_-7.0", 1.757f }, { "58.5_-3.0", 1.924f }, { "58.5_-3.5", 1.382f }, { "58.5_-4.0", 0.97f }, { "58.5_-4.5", 0.903f }, { "58.5_-5.0", 1.605f }, { "58.5_-5.5", 2.935f }, { "58.5_-6.0", 2.901f }, { "58.5_-6.5", 2.723f }, { "58.5_-7.0", 2.661f }, { "59.0_-2.5", 2.975f }, { "59.0_-3.0", 2.525f }, { "59.0_-3.5", 3.066f }, { "59.5_-1.5", 3.281f }, { "59.5_-2.5", 3.684f }, { "59.5_-3.0", 3.79f }, { "60.0_-1.0", 2.361f }, { "60.0_-1.5", 2.383f }, { "60.5_-1.0", 1.794f }, { "60.5_-1.5", 1.783f }, { "61.0_-1.0", 1.721f }
        };

        const float rounded_latitude = round_coordinate(latitude);
        const float rounded_longtitude = round_coordinate(longitude);

        const std::string key = float_to_string(rounded_latitude, 1) + "_" + float_to_string(rounded_longtitude, 1);
        return coldest_outside_temperatures_of_year_per_region.at(key);
    }

    float calculate_ground_temperature(const float latitude) {
        return 15 - (latitude - 50) * (4.0f / 9.0f); // Linear regression ground temp across UK at 100m depth
    }

    int calculate_tes_range(const float tes_volume_max) {
        return static_cast<int>((tes_volume_max + 0.01f) / 0.1f); // +0.01f avoids floating point error
    }

    int calculate_solar_maximum(const float house_size) {
        return static_cast<int>(house_size / 8) * 2;  // Quarter of the roof for solar, even number
    }

    float calculate_house_size_thermal_transmittance_product(const float house_size, const float dwelling_thermal_transmittance) {
        return house_size * dwelling_thermal_transmittance / 1000;
    }

    const std::array<float, 24>& select_temp_profile(const HeatOption hp_option, const std::array<float, 24>& hp_temp_profile, const std::array<float, 24>& erh_temp_profile) {
        switch (hp_option)
        {
        case HeatOption::ASHP:
        case HeatOption::GSHP:
            return hp_temp_profile;
        default:
            return erh_temp_profile;
        }
    }

    float calculate_cop_ref(const HeatOption hp_option) {
        switch (hp_option) // hp sources: A review of domestic heat pumps
        {
        case HeatOption::ERH:
            return 1;
        case HeatOption::ASHP:
            return ax2bxc(0.000630f, -0.121f, 6.81f, 35.0f - 7.0f); // 35oC hot water temp, 7oC ambient temp
        default: //HeatOptions::GSHP
            return ax2bxc(0.000734f, -0.150f, 8.77f, 35.0f); // 35oC hot water temp, 0oC ambient temp
        }
    }

    float calculate_cop_worst(const HeatOption hp_option, const int hot_water_temp, const float coldest_outside_temp, const float ground_temp) {
        switch (hp_option) // hp sources: A review of domestic heat pumps
        {
        case HeatOption::ERH:
            return 1;
        case HeatOption::ASHP:
            return ax2bxc(0.000630f, -0.121f, 6.81f, hot_water_temp - coldest_outside_temp);
        default: //HeatOptions::GSHP
            return ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temp - ground_temp);
        }
    }

    float clamp(float value, float min, float max) {
        if (value < min) { return min; }
        else if (value > max) { return max; }
        else { return value; }
    }

    float calculate_hp_electrical_power(const HeatOption hp_option, const float max_hourly_erh_demand, const float max_hourly_hp_demand, const float cop_worst, const float cop_ref) {
        // Mitsubishi have 4kWth ASHP, Kensa have 3kWth GSHP
        // 7kWth Typical maximum size for domestic power
        switch (hp_option)
        {
        case HeatOption::ERH:
            return clamp(max_hourly_erh_demand, 4.0f / cop_ref, 7.0f);
        case HeatOption::ASHP:
            return clamp(max_hourly_hp_demand / cop_worst, 4.0f / cop_ref, 7.0f);
        default: // GSHP
            return clamp(max_hourly_hp_demand / cop_worst, 6.0f / cop_ref, 7.0f);
        }
    }

    int calculate_solar_size_range(const SolarOption solar_option, const int solar_maximum) {
        switch (solar_option)
        {
        case SolarOption::None:
            return 1;
        case SolarOption::FP_PV:
        case SolarOption::ET_PV:
            return std::max(solar_maximum / 2 - 1, 1);
        default:
            return std::max(solar_maximum / 2, 1);
        }
    }

    std::vector<size_t> linearly_space(float range, size_t segments) {
        std::vector<size_t> points;
        points.reserve(segments + 1);
        const float step = range / segments;
        int j = 0;
        for (float i = 0; j < range; i += step) {
            j = static_cast<int>(i);
            if (static_cast<float>(i - j) > 0.5f) ++j;
            points.push_back(j);
            //std::cout << i << ", " << j << '\n';
        }
        return points;
    }

    struct IndexRect {
        size_t i1, j1, i2, j2;
    };

    float min_4f(const float a, const float b, const float c, const float d) {
        float m = a;
        if (b < m) m = b;
        if (c < m) m = c;
        if (d < m) m = d;
        return m;
    }

    float get_or_calculate(const size_t i, const size_t j, const size_t x_size, float& min_z, std::vector<float>& zs,
        const HeatOption hp_option, const SolarOption solar_option, float& optimum_tes_npc, const int solar_maximum, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product) {
        constexpr float unset_z = 3.40282e+038f;
        float& z = zs.at(i + j * x_size);
        if (z == unset_z) {
            z = calculate_optimal_tariff(hp_option, solar_option, static_cast<int>(j), optimum_tes_npc, solar_maximum, static_cast<int>(i), cop_worst, hp_electrical_power, ground_temp, optimal_spec, temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
            if (z < min_z) min_z = z;
        }
        return z;
    }

    void if_unset_calculate(const size_t i, const size_t j, const size_t x_size, float& min_z, std::vector<float>& zs,
        const HeatOption hp_option, const SolarOption solar_option, float& optimum_tes_npc, const int solar_maximum, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product) {
        constexpr float unset_z = 3.40282e+038f;
        // i = tes_option, j = solar_size
        if (zs.at(i + j * x_size) == unset_z) {
            // return what ever variable you want to optimise by (designed for npc)
            float z = calculate_optimal_tariff(hp_option, solar_option, static_cast<int>(j), optimum_tes_npc, solar_maximum, static_cast<int>(i), cop_worst, hp_electrical_power, ground_temp, optimal_spec, temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
            // may not need min_z
            if (z < min_z) min_z = z;
        }
    }

    void simulate_heat_solar_combination(const HeatOption hp_option, const SolarOption solar_option, const int solar_maximum, const int tes_range, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>& erh_hourly_temperatures_over_day, const std::array<float, 24>& hp_hourly_temperatures_over_day, const int hot_water_temperature, const float coldest_outside_temperature_of_year, const float maximum_hourly_erh_demand, const float maximum_hourly_hp_demand, const float thermostat_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product) {

        const std::array<float, 24>& temp_profile = select_temp_profile(hp_option, hp_hourly_temperatures_over_day, erh_hourly_temperatures_over_day);
        const float cop_ref = calculate_cop_ref(hp_option);
        const float cop_worst = calculate_cop_worst(hp_option, hot_water_temperature, coldest_outside_temperature_of_year, ground_temp);
        const float hp_electrical_power = calculate_hp_electrical_power(hp_option, maximum_hourly_erh_demand, maximum_hourly_hp_demand, cop_worst, cop_ref);
        const int solar_size_range = calculate_solar_size_range(solar_option, solar_maximum);
        float optimum_tes_npc = 3.40282e+038f;

        // OPTIMISER ==========================================================================================
        const size_t min_step = 3;
        float gradient_factor = 0.2f;
        int target_step = 7;
        // user defined variables
        size_t x_size = static_cast<size_t>(tes_range), y_size = static_cast<size_t>(solar_size_range);
        //std::cout << "tes_range: " << tes_range << ", solar_size_range: " << solar_size_range << '\n';
        // only use surface optimisation for surfaces larger than 3 nodes along each dimension
        if (x_size > 3 && y_size > 3 && simulation_options.use_optimisation_surfaces) {
            // non-user variables
            constexpr float unset_z = 3.40282e+038f; // if z has no been found yet it is set to max float value
            float min_z = unset_z; // record the current minimum z
            float max_mx = 0, max_my = 0; // gradient of steepest segment

            // create blank surface of z's
            std::vector<float> zs(x_size * y_size, unset_z);

            // calculate initial points to search on surface
            const size_t x_subdivisions = std::max(x_size / target_step, min_step);
            const size_t y_subdivisions = std::max(y_size / target_step, min_step);
            std::vector<size_t> is = linearly_space(static_cast<float>(x_size - 1), x_subdivisions);
            std::vector<size_t> js = linearly_space(static_cast<float>(y_size - 1), y_subdivisions);

            // combine 1D x and y indices into a 2D mesh 
            std::vector<IndexRect> index_rects;
            for (size_t j = 0; j < y_subdivisions; ++j) {
                for (size_t i = 0; i < x_subdivisions; ++i) {
                    index_rects.emplace_back(IndexRect{ is.at(i), js.at(j), is.at(i + 1), js.at(j + 1) });
                }
            }

            // calculate z for each position and set the min_z and steepest gradient for x & y
            for (IndexRect& r : index_rects) {
                const float z11 = get_or_calculate(r.i1, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                const float z21 = get_or_calculate(r.i2, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                const float z22 = get_or_calculate(r.i2, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                const float z12 = get_or_calculate(r.i1, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);

                const float mx = std::abs((z11 - z21) / (r.i2 - r.i1));
                const float my = std::abs((z11 - z12) / (r.j2 - r.j1));

                if (mx > max_mx) max_mx = mx;
                if (my > max_my) max_my = my;
            }

            // multiply steepest gradient by user defined factor (how much variation in z is there between points?)
            max_mx *= gradient_factor;
            max_my *= gradient_factor;

            while (!index_rects.empty()) {
                std::vector<IndexRect> next_index_rects;
                for (IndexRect& r : index_rects) {

                    // calculate distance between indices
                    const size_t di = r.i2 - r.i1;
                    const size_t dj = r.j2 - r.j1;

                    // assume length > 1 as it is checked when creating a new segment

                    // get npc at nodes of segment
                    const float z11 = get_or_calculate(r.i1, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                    const float z21 = get_or_calculate(r.i2, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                    const float z22 = get_or_calculate(r.i2, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                    const float z12 = get_or_calculate(r.i1, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);

                    // get node with lowest npc
                    float min_local_z = min_4f(z11, z21, z22, z12);
                    // estimate minimum npc between nodes
                    float min_z_estimate = min_local_z - (max_mx * di + max_my * dj);

                    // if segment could have npc lower than the current min subdivide
                    if (min_z_estimate < min_z) {
                        if (di == 1 && dj == 1) { // no more subdivision possible
                            // should not be possible to reach
                            //std::cout << "UNREACHABLE!\n"; // not is the starting surface is only 2xn so the 2 wide sur
                        }
                        else if (di == 1) { // rect only divisible along j
                            const size_t j12 = r.j1 + dj / 2;

                            if_unset_calculate(r.i1, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(r.i2, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);

                            // if rect can be subdivided then subdivide
                            if (j12 - r.j1 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  r.i2, j12 });
                            if (r.j2 - j12 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, j12,  r.i2, r.j2 });
                        }
                        else if (dj == 1) { // rect only divisible along i
                            const size_t i12 = r.i1 + di / 2;

                            if_unset_calculate(i12, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(i12, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);

                            // if rect can be subdivided then subdivide
                            if (i12 - r.i1 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  i12, r.j2 });
                            if (r.i2 - i12 > 1) next_index_rects.emplace_back(IndexRect{ i12, r.j1,  r.i2, r.j2 });
                        }
                        else {
                            // midpoint can be found for both axes
                            const size_t i12 = r.i1 + di / 2;
                            const size_t j12 = r.j1 + dj / 2;

                            if_unset_calculate(i12, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(i12, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(r.i1, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(r.i2, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                            if_unset_calculate(i12, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);

                            const bool sub_i1 = i12 - r.i1 == 1, sub_i2 = r.i2 - i12 == 1;
                            const bool sub_j1 = j12 - r.j1 == 1, sub_j2 = r.j2 - j12 == 1;

                            // one of the dimensions must have a length > 1 if the rect is to be subdivided further
                            if (!(sub_i1 && sub_j1)) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  i12, j12 });
                            if (!(sub_i2 && sub_j1)) next_index_rects.emplace_back(IndexRect{ i12, r.j1,  r.i2, j12 });
                            if (!(sub_i1 && sub_j2)) next_index_rects.emplace_back(IndexRect{ r.i1, j12,  i12, r.j2 });
                            if (!(sub_i2 && sub_j2)) next_index_rects.emplace_back(IndexRect{ i12, j12,  r.i2, r.j2 });
                        }
                    }
                }
                index_rects = next_index_rects;
            }

            if (false) {
                // DEBUG INFORMATION
                int points_searched = 0;
                for (size_t j = 0; j < y_size; ++j) {
                    for (size_t i = 0; i < x_size; ++i) {
                        const float z = zs.at(i + j * x_size);
                        if (z == unset_z) {
                            //std::cout << "-";
                        }
                        else {
                            //std::cout << "#";
                            points_searched++;
                        }
                    }
                    //std::cout << '\n';
                }

                const float efficiency = (static_cast<float>(points_searched) / static_cast<float>(zs.size())) * 100.0f;
                std::cout << "min z: " << min_z << ", points searched: " << points_searched << ", efficiency: " << efficiency << ", gf: " << gradient_factor << ", step: " << target_step << '\n';
            }
            return;
        }

        // brute force method ====================================================================================================
        //std::cout << "Inputs dont meeting requirements for surface optimisation. Falling back to iteration.\n";
        for (int solar_size = 0; solar_size < solar_size_range; ++solar_size) {
            for (int tes_option = 0; tes_option < tes_range; ++tes_option) {
                calculate_optimal_tariff(hp_option, solar_option, solar_size, optimum_tes_npc, solar_maximum, tes_option, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile, thermostat_temperature, hot_water_temperature, cumulative_discount_rate, monthly_solar_gain_ratios_north, monthly_solar_gain_ratios_south, monthly_cold_water_temperatures, dhw_monthly_factors, monthly_solar_declinations, monthly_roof_ratios_south, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
            }
        }
    }

    int calculate_solar_thermal_size(const SolarOption solar_option, const int solar_size) {
        switch (solar_option)
        {
        case SolarOption::None:
        case SolarOption::PV:
            return 0;
        default:
            return (solar_size * 2 + 2);
        }
    }

    int calculate_pv_size(const SolarOption solar_option, const int solar_size, const int solar_maximum, const int solar_thermal_size) {
        switch (solar_option)
        {
        case SolarOption::PV:
        case SolarOption::PVT:
            return solar_size * 2 + 2;
        case SolarOption::FP_PV:
        case SolarOption::ET_PV:
            return solar_maximum - solar_thermal_size;
        default:
            return 0;
        }
    }

    float calculate_capex_heatopt(const HeatOption hp_option, const float hp_thermal_power) {
        switch (hp_option)
        {
        case HeatOption::ERH: // £1000 cost to install ERH, Small additional cost to TES, https://zenodo.org/record/4692649#.YQEbio5KjIV
            return 1000 + 100;
        case HeatOption::ASHP: // ASHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
            return (200 + 4750 / std::powf(hp_thermal_power, 1.25f)) * hp_thermal_power + 1500;  // £s
        default: // GSHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
            return (200 + 4750 / std::powf(hp_thermal_power, 1.25f)) * hp_thermal_power + 800 * hp_thermal_power;
        }
    }

    float calculate_capex_pv(const SolarOption solar_option, const int pv_size) {
        switch (solar_option)
        {
        case SolarOption::PV:
        case SolarOption::FP_PV:
        case SolarOption::ET_PV:
            // PV panels installed
            if (pv_size * 0.2f < 4.0f) { // Less than 4kWp
                return pv_size * 0.2f * 1100; // m2 * 0.2kWp / m2 * £1100 / kWp = £
            }
            else {  // Larger than 4kWp lower £ / kWp
                return pv_size * 0.2f * 900; // m2 * 0.2kWp / m2 * £900 / kWp = £
            }
        default:
            return 0;
        }
    }

    float calculate_capex_solar_thermal(const SolarOption solar_option, const int solar_thermal_size) {
        switch (solar_option)
        {
        case SolarOption::FP:
        case SolarOption::FP_PV:
            // Flat plate solar thermal
            // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
            // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
            return solar_thermal_size * (225.0f + 270.0f / (9.0f * 1.6f)) + 490.0f + 800.0f + 800.0f;
        case SolarOption::PVT:
            // https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
            return (solar_thermal_size / 1.6f) * (480.0f + 270.0f / 9.0f) + 640.0f + 490.0f + 800.0f + 1440.0f;
        case SolarOption::ET:
        case SolarOption::ET_PV:
            // Evacuated tube solar thermal
            // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
            // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
            return solar_thermal_size * (280.0f + 270.0f / (9.0f * 1.6f)) + 490.0f + 800.0f + 800.0f;
        default:
            return 0;
        }
    }

    float calculate_capex_tes_volume(const float tes_volume_current) {
        // Formula based on this data https ://assets.publishing.service.gov.uk/government/uploads/system/uploads/attachment_data/file/545249/DELTA_EE_DECC_TES_Final__1_.pdf
        return 2068.3f * std::powf(tes_volume_current, 0.553f);
    }

    float calculate_cumulative_discount_rate(const float discount_rate, const int npc_years) {
        float discount_rate_current = 1;
        float cumulative_discount_rate = 0;
        for (int year = 0; year < npc_years; ++year) {
            cumulative_discount_rate += 1 / discount_rate_current;
            discount_rate_current *= discount_rate;
        }
        return cumulative_discount_rate;
    }

    std::array<float, 12> calculate_roof_ratios_south(const std::array<float, 12>& monthly_solar_declinations, const float latitude) {
        const float pf = std::sin(PI / 180 * 35 / 2);  // Assume roof is 35° from horizontal
        const float a = ax3bx2cxd(-0.66f, -0.106f, 2.93f, 0, pf);
        const float b = ax3bx2cxd(3.63f, -0.374f, -7.4f, 0, pf);
        const float c = ax3bx2cxd(-2.71f, -0.991f, 4.59f, 1, pf);

        std::array<float, 12> ratios_roof_south = {};
        size_t month = 0;
        for (const float& solar_declination_current : monthly_solar_declinations) {
            const float solar_height_factor = std::cos(PI / 180 * (latitude - solar_declination_current));
            ratios_roof_south.at(month) = ax2bxc(a, b, c, solar_height_factor);
            ++month;
        }

        return ratios_roof_south;
    }

    float calculate_optimal_tariff(const HeatOption hp_option, const SolarOption solar_option, const int solar_size, float& optimum_tes_npc, const int solar_maximum, const int tes_option, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product) {
        // find optimal for given solar_size and tes_vol
        const int solar_thermal_size = calculate_solar_thermal_size(solar_option, solar_size);
        const int pv_size = calculate_pv_size(solar_option, solar_size, solar_maximum, solar_thermal_size);
        float tes_volume_current = 0.1f + tes_option * 0.1f; // m3
        const float hp_thermal_power = hp_electrical_power * calculate_cop_ref(hp_option); // hp option
        const float capex = calculate_capex_heatopt(hp_option, hp_thermal_power) + calculate_capex_pv(solar_option, pv_size) + calculate_capex_solar_thermal(solar_option, solar_thermal_size) + calculate_capex_tes_volume(tes_volume_current);

        const float tes_radius = std::pow((tes_volume_current / (2 * PI)), (1.0f / 3.0f));  //For cylinder with height = 2x radius
        const float tes_charge_full = tes_volume_current * 1000 * 4.18f * (hot_water_temperature - 40) / 3600; // 40 min temp
        const float tes_charge_boost = tes_volume_current * 1000 * 4.18f * (60 - 40) / 3600; //  # kWh, 60C HP with PV boost
        const float tes_charge_max = tes_volume_current * 1000 * 4.18f * (95 - 40) / 3600; //  # kWh, 95C electric and solar

        const float tes_charge_min = 10 * 4.18f * (hot_water_temperature - 10) / 3600; // 10litres hot min amount
        //CWT coming in from DHW re - fill, accounted for by DHW energy out, DHW min useful temperature 40°C
        //Space heating return temperature would also be ~40°C with flow at 51°C

        constexpr std::array<int, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        float optimum_tariff = 1000000;
        float min_npc = 1000000;
        for (int tariff_int = 0; tariff_int < 5; ++tariff_int) {
            Tariff tariff = static_cast<Tariff>(tariff_int);
            size_t hour_year_counter = 0;
            //std::cout << "here\n";
            float inside_temp_current = thermostat_temperature;  // Initial temp
            float solar_thermal_generation_total = 0;
            float operational_costs_peak = 0;
            float operational_costs_off_peak = 0;
            float operation_emissions = 0;

            //(hp_option == HeatOption::ERH && solar_option == SolarOption::FP_PV && pv_size == 6 && solar_thermal_size == 8 && tes_volume_current == 0.2f)

            float tes_state_of_charge = tes_charge_full;  // kWh, for H2O, starts full to prevent initial demand spike
            // https ://www.sciencedirect.com/science/article/pii/S0306261916302045

            int month = 0;
            for (int days_in_month : days_in_months) {
                float ratio_sg_south = monthly_solar_gain_ratios_south.at(month);
                float ratio_sg_north = monthly_solar_gain_ratios_north.at(month);
                float cwt_current = monthly_cold_water_temperatures.at(month);
                float dhw_mf_current = dhw_monthly_factors.at(month);

                const float solar_declination_current = monthly_solar_declinations.at(month);
                float ratio_roof_south = monthly_roof_ratios_south.at(month);

                for (size_t day = 0; day < days_in_month; ++day) {
                    simulate_heating_system_for_day(temp_profile, inside_temp_current, ratio_sg_south, ratio_sg_north, cwt_current, dhw_mf_current, tes_state_of_charge, tes_charge_full, tes_charge_boost, tes_charge_max, tes_radius, ground_temp, hp_option, solar_option, pv_size, solar_thermal_size, hp_electrical_power, tariff, operational_costs_peak, operational_costs_off_peak, operation_emissions, solar_thermal_generation_total, ratio_roof_south, tes_charge_min, hour_year_counter, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year, u_value, heat_capacity, agile_tariff_per_hour_over_year, hot_water_hourly_ratios, average_daily_hot_water_volume, hot_water_temperature, grid_emissions, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product);
                }
                ++month;
            }

            const float total_operational_cost = operational_costs_peak + operational_costs_off_peak; // tariff
            const float npc = capex + total_operational_cost * cumulative_discount_rate;

            if (simulation_options.output_all_specs) {
                const float net_present_cost_current = capex + total_operational_cost * cumulative_discount_rate;
                write_optimal_specification({ hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, tariff, total_operational_cost, capex,  net_present_cost_current, operation_emissions }, all_specs_file);
            }

            if (npc < min_npc) min_npc = npc;

            if (total_operational_cost < optimum_tariff) {
                optimum_tariff = total_operational_cost;

                const float net_present_cost_current = capex + total_operational_cost * cumulative_discount_rate; // £s

                if (net_present_cost_current < optimum_tes_npc) {
                    // Lowest cost TES & tariff for heating tech. For OpEx vs CapEx plots, with optimised TES and tariff
                    optimum_tes_npc = net_present_cost_current;
                    optimal_spec = { hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, tariff, total_operational_cost, capex,  net_present_cost_current, operation_emissions };
                }
            }
        }
        return min_npc;
    }

    void calculate_inside_temp_change(float& inside_temp_current, const float outside_temp_current, const float solar_irradiance_current, const float ratio_sg_south, const float ratio_sg_north, const float ratio_roof_south, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product, const float heat_capacity) {
        const float incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south;
        const float incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north;
        const float solar_gain_south = incident_irradiance_sg_s * solar_gain_house_factor;
        const float solar_gain_north = incident_irradiance_sg_n * solar_gain_house_factor;

        //float solar_irradiance_current = Solar_Irradiance[Weather_Count]
        //const float heat_loss = (house_size * thermal_transmittance * (inside_temp_current - outside_temp_current)) / 1000;
        const float heat_loss = house_size_thermal_transmittance_product * (inside_temp_current - outside_temp_current);

        // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
        inside_temp_current += (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain) / heat_capacity;
    }

    TesTempAndHeight::TesTempAndHeight(const float upper_temperature, const float lower_temperature, const float thermocline_height)
        : upper_temperature(upper_temperature), lower_temperature(lower_temperature), thermocline_height(clamp_height(thermocline_height)) {

    }

    float TesTempAndHeight::clamp_height(const float height) {
        if (height < 0) {
            return 0;
        }
        else if (height > 1) {
            return 1;
        }
        else {
            return height;
        }
    }

    TesTempAndHeight calculate_tes_temp_and_thermocline_height(const float tes_state_of_charge, const float tes_charge_full, const float tes_charge_max, const float tes_charge_boost, const float cwt_current) {
        if (tes_state_of_charge <= tes_charge_full) {  // Currently at nominal temperature ranges
            // tes_lower_temperature Bottom of the tank would still be at CWT,
            // tes_thermocline_height %, from top down, .25 is top 25 %
            return { 51, cwt_current, tes_state_of_charge / tes_charge_full };
        }
        else if (tes_state_of_charge <= tes_charge_boost) { // At boosted temperature ranges
            return { 60, 51, (tes_state_of_charge - tes_charge_full) / (tes_charge_boost - tes_charge_full) };
        }
        else { // At max tes temperature
            return { 95, 60, (tes_state_of_charge - tes_charge_boost) / (tes_charge_max - tes_charge_boost) };
        }
    }

    CopCurrentAndBoost calculate_cop_current_and_boost(const HeatOption hp_option, const float outside_temp_current, const float ground_temp, const int hot_water_temperature) {
        // return current, boost
        switch (hp_option)
        {
        case HeatOption::ERH: //Electric Heater
            return { 1.0f, 1.0f };
        case HeatOption::ASHP: // ASHP, source A review of domestic heat pumps
            return { ax2bxc(0.00063f, -0.121f, 6.81f, hot_water_temperature - outside_temp_current), ax2bxc(0.00063f, -0.121f, 6.81f, 60 - outside_temp_current) };
        default: // GSHP, source A review of domestic heat pumps
            return { ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temperature - ground_temp), ax2bxc(0.000734f, -0.150f, 8.77f, 60 - ground_temp) };
        }
    }

    float calculate_pv_efficiency(const SolarOption solar_option, const float tes_upper_temperature, const float tes_lower_temperature) {
        switch (solar_option)
        {
        case SolarOption::PVT: // PVT
            return(14.7f * (1 - 0.0045f * ((tes_upper_temperature + tes_lower_temperature) / 2.0f - 25))) / 100;
            // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
        default:
            return 0.1928f;
            // Technology Library https ://zenodo.org/record/4692649#.YQEbio5KjIV
            // monocrystalline used for domestic
        }
    }

    float calculate_solar_thermal_generation_current(const SolarOption solar_option, const float tes_upper_temperature, const float tes_lower_temperature, const int solar_thermal_size, const float incident_irradiance_roof_south, const float outside_temp_current) {
        float solar_thermal_generation_current = 0; // if solar_option < 2
        if (solar_option >= SolarOption::FP) {
            const float solar_thermal_collector_temperature = (tes_upper_temperature + tes_lower_temperature) / 2;
            // Collector to heat from tes lower temperature to tes upper temperature, so use the average temperature

            if (incident_irradiance_roof_south == 0) {
                return 0;
            }
            else {
                float a, b, c;
                switch (solar_option)
                {
                case SolarOption::FP: // Flat plate
                case SolarOption::FP_PV:
                    // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                    a = -0.000038f; b = -0.0035f; c = 0.78f;
                    break;
                case SolarOption::PVT: // PVT 
                    // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                    a = -0.0000176f; b = -0.003325f; c = 0.726f;
                    break;
                default: // Evacuated tube
                    // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                    a = -0.00002f; b = -0.0009f; c = 0.625f;
                    break;
                }
                return std::max(0.8f * solar_thermal_size * ax2bxc(a, b, c * incident_irradiance_roof_south, solar_thermal_collector_temperature - outside_temp_current), 0.0f);
            }
        }
        else {
            return 0;
        }
    }

    float calculate_hourly_space_demand(float& inside_temp_current, const float desired_min_temp_current, const float cop_current, const float tes_state_of_charge, const float dhw_hr_demand, const float hp_electrical_power, const float heat_capacity) {
        if (inside_temp_current > desired_min_temp_current) {
            return 0;
        }
        else {
            float space_hr_demand = (desired_min_temp_current - inside_temp_current) * heat_capacity;
            //std::cout << space_hr_demand << ' ' << desired_min_temp_current << ' ' << inside_temp_current << '\n';
            if ((space_hr_demand + dhw_hr_demand) < (tes_state_of_charge + hp_electrical_power * cop_current)) {
                inside_temp_current = desired_min_temp_current;
                return space_hr_demand;
            }
            else {
                if (tes_state_of_charge > 0) { // Priority to space demand over TES charging
                    space_hr_demand = (tes_state_of_charge + hp_electrical_power * cop_current) - dhw_hr_demand;
                }
                else {
                    space_hr_demand = (hp_electrical_power * cop_current) - dhw_hr_demand;
                }
                inside_temp_current += space_hr_demand / heat_capacity;
                return space_hr_demand;
            }
        }
    }

    float calculate_electrical_demand_for_heating(float& tes_state_of_charge, const float space_water_demand, const float hp_electrical_power, const float cop_current) {
        if (space_water_demand < tes_state_of_charge) { // TES can provide all demand
            tes_state_of_charge -= space_water_demand;
            return 0;
        }
        else if (space_water_demand < (tes_state_of_charge + hp_electrical_power * cop_current)) {
            if (tes_state_of_charge > 0) {
                const float electrical_demand_current = (space_water_demand - tes_state_of_charge) / cop_current;;
                tes_state_of_charge = 0;  // TES needs support so taken to empty if it had any charge
                return electrical_demand_current;
            }
            else {
                return space_water_demand / cop_current;
            }
        }
        else { // TES and HP can't meet hour demand
            if (tes_state_of_charge > 0) tes_state_of_charge = 0;
            return hp_electrical_power;
        }
    }

    void calculate_electrical_demand_for_tes_charging(float& electrical_demand_current, float& tes_state_of_charge, const float tes_charge_full, const Tariff tariff, const int hour, const float hp_electrical_power, const float cop_current, const float agile_tariff_current) {
        // Charges TES at off peak electricity times
        if (tes_state_of_charge < tes_charge_full &&
            ((tariff == Tariff::FlatRate && 12 < hour && hour < 16) ||
                (tariff == Tariff::Economy7 && (hour == 23 || hour < 6)) ||
                (tariff == Tariff::BulbSmart && 12 < hour && hour < 16) ||
                (tariff == Tariff::OctopusGo && 0 <= hour && hour < 5) ||
                (tariff == Tariff::OctopusAgile && agile_tariff_current < 9.0f))) {
            // Flat rate and smart tariff charges TES at typical day peak air temperature times
            // GSHP is not affected so can keep to these times too
            if ((tes_charge_full - tes_state_of_charge) < ((hp_electrical_power - electrical_demand_current) * cop_current)) {
                // Small top up
                electrical_demand_current += (tes_charge_full - tes_state_of_charge) / cop_current;
                tes_state_of_charge = tes_charge_full;
            }
            else { // HP can not fully top up in one hour
                tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_current;
                electrical_demand_current = hp_electrical_power;
            }
        }
    }

    void boost_tes_and_electrical_demand(float& tes_state_of_charge, float& electrical_demand_current, const float pv_remaining_current, const float tes_charge_boost, const float hp_electrical_power, const float cop_boost) {
        //Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
        const float tes_boost_state_charge_diff = tes_charge_boost - tes_state_of_charge;
        if (pv_remaining_current > 0 && tes_boost_state_charge_diff > 0) {
            if ((tes_boost_state_charge_diff < (pv_remaining_current * cop_boost)) && (tes_boost_state_charge_diff < ((hp_electrical_power - electrical_demand_current) * cop_boost))) {
                electrical_demand_current += tes_boost_state_charge_diff / cop_boost;
                tes_state_of_charge = tes_charge_boost;
            }
            else if (pv_remaining_current < hp_electrical_power) {
                tes_state_of_charge += pv_remaining_current * cop_boost;
                electrical_demand_current += pv_remaining_current;
            }
            else {
                tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_boost;
                electrical_demand_current = hp_electrical_power;
            }
        }
    }

    void recharge_tes_to_minimum(float& tes_state_of_charge, float& electrical_demand_current, const float tes_charge_min, const float hp_electrical_power, const float cop_current) {
        if (tes_state_of_charge < tes_charge_min) { // Take back up to 10L capacity if possible no matter what time
            if ((tes_charge_min - tes_state_of_charge) < (hp_electrical_power - electrical_demand_current) * cop_current) {
                electrical_demand_current += (tes_charge_min - tes_state_of_charge) / cop_current;
                tes_state_of_charge = tes_charge_min;
            }
            else if (electrical_demand_current < hp_electrical_power) { // Can't take all the way back up to 10L charge
                tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_current;
            }
        }
    }

    void add_electrical_import_cost_to_opex(float& operational_costs_off_peak, float& operational_costs_peak, const float electrical_import, const Tariff tariff, const float agile_tariff_current, const int hour) {
        switch (tariff)
        {
        case Tariff::FlatRate:
            // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
            // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
            operational_costs_peak += 0.163f * electrical_import;
            break;
        case Tariff::Economy7:
            // Economy 7 tariff, same source as flat rate above
            if (hour < 6 || hour == 23) { // Off Peak
                operational_costs_off_peak += 0.095f * electrical_import;
            }
            else { // Peak
                operational_costs_peak += 0.199f * electrical_import;
            }
            break;
        case Tariff::BulbSmart:
            // Bulb smart, for East Midlands values 2021
            // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
            if (15 < hour && hour < 19) { // Peak winter times throughout the year
                operational_costs_peak += 0.2529f * electrical_import;
            }
            else { // Off peak
                operational_costs_off_peak += 0.1279f * electrical_import;
            }
            break;
        case Tariff::OctopusGo:
            // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
            // https://www.octopusreferral.link/octopus-energy-go-tariff/
            if (0 <= hour && hour < 5) { // Off Peak
                operational_costs_off_peak += 0.05f * electrical_import;
            }
            else { // Peak
                operational_costs_peak += 0.1533f * electrical_import;
            }
            break;
        default:
            // Octopus Agile file 2020
            // 2021 Octopus export rates https ://octopus.energy/outgoing/
            if (agile_tariff_current < 9.0f) { // Off peak, lower range of variable costs
                operational_costs_off_peak += (agile_tariff_current / 100) * electrical_import;
            }
            else { // Peak, upper range of variable costs
                operational_costs_peak += (agile_tariff_current / 100) * electrical_import;
            }
            break;
        }
    }

    void subtract_pv_revenue_from_opex(float& operational_costs_off_peak, float& operational_costs_peak, const float pv_equivalent_revenue, const Tariff tariff, const float agile_tariff_current, const int hour) {
        switch (tariff)
        {
        case Tariff::FlatRate:
            // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
            // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
            operational_costs_peak -= pv_equivalent_revenue * (0.163f + 0.035f) / 2;
            break;
        case Tariff::Economy7:
            // Economy 7 tariff, same source as flat rate above
            if (hour < 6 || hour == 23) { // Off Peak
                operational_costs_off_peak -= pv_equivalent_revenue * (0.095f + 0.035f) / 2;
            }
            else { // Peak
                operational_costs_peak -= pv_equivalent_revenue * (0.199f + 0.035f) / 2;
            }
            break;
        case Tariff::BulbSmart:
            // Bulb smart, for East Midlands values 2021
            // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
            if (15 < hour && hour < 19) { // Peak winter times throughout the year
                operational_costs_peak -= pv_equivalent_revenue * (0.2529f + 0.035f) / 2;
            }
            else { // Off peak
                operational_costs_off_peak -= pv_equivalent_revenue * (0.1279f + 0.035f) / 2;
            }
            break;
        case Tariff::OctopusGo:
            // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
            // https://www.octopusreferral.link/octopus-energy-go-tariff/
            if (0 <= hour && hour < 5) { // Off Peak
                operational_costs_off_peak -= pv_equivalent_revenue * (0.05f + 0.03f) / 2;
            }
            else { // Peak
                operational_costs_peak -= pv_equivalent_revenue * (0.1533f + 0.03f) / 2;
            }
            break;
        default:
            // Octopus Agile file 2020
            // 2021 Octopus export rates https ://octopus.energy/outgoing/
            if (agile_tariff_current < 9.0f) { // Off peak, lower range of variable costs
                operational_costs_off_peak -= pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
            }
            else { // Peak, upper range of variable costs
                operational_costs_peak -= pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
            }
            break;
        }
    }

    float calculate_emissions_solar_thermal(const float solar_thermal_generation_current) {
        // Operational emissions summation
        // 22.5 average ST
        // from https ://post.parliament.uk/research-briefings/post-pn-0523/
        return solar_thermal_generation_current * 22.5f;
    }

    float calculate_emissions_pv_generation(const float pv_generation_current, const float pv_equivalent_revenue, const int grid_emissions, const int pv_size) {
        // https://www.parliament.uk/globalassets/documents/post/postpn_383-carbon-footprint-electricity-generation.pdf
        // 75 for PV, 75 - Grid_Emissions show emissions saved for the grid or for reducing other electrical bills
        if (pv_size > 0) {
            return (pv_generation_current - pv_equivalent_revenue) * 75 + pv_equivalent_revenue * (75 - grid_emissions);
        }
        else {
            return 0;
        }
    }

    float calculate_emissions_grid_import(const float electrical_import, const int grid_emissions) {
        return electrical_import * grid_emissions;
    }

    void simulate_heating_system_for_day(const std::array<float, 24>* temp_profile, float& inside_temp_current, const float ratio_sg_south, const float ratio_sg_north, const float cwt_current, float dhw_mf_current, float& tes_state_of_charge, const float tes_charge_full, const float tes_charge_boost, const float tes_charge_max, const float tes_radius, const float ground_temp, const HeatOption hp_option, const SolarOption solar_option, const int pv_size, const int solar_thermal_size, const float hp_electrical_power, const Tariff tariff, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, const float ratio_roof_south, const float tes_charge_min, size_t& hour_year_counter, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int hot_water_temperature, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product) {
        const float pi_d = PI * tes_radius * 2;
        const float pi_r2 = PI * tes_radius * tes_radius;
        const float pi_d2 = pi_d * tes_radius * 2;

        for (size_t hour = 0; hour < 24; ++hour) {
            const float outside_temp_current = hourly_outside_temperatures_over_year.at(hour_year_counter);
            const float solar_irradiance_current = hourly_solar_irradiances_over_year.at(hour_year_counter);
            calculate_inside_temp_change(inside_temp_current, outside_temp_current, solar_irradiance_current, ratio_sg_south, ratio_sg_north, ratio_roof_south, solar_gain_house_factor, body_heat_gain, house_size_thermal_transmittance_product, heat_capacity);
            const auto [tes_upper_temperature, tes_lower_temperature, tes_thermocline_height] = calculate_tes_temp_and_thermocline_height(tes_state_of_charge, tes_charge_full, tes_charge_max, tes_charge_boost, cwt_current);
            //std::cout << hour << " 1 " << inside_temp_current << '\n';
            const float tes_upper_losses = (tes_upper_temperature - inside_temp_current) * u_value * (pi_d2 * tes_thermocline_height + pi_r2); // losses in kWh
            const float tes_lower_losses = (tes_lower_temperature - inside_temp_current) * u_value * (pi_d2 * (1 - tes_thermocline_height) + pi_r2);
            const float total_losses = tes_upper_losses + tes_lower_losses;
            tes_state_of_charge -= total_losses;
            inside_temp_current += total_losses / heat_capacity;
            //std::cout << hour << " 2 " << inside_temp_current << '\n';
            const float desired_min_temp_current = temp_profile->at(hour);
            const float agile_tariff_current = agile_tariff_per_hour_over_year.at(hour_year_counter);
            const float dhw_hr_current = hot_water_hourly_ratios.at(hour);
            const float dhw_hr_demand = (average_daily_hot_water_volume * 4.18f * (hot_water_temperature - cwt_current) / 3600) * dhw_mf_current * dhw_hr_current;

            const auto [cop_current, cop_boost] = calculate_cop_current_and_boost(hp_option, outside_temp_current, ground_temp, hot_water_temperature);

            const float pv_efficiency = calculate_pv_efficiency(solar_option, tes_upper_temperature, tes_lower_temperature);

            const float incident_irradiance_roof_south = solar_irradiance_current * ratio_roof_south / 1000; // kW / m2
            float pv_generation_current = pv_size * pv_efficiency * incident_irradiance_roof_south * 0.8f;  // 80 % shading factor

            const float solar_thermal_generation_current = calculate_solar_thermal_generation_current(solar_option, tes_upper_temperature, tes_lower_temperature, solar_thermal_size, incident_irradiance_roof_south, outside_temp_current);
            tes_state_of_charge += solar_thermal_generation_current;
            solar_thermal_generation_total += solar_thermal_generation_current;
            // Dumps any excess solar generated heat to prevent boiling TES
            tes_state_of_charge = std::min(tes_state_of_charge, tes_charge_max);

            const float space_hr_demand = calculate_hourly_space_demand(inside_temp_current, desired_min_temp_current, cop_current, tes_state_of_charge, dhw_hr_demand, hp_electrical_power, heat_capacity);
            //std::cout << hour << " 3 " << inside_temp_current << '\n';
            float electrical_demand_current = calculate_electrical_demand_for_heating(tes_state_of_charge, space_hr_demand + dhw_hr_demand, hp_electrical_power, cop_current);
            calculate_electrical_demand_for_tes_charging(electrical_demand_current, tes_state_of_charge, tes_charge_full, tariff, static_cast<int>(hour), hp_electrical_power, cop_current, agile_tariff_current);
            const float pv_remaining_current = pv_generation_current - electrical_demand_current;

            //Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
            boost_tes_and_electrical_demand(tes_state_of_charge, electrical_demand_current, pv_remaining_current, tes_charge_boost, hp_electrical_power, cop_boost);

            recharge_tes_to_minimum(tes_state_of_charge, electrical_demand_current, tes_charge_min, hp_electrical_power, cop_current);

            float pv_equivalent_revenue;
            float electrical_import;
            if (pv_generation_current > electrical_demand_current) { // Generating more electricity than using
                pv_equivalent_revenue = pv_generation_current - electrical_demand_current;
                electrical_import = 0;
                subtract_pv_revenue_from_opex(operational_costs_off_peak, operational_costs_peak, pv_equivalent_revenue, tariff, agile_tariff_current, static_cast<int>(hour));
            }
            else {
                pv_equivalent_revenue = 0;
                electrical_import = electrical_demand_current - pv_generation_current;
                add_electrical_import_cost_to_opex(operational_costs_off_peak, operational_costs_peak, electrical_import, tariff, agile_tariff_current, static_cast<int>(hour));
            }

            operation_emissions += calculate_emissions_solar_thermal(solar_thermal_generation_current) +
                calculate_emissions_pv_generation(pv_generation_current, pv_equivalent_revenue, grid_emissions, pv_size) +
                calculate_emissions_grid_import(electrical_import, grid_emissions);
            hour_year_counter++;
        }
    }

    void print_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const int float_print_precision) {
        std::cout << "\n--- Optimum TES and Net Present Cost per Heating & Solar Option ---";
        std::cout << "\nHeat Opt, Solar Opt, PV Size, Solar Size, TES Vol, OPEX, CAPEX, NPC, Emissions, Tariff\n";

        const std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
        const std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT" };
        const std::array<std::string, 5> tariff_names = { "Flat Rate", "Economy 7", "Bulb Smart", "Octopus Go", "Octopus Agile" };

        for (const auto& s : optimal_specifications) {
            std::cout << heat_opt_names.at(static_cast<int>(s.heat_option)) << ", " << solar_opt_names.at(static_cast<int>(s.solar_option)) << ", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << float_to_string(s.operational_expenditure, float_print_precision) << ", " << float_to_string(s.capital_expenditure, float_print_precision) << ", " << float_to_string(s.net_present_cost, float_print_precision) << ", " << float_to_string(s.operation_emissions, float_print_precision) << ", " << tariff_names.at(static_cast<int>(s.tariff)) << "\n";
        }
    }

    void write_optimal_specification(const HeatSolarSystemSpecifications& spec, std::ofstream& file) {
            file << static_cast<int>(spec.heat_option) << "," <<
                static_cast<int>(spec.solar_option) << "," <<
                spec.pv_size << "," <<
                spec.solar_thermal_size << "," <<
                float_to_string(spec.tes_volume, 1) << "," <<
                static_cast<int>(spec.tariff) << "," <<
                float_to_string(spec.operational_expenditure, 2) << "," <<
                float_to_string(spec.capital_expenditure, 2) << "," <<
                float_to_string(spec.net_present_cost, 2) << "," <<
                float_to_string(spec.operation_emissions, 2) << "\n";
    }

    void write_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const std::string& filename) {
        std::ofstream file(filename);
        for (const auto& optimal_specification : optimal_specifications) {
            write_optimal_specification(optimal_specification, file);
        }
        file.close();
    }

    std::string output_to_javascript(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications) {
        const std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
        const std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT" };
        const std::array<std::string, 5> tariff_names = { "Flat Rate", "Economy 7", "Bulb Smart", "Octopus Go", "Octopus Agile" };

        std::stringstream ss;
        ss << '[';
        int i = 0;
        for (const auto& s : optimal_specifications) {
            if (i != 0) ss << ',';
            ss << "[\"" << heat_opt_names.at(static_cast<int>(s.heat_option)) << "\", \"" << solar_opt_names.at(static_cast<int>(s.solar_option)) << "\", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << float_to_string(s.operational_expenditure, 0) << ", " << float_to_string(s.capital_expenditure, 0) << ", " << float_to_string(s.net_present_cost, 0) << ", " << float_to_string(s.operation_emissions / 1000, 0) << "]";
            ++i;
        }
        ss << ']';
        return ss.str();
    }
}