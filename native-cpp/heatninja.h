#pragma once
#define EM_COMPATIBLE
#include <array>
#include <vector>
#include <string>

namespace heatninja {
    // key terms
    // erh = electric resistance heating
    // hp = heat pump
    // dhw = domestic hot water

    struct SimulationOptions {
        bool output_demand;
        bool output_optimal_specs;
        bool output_all_specs; // not compatible with multithreading or optimal surfaces
        size_t output_file_index;

        bool use_multithreading;
        bool use_optimisation_surfaces;
    };

    // tools
    std::string float_to_string(const float value, const int precision);

    float ax2bxc(float a, float b, float c, float x);

    float ax3bx2cxd(float a, float b, float c, float d, float x);

    // simulation

    std::string run_simulation(const float thermostat_temperature, const float latitude, const float longitude, const int num_occupants, const float house_size, const std::string& postcode, const int epc_space_heating, const float tes_volume_max, const SimulationOptions& simulation_options);

    float round_coordinate(const float coordinate);

    std::vector<float> import_weather_data(const std::string& data_type, const float latitude, const float longitude);

    std::vector<float> import_per_hour_of_year_data(const std::string& filename);

    std::array<float, 24> calculate_erh_hourly_temperature_profile(const float t);

    std::array<float, 24> calculate_hp_hourly_temperature_profile(const float t);

    std::array<float, 12> calculate_monthly_cold_water_temperatures(const float latitude);

    std::array<float, 12> calculate_monthly_solar_height_factors(const float latitude, const std::array<float, 12>& monthly_solar_declination);

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_south(const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_north(const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<float, 12> calculate_monthly_solar_gain_ratios_south(const std::array<float, 12>& monthly_solar_height_factors);

    std::array<float, 12> calculate_monthly_solar_gain_ratios_north(const std::array<float, 12>& monthly_solar_height_factors);

    std::array<float, 12> calculate_monthly_solar_gains_south(const std::array<float, 12>& incident_irradiance_solar_gains_south, const float solar_gain_house_factor);

    std::array<float, 12> calculate_monthly_solar_gains_north(const std::array<float, 12>& incident_irradiance_solar_gains_north, const float solar_gain_house_factor);

    float calculate_average_daily_hot_water_volume(const int num_occupants);

    float calculate_solar_gain_house_factor(const float house_size);

    float calculate_epc_body_gain(const float house_size);

    float calculate_heat_capacity(const float house_size);

    float calculate_body_heat_gain(const int num_occupants);

    int calculate_region_identifier(const std::string& postcode);

    std::array<float, 12> calculate_monthly_epc_outside_temperatures(int region_identifier);

    std::array<int, 12> calculate_monthly_epc_solar_irradiances(int region_identifier);

    const std::array<float, 24>& select_hourly_epc_temperature_profile(const size_t month, const size_t day, const std::array<float, 24>& summer_profile, const std::array<float, 24>& weekend_profile, const std::array<float, 24>& default_profile);

    struct ThermalTransmittanceAndOptimisedEpcDemand {
        float thermal_transmittance, optimised_epc_demand;
    };

    ThermalTransmittanceAndOptimisedEpcDemand calculate_dwellings_thermal_transmittance(const float house_size, const float epc_body_gain, const std::array<float, 12>& monthly_epc_outside_temperatures, const std::array<int, 12>& monthly_epc_solar_irradiances, const std::array<float, 12>& monthly_solar_height_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_solar_gains_south, const std::array<float, 12>& monthly_solar_gains_north, const float heat_capacity, const int epc_space_heating);

    struct Demand {
        float total, max_hourly, space, hot_water;
    };

    Demand calculate_yearly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, const float thermostat_temperature, const std::array<float, 12>& hot_water_monthly_factors, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain);

    void calculate_hourly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, float& inside_temp_current, const float ratio_solar_gain_south, const float ratio_solar_gain_north, const float cwt_current, const float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, const size_t hour_year_counter, const size_t hour, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain);

    void write_demand_data(const std::string filename, const float dwelling_thermal_transmittance, const float optimised_epc_demand, const float yearly_erh_demand, const float maximum_hourly_erh_demand, const float yearly_erh_space_demand, const float yearly_erh_hot_water_demand, const float yearly_hp_demand, const float maximum_hourly_hp_demand, const float yearly_hp_space_demand, const float yearly_hp_hot_water_demand);

