#include "heatninja.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifndef EM_COMPATIBLE
#include <chrono>
#endif

void runSimulationWithDefaultParameters();

extern "C" {
    const char* run_simulation(const char* postcode_char, float latitude, float longitude,
        int num_occupants, float house_size, float temp, int epc_space_heating, float tes_volume_max, bool use_optimisation_surfaces);
}

// FUNCTION DEFINITIONS

#ifndef EM_COMPATIBLE
class Timer {
    std::chrono::steady_clock::time_point start_time;
public:
    Timer()
        :start_time(std::chrono::steady_clock::now())
    {

    }

    void stop() {
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "Runtime: " << elapsed_time / 1000.0 << " s\n";
    }
};
#endif

void runSimulationWithDefaultParameters()
{
    int num_occupants = 2;
    std::string postcode = "CV4 7AL";
    int epc_space_heating = 3000;
    float house_size = 60.0f;
    float tes_volume_max = 0.5f;
    float thermostat_temperature = 20.0f;
    const float latitude = 52.3833f;
    const float longitude = -1.5833f;

#ifndef EM_COMPATIBLE
    bool output_demand = true;
    bool output_optimal_specs = true;
    bool output_all_specs = true;
    size_t output_file_index = 3;
    bool use_multithreading = true;
    bool use_optimisation_surfaces = true;
#else
    bool output_demand = false;
    bool output_optimal_specs = false;
    bool output_all_specs = false;
    size_t output_file_index = 0;
    bool use_multithreading = false;
    bool use_optimisation_surfaces = true;
#endif

#ifndef EM_COMPATIBLE
    Timer t;
#endif
    heatninja::SimulationOptions simulation_options = { output_demand, output_optimal_specs, output_all_specs, output_file_index, use_multithreading, use_optimisation_surfaces };
    std::string java_script_output = heatninja::run_simulation(thermostat_temperature, latitude, longitude, num_occupants, house_size, postcode, epc_space_heating, tes_volume_max, simulation_options);
    //std::cout << java_script_output << "\n";
#ifndef EM_COMPATIBLE
    t.stop();
#endif
}

void readInputFile(std::string filename) {
    std::ifstream infile(filename);
    std::string line;
    int i = 0;
    while (std::getline(infile, line))
    {
        std::stringstream ss(line);

        std::string postcode;
        std::getline(ss, postcode, ',');

        if (postcode == "postcode") continue;
        ++i;
        if (i < 70) {
            continue;
        }

        std::string temporary;

        std::getline(ss, temporary, ',');
        float latitude = std::stof(temporary);

        std::getline(ss, temporary, ',');
        float longitude = std::stof(temporary);

        std::getline(ss, temporary, ',');
        int num_occupants = std::stoi(temporary);

        std::getline(ss, temporary, ',');
        float house_size = std::stof(temporary);

        std::getline(ss, temporary, ',');
        float temp = std::stof(temporary);

        std::getline(ss, temporary, ',');
        int epc_space_heating = std::stoi(temporary);

        std::getline(ss, temporary, ',');
        float tes_volume_max = std::stof(temporary);

        std::cout << postcode << ", " << latitude << ", " << longitude << ", " << num_occupants << ", " << house_size << ", " << temp << ", " << epc_space_heating << ", " << tes_volume_max << ", " << '\n';
        run_simulation(postcode.c_str(), latitude, longitude, num_occupants, house_size, temp, epc_space_heating, tes_volume_max, true);
    }
    infile.close();
}

// FUNCTIONS ACCESSIBLE FROM JAVASCRIPT
extern "C" {
    const char* run_simulation(const char* postcode_char, float latitude, float longitude,
        int num_occupants, float house_size, float thermostat_temperature, int epc_space_heating, float tes_volume_max, bool use_optimisation_surfaces)
    {
        const std::string postcode(postcode_char);

        bool output_demand = false;
        bool output_optimal_specs = false;
        bool output_all_specs = false;
        size_t output_file_index = 0;
        bool use_multithreading = false;

        heatninja::SimulationOptions simulation_options = { output_demand, output_optimal_specs, output_all_specs, output_file_index, use_multithreading, use_optimisation_surfaces };
        const std::string java_script_output = heatninja::run_simulation(thermostat_temperature, latitude, longitude, num_occupants, house_size, postcode, epc_space_heating, tes_volume_max, simulation_options);

        char* result_char = new char[java_script_output.size() + 1];
        std::copy(java_script_output.begin(), java_script_output.end(), result_char);
        result_char[java_script_output.size()] = '\0';
        return result_char;
    }
}

int main()
{
    runSimulationWithDefaultParameters();
}