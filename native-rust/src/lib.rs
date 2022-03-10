use wasm_bindgen::prelude::*;
extern crate web_sys;

// to build call: wasm-pack build --target web
// wasm-pack build --debug --target web
// wasm-pack build --target nodejs
// A macro to provide `println!(..)`-style syntax for `console.log` logging.
macro_rules! println {
    ( $( $t:tt )* ) => {
        web_sys::console::log_1(&format!( $( $t )* ).into());
    }
}

mod heat_ninja;

#[wasm_bindgen]
pub fn run_simulation(
    thermostat_temperature: f32,
    latitude: f32,
    longitude: f32,
    num_occupants: u8,
    house_size: f32,
    postcode: String,
    epc_space_heating: f32,
    tes_volume_max: f32,
    agile_tariff_per_hour_over_year: &[f32],
    hourly_outside_temperatures_over_year: &[f32],
    hourly_solar_irradiances_over_year: &[f32],
    enable_optimisation: bool,
) -> String {
    println!(
        "Inputs: {}, {}, {}, {}, {}, {}, {}, {}",
        thermostat_temperature,
        latitude,
        longitude,
        num_occupants,
        house_size,
        postcode,
        epc_space_heating,
        tes_volume_max
    );

    let config: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results_as_csv: false,
        use_multithreading: false,
        save_results_as_csv: false,
        save_results_as_json: false,
        save_all_nodes_as_csv: false,
        print_results_as_json: false,
        save_surfaces: false,
        file_index: 0,
        use_surface_optimisation: enable_optimisation,
        return_format: heat_ninja::ReturnFormat::JSON,
    };

    heat_ninja::run_simulation(
        thermostat_temperature,
        latitude,
        longitude,
        num_occupants,
        house_size,
        &postcode,
        epc_space_heating,
        tes_volume_max,
        &agile_tariff_per_hour_over_year,
        &hourly_outside_temperatures_over_year,
        &hourly_solar_irradiances_over_year,
        &config,
    )
}
