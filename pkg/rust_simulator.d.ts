/* tslint:disable */
/* eslint-disable */
/**
* @param {number} thermostat_temperature
* @param {number} latitude
* @param {number} longitude
* @param {number} num_occupants
* @param {number} house_size
* @param {string} postcode
* @param {number} epc_space_heating
* @param {number} tes_volume_max
* @param {Float32Array} agile_tariff_per_hour_over_year
* @param {Float32Array} hourly_outside_temperatures_over_year
* @param {Float32Array} hourly_solar_irradiances_over_year
* @returns {string}
*/
export function run_simulation(thermostat_temperature: number, latitude: number, longitude: number, num_occupants: number, house_size: number, postcode: string, epc_space_heating: number, tes_volume_max: number, agile_tariff_per_hour_over_year: Float32Array, hourly_outside_temperatures_over_year: Float32Array, hourly_solar_irradiances_over_year: Float32Array): string;