    // OPTIMAL SPECIFICATIONS

    enum class HeatOption : int {
        ERH = 0, // electric resistance heating
        ASHP = 1, // air source heat pump
        GSHP = 2 // ground source heat pump
    };

    enum class SolarOption : int {
        None = 0,
        PV = 1, // PV = Photovoltaics
        FP = 2, // FP = Flat Plate
        ET = 3, // ET = Evacuate Tube
        FP_PV = 4,
        ET_PV = 5,
        PVT = 6 // PVT = Photovoltaic thermal hybrid solar collector
    };

    enum class Tariff : int {
        FlatRate = 0,
        Economy7 = 1,
        BulbSmart = 2,
        OctopusGo = 3,
        OctopusAgile = 4
    };

    struct HeatSolarSystemSpecifications {
        HeatOption heat_option;
        SolarOption solar_option;

        int pv_size;
        int solar_thermal_size;
        float tes_volume;

        Tariff tariff;

        float operational_expenditure;
        float capital_expenditure;
        float net_present_cost;

        float operation_emissions;
    };

    float calculate_coldest_outside_temperature_of_year(const float latitude, const float longitude);

    float calculate_ground_temperature(const float latitude);

    int calculate_tes_range(const float tes_volume_max);

    int calculate_solar_maximum(const float house_size);

    float calculate_house_size_thermal_transmittance_product(const float house_size, const float dwelling_thermal_transmittance);

    const std::array<float, 24>& select_temp_profile(const HeatOption hp_option, const std::array<float, 24>& hp_temp_profile, const std::array<float, 24>& erh_temp_profile);

    float calculate_cop_worst(const HeatOption hp_option, const int hot_water_temp, const float coldest_outside_temp, const float ground_temp);

    float calculate_hp_electrical_power(const HeatOption hp_option, const float max_hourly_erh_demand, const float max_hourly_hp_demand, const float cop_worst);

    int calculate_solar_size_range(const SolarOption solar_option, const int solar_maximum);

    std::vector<size_t> linearly_space(float range, size_t segments);

    float min_4f(const float a, const float b, const float c, const float d);

    float get_or_calculate(const size_t i, const size_t j, const size_t x_size, float& min_z, std::vector<float>& zs,
        const HeatOption hp_option, const SolarOption solar_option, float& optimum_tes_npc, const int solar_maximum, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product);

    void if_unset_calculate(const size_t i, const size_t j, const size_t x_size, float& min_z, std::vector<float>& zs,
        const HeatOption hp_option, const SolarOption solar_option, float& optimum_tes_npc, const int solar_maximum, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product);

    void simulate_heat_solar_combination(const HeatOption hp_option, const SolarOption solar_option, const int solar_maximum, const int tes_range, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>& erh_hourly_temperatures_over_day, const std::array<float, 24>& hp_hourly_temperatures_over_day, const int hot_water_temperature, const float coldest_outside_temperature_of_year, const float maximum_hourly_erh_demand, const float maximum_hourly_hp_demand, const float thermostat_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product);

    int calculate_solar_thermal_size(const SolarOption solar_option, const int solar_size);

    int calculate_pv_size(const SolarOption solar_option, const int solar_size, const int solar_maximum, const int solar_thermal_size);

    float calculate_capex_heatopt(const HeatOption hp_option, const float hp_electrical_power_worst);

    float calculate_capex_pv(const SolarOption solar_option, const int pv_size);

    float calculate_capex_solar_thermal(const SolarOption solar_option, const int solar_thermal_size);

    float calculate_capex_tes_volume(const float tes_volume_current);

    float calculate_cumulative_discount_rate(const float discount_rate, const int npc_years);

    std::array<float, 12> calculate_roof_ratios_south(const std::array<float, 12>& monthly_solar_declinations, const float latitude);

    float calculate_optimal_tariff(const HeatOption hp_option, const SolarOption solar_option, const int solar_size, float& optimum_tes_npc, const int solar_maximum, const int tes_option, const float cop_worst, const float hp_electrical_power, const float ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<float, 24>* temp_profile, const float thermostat_temperature, const int hot_water_temperature, const float cumulative_discount_rate, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& dhw_monthly_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_roof_ratios_south, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product);

    void calculate_inside_temp_change(float& inside_temp_current, const float outside_temp_current, const float solar_irradiance_current, const float ratio_sg_south, const float ratio_sg_north, const float ratio_roof_south, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product, const float heat_capacity);

    struct TesTempAndHeight {
        float upper_temperature, lower_temperature, thermocline_height;

        TesTempAndHeight(const float upper_temperature, const float lower_temperature, const float thermocline_height);

        float clamp_height(const float height);
    };

    TesTempAndHeight calculate_tes_temp_and_thermocline_height(const float tes_state_of_charge, const float tes_charge_full, const float tes_charge_max, const float tes_charge_boost, const float cwt_current);

    struct CopCurrentAndBoost
    {
        float current;
        float boost;
    };

    CopCurrentAndBoost calculate_cop_current_and_boost(const HeatOption hp_option, const float outside_temp_current, const float ground_temp, const int hot_water_temperature);

    float calculate_pv_efficiency(const SolarOption solar_option, const float tes_upper_temperature, const float tes_lower_temperature);

    float calculate_solar_thermal_generation_current(const SolarOption solar_option, const float tes_upper_temperature, const float tes_lower_temperature, const int solar_thermal_size, const float incident_irradiance_roof_south, const float outside_temp_current);

    float calculate_hourly_space_demand(float& inside_temp_current, const float desired_min_temp_current, const float cop_current, const float tes_state_of_charge, const float dhw_hr_demand, const float hp_electrical_power, const float heat_capacity);

    float calculate_electrical_demand_for_heating(float& tes_state_of_charge, const float space_water_demand, const float hp_electrical_power, const float cop_current);

    void calculate_electrical_demand_for_tes_charging(float& electrical_demand_current, float& tes_state_of_charge, const float tes_charge_full, const Tariff tariff, const int hour, const float hp_electrical_power, const float cop_current, const float agile_tariff_current);

    void boost_tes_and_electrical_demand(float& tes_state_of_charge, float& electrical_demand_current, const float pv_remaining_current, const float tes_charge_boost, const float hp_electrical_power, const float cop_boost);

    void recharge_tes_to_minimum(float& tes_state_of_charge, float& electrical_demand_current, const float tes_charge_min, const float hp_electrical_power, const float cop_current);

    void add_electrical_import_cost_to_opex(float& operational_costs_off_peak, float& operational_costs_peak, const float electrical_import, const Tariff tariff, const float agile_tariff_current, const int hour);

    void subtract_pv_revenue_from_opex(float& operational_costs_off_peak, float& operational_costs_peak, const float pv_equivalent_revenue, const Tariff tariff, const float agile_tariff_current, const int hour);

    float calculate_emissions_solar_thermal(const float solar_thermal_generation_current);

    float calculate_emissions_pv_generation(const float pv_generation_current, const float pv_equivalent_revenue, const int grid_emissions, const int pv_size);

    float calculate_emissions_grid_import(const float electrical_import, const int grid_emissions);

    void simulate_heating_system_for_day(const std::array<float, 24>* temp_profile, float& inside_temp_current, const float ratio_sg_south, const float ratio_sg_north, const float cwt_current, float dhw_mf_current, float& tes_state_of_charge, const float tes_charge_full, const float tes_charge_boost, const float tes_charge_max, const float tes_radius, const float ground_temp, const HeatOption hp_option, const SolarOption solar_option, const int pv_size, const int solar_thermal_size, const float hp_electrical_power, const Tariff tariff, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, const float ratio_roof_south, const float tes_charge_min, size_t& hour_year_counter, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float u_value, const float heat_capacity, const std::vector<float>& agile_tariff_per_hour_over_year, const std::array<float, 24>& hot_water_hourly_ratios, const float average_daily_hot_water_volume, const int hot_water_temperature, const int grid_emissions, const float solar_gain_house_factor, const float body_heat_gain, const float house_size_thermal_transmittance_product);

    void print_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const int float_print_precision);

    void write_optimal_specification(const HeatSolarSystemSpecifications& spec, std::ofstream& file);

    void write_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const std::string& filename);

    std::string output_to_javascript(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications);

    std::string calculate_hydrogen_gas_biomass_systems(const float yearly_erh_demand, const float yearly_hp_demand, const int epc_space_heating, const float cumulative_discount_rate, const int npc_years, const int grid_emissions);
}