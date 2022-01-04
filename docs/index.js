// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// guide

// globals
//  sim_worker
//  sim_output

// constants
//  parameter_name_list
//  heat_options_json
//  heat_options_print
//  solar_options_json
//  solar_options_print
//  epc_api_url

// functions
//  'initiate sim_worker'
// parameter functions
//  load_parameters(parameter_list)
//  load_default_parameters()
//  get_parameter_id(name)
//  load_url_with_parameters()
//  generate_url_with_parameters()
//  collect_parameters()
// postcode functions
//  postcode_onchange_callback()
//  call_postcode_api(postcode)
// epc functions
//  find_address
//  get_epc_data
// net present cost functions
//  calculate_net_present_cost(opex, capex, cumulative_discount_rate)
//  calculate_cumulative_discount_rate(discount_rate, npc_years)
//  lifetime_callback()
//  recalculate_net_present_costs()
// submit, download, copy simulation functions
//  submit_simulation()
//  create_simulation_output_file_download()
//  clipboard_simulation_output()
// rendering the simulation table functions
//  get_sorted_indices(array)
//  fill_table_row(table, system)
//  group_systems_callback()
//  sortby_callback()
//  collapse_groups_callback()
//  make_cell(tr, value)
//  make_cells(tr, system)
//  render_simulation_table()

// worker
//  output message types:
//      initiate
//      run simulation
//  input message types:
//      initiation complete
//      simulation complete

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// pre main

// debug: for adding origin to CORS
// console.log('origin:', location.origin);

// simulation output (JSON format)
var sim_output;

// list of parameters for the simulation. Can be used as url parameters to preload parameters
// format: key: url-param-name, value: html-id
const parameter_name_list = [
    'postcode',
    'latitude',
    'longitude',
    'occupants',
    'temperature',
    'space_heating',
    'floor_area',
    'tes_max',
];

const discount_rate = 1.035; // 3.5% standard for UK HMRC
let npc_years = 20;
let cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);

// const epc_api_url = 'http://heatmyhomeninja-env.eba-w2gamium.us-east-2.elasticbeanstalk.com/';
//const epc_api_url = 'http://localhost:3000/';
const epc_api_url = 'https://customapi.heatmyhome.ninja/'

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// main
// check if web workers are supported
if (window.Worker) {
    // run simulations on worker to avoid UI freezing
    // instantiate web worker.
    var sim_worker = new Worker('worker.js');

    sim_worker.onmessage = function (msg) {
        //console.log('Message received from worker:', msg);
        const msg_type = msg.data[0];
        switch (msg_type) {
            case 'initiation complete':
                console.log('simulation worker initiated');
                load_default_parameters();
                break;
            case 'simulation complete':
                if (msg.data[1] == "") {
                    document.getElementById('sim-runtime').innerHTML = "FAILED";
                    document.getElementById('sim-submit').disabled = false;
                } else {
                    // log and display simulation runtime
                    let end = performance.now();
                    let runtime = ((end - msg.data[2]) / 1000.0).toPrecision(3);
                    console.log(`C++ Simulation Runtime: ${runtime}s`);
                    document.getElementById('sim-runtime').innerHTML = runtime;

                    // save, log and render simulation output
                    sim_output = JSON.parse(msg.data[1]);
                    document.getElementById('results').classList.remove("hide");
                    document.getElementById('sim-submit').disabled = false;
                    console.log(sim_output);
                    render_simulation_table();
                    create_simulation_output_file_download();
                }
                break;
            default:
                console.warn('Message from worker is not linked to any event: ', msg.data);
        }
    }

    sim_worker.postMessage("initiate");
} else {
    console.warn('Web Workers are not supported');
    // take user to another page
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// parameter functions

function get_parameter_id(name) {
    //e.g. space_heating -> sim-space-heating
    return `sim-${name.replace('_', '-')}`;
}

function load_parameters(parameter_list) {
    for (const [parameter, value] of Object.entries(parameter_list)) {
        document.getElementById(get_parameter_id(parameter)).value = value;
    }
}

function load_default_parameters() {
    const urlParams = new URLSearchParams(window.location.search);
    let autofill = Number(urlParams.get('autofill'));
    console.log('autofill mode:', autofill);

    switch (autofill) {
        case 1:
            const non_api_parameters = {
                'postcode': 'CV47AL',
                'occupants': 2,
                'temperature': 20.0,
                'tes_max': 0.5,
            };
            load_parameters(non_api_parameters);
            postcode_onchange_callback();
            break;
        case 2:
            const default_parameters = {
                'postcode': 'CV47AL',
                'latitude': 52.3833,
                'longitude': -1.5833,
                'occupants': 2,
                'temperature': 20.0,
                'space_heating': 3000,
                'floor_area': 60.0,
                'tes_max': 0.5,
            };
            load_parameters(default_parameters);
            break;
        default:
            // load any other parameters
            load_url_with_parameters();
    }
}

function generate_url_with_parameters() {
    search = Array();
    for (const name of parameter_name_list) {
        const element_value = document.getElementById(get_parameter_id(name)).value;
        if (element_value) {
            search.push(`${name}=${element_value}`)
        }
    }
    const url = location.protocol + '//' + location.host + location.pathname + `?${search.join('&')}`;
    console.log('generated url with parameters:', url);
    navigator.clipboard.writeText(url);
}

function load_url_with_parameters() {
    const urlParams = new URLSearchParams(window.location.search);

    for (const name of parameter_name_list) {
        const value = urlParams.get(name);
        if (value) {
            document.getElementById(get_parameter_id(name)).value = value;
        }
    }
    const url = location.protocol + '//' + location.host + location.pathname;
    console.log('loaded url with parameters:', window.location.href)
    window.history.replaceState({}, '', url);
}

function collect_parameters() {
    let parameters = {};
    for (const name of parameter_name_list) {
        const element_value = document.getElementById(get_parameter_id(name)).value;
        if (element_value) {
            if (/[a-z]/i.test(element_value)) {
                parameters[name] = element_value;
            } else { // if parameter does not contain letters assume number
                parameters[name] = Number(element_value);
            }
        }
    }
    return parameters;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// postcode functions

async function postcode_onchange_callback() {
    // get postcode value and ensure consistent format (upper case, no spaces)
    let postcode_element = document.getElementById('sim-postcode');
    let postcode = postcode_element.value;
    postcode = postcode.toUpperCase().replace(' ', '');
    postcode_element.value = postcode;

    // if postcode api doesnt find longitude & latitude values could stop and not find address?
    await call_postcode_api(postcode);
    find_address();
}

async function call_postcode_api(postcode) {
    // call postcode api to get longitude and latitude values
    const postcode_url = 'https://api.postcodes.io/postcodes/' + postcode;
    fetch(postcode_url).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('postcode api data:', data);
                document.getElementById('sim-latitude').value = data.result.latitude;
                document.getElementById('sim-longitude').value = data.result.longitude;
                document.getElementById('sim-postcode').style.textDecorationLine = 'none';
                return true;
            } else {
                throw new Error(data['error']);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
            document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
            return false;
        });
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// epc functions

function find_address() {
    const postcode = document.getElementById('sim-postcode').value;
    fetch(`${epc_api_url}?postcode=${postcode}`).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('addresses: ', data);
                let element = document.getElementById('sim-addresses');
                //console.log(document.getElementsByTagName('option').length);
                while (element.getElementsByTagName('option').length > 0) {
                    element.removeChild(element.lastChild);
                }
                for (const [address, certificate] of data.result) {
                    //console.log(address, certificate);
                    let option_ele = document.createElement('option');
                    option_ele.value = certificate;
                    option_ele.text = address;
                    element.appendChild(option_ele);
                }
                get_epc_data();
            } else {
                throw new Error(data['error']);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });;
}

function get_epc_data() {
    let select = document.getElementById('sim-addresses');
    let certificate = select.options[select.selectedIndex].value;
    //console.log(certificate);

    fetch(`${epc_api_url}?certificate=${certificate}`).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('epc certificate: ', data);
                if (data['status'] == 200) {
                    const datar = data['result'];
                    if (datar['space-heating']) {
                        const space_heating = datar['space-heating'].match(/\d+/)[0];
                        document.getElementById('sim-space-heating').value = space_heating;
                    } else {
                        document.getElementById('sim-space-heating').value = null;
                    }
                    if (datar['floor-area']) {
                        const floor_area = datar['floor-area'].match(/\d+/)[0];
                        document.getElementById('sim-floor-area').value = floor_area;
                    } else {
                        document.getElementById('sim-floor-area').value = null;
                    }
                }
            } else {
                throw new Error(data['error']);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// net present cost functions

function calculate_net_present_cost(system) {
    return system["capital-expenditure"] + system["operational-expenditure"] * cumulative_discount_rate;
}

function calculate_cumulative_discount_rate(discount_rate, npc_years) {
    let discount_rate_current = 1;
    let cumulative_discount_rate = 0;
    for (let year = 0; year < npc_years; year++) {
        cumulative_discount_rate += 1.0 / discount_rate_current;
        discount_rate_current *= discount_rate;
    }
    return cumulative_discount_rate;
}

function lifetime_callback() {
    npc_years = document.getElementById('sim-lifetime').value;
    cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);
    recalculate_net_present_costs();
    render_simulation_table();
}

function recalculate_net_present_costs() {
    const systems = sim_output["systems"];
    for (let heat_option of heat_options_json) {
        let heat_system = systems[heat_option];
        //console.log(system, heat_option);
        switch (heat_option) {
            case "electric-boiler":
            case "air-source-heat-pump":
            case "ground-source-heat-pump":
            case "hydrogen-boiler":
            case "hydrogen-fuel-cell":
                for (let system of Object.values(heat_system)) {
                    system['net-present-cost'] = calculate_net_present_cost(system);
                }
                break;
            default:
                heat_option['net-present-cost'] = calculate_net_present_cost(heat_option);
        }
    }

    for (let system of Object.values(sim_output['systems'])) {

    }
    console.log(sim_output);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// submit, download, copy simulation functions

function submit_simulation() {
    document.getElementById('results').classList.add("hide");

    // get parameters from html and convert to json object
    let inputs = collect_parameters();
    const surf_opt = 'surface_optimisation';
    inputs[surf_opt] = document.getElementById(get_parameter_id(surf_opt)).checked;

    document.getElementById('sim-runtime').innerHTML = '...';
    console.log('simulation parameters:', inputs);
    document.getElementById('sim-submit').disabled = true;
    sim_worker.postMessage(["run simulation", inputs]);
}

function create_simulation_output_file_download() {
    let txt = JSON.stringify(sim_output, null, 2);
    var myBlob = new Blob([txt], { type: "text/plain" });

    var dlink = document.getElementById("download-link");
    var url = window.URL.createObjectURL(myBlob);
    dlink.href = url;
    dlink.download = "heatninja.json";
}

function clipboard_simulation_output() {
    let txt = JSON.stringify(sim_output, null, 2);
    navigator.clipboard.writeText(txt);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// rendering the simulation table functions

// the names of the heat options in the simulation output json object
const heat_options_json = ["electric-boiler", "air-source-heat-pump",
    "ground-source-heat-pump", "hydrogen-boiler", "hydrogen-fuel-cell",
    "biomass-boiler", "gas-boiler"];

// dict of heat option names to display in table
const heat_options_print = {
    "electric-boiler": "EleBo", "air-source-heat-pump": "ASHP",
    "ground-source-heat-pump": "GSHP", "hydrogen-boiler": "H2Bo", "hydrogen-fuel-cell": "H2FC",
    "biomass-boiler": "BioBo", "gas-boiler": "GasBo"
};

// the names of the solar options in the simulation output json object
const solar_options_json = ["none", "photovoltaic",
    "flat-plate", "evacuated-tube", "flat-plate-and-photovoltaic",
    "evacuated-tube-and-photovoltaic", "photovoltaic-thermal-hybrid"];

// dict of solar option names to display in table
const solar_options_print = {
    "none": "None", "photovoltaic": "PV",
    "flat-plate": "FP", "evacuated-tube": "ET", "flat-plate-and-photovoltaic": "FP+PV",
    "evacuated-tube-and-photovoltaic": "ET+PV", "photovoltaic-thermal-hybrid": "PVT"
};

function get_sorted_indices(array) {
    let value_index_pairs = [];
    for (let i in array) {
        value_index_pairs.push([array[i], i]);
    }
    value_index_pairs.sort(function (left, right) {
        return left[0] < right[0] ? -1 : 1;
    });
    let indexes = []; // sorted indexes
    let values = []; // sorted values
    for (let i in value_index_pairs) {
        values.push(Number(value_index_pairs[i][0]));
        indexes.push(Number(value_index_pairs[i][1]));
    }
    return indexes;
}

function fill_table_row(table, system) {
    let tr = document.createElement('tr');
    make_cell(tr, system[0]); // heat option
    make_cell(tr, system[1]); // solar / hydrogen option / none
    make_cells(tr, system[2]); // other system specs
    table.appendChild(tr);
}

function group_systems_callback() {
    let group_systems = document.getElementById('group-systems-input').checked;
    let element = document.getElementById('collapse-groups-input');

    if (!group_systems) {
        element.checked = false;
        element.disabled = true;
    } else {
        element.disabled = false;
    }
    render_simulation_table();
}

function sortby_callback() {
    const no_sort = Number(document.getElementById('sortby').value) == 0;
    document.getElementById('group-systems-input').disabled = no_sort;
    if (no_sort) { document.getElementById('collapse-groups-input').disabled = true; }
    group_systems_callback();
}

function collapse_groups_callback() {
    render_simulation_table();
}

function make_cell(tr, value) {
    //console.log("row", tr);
    let td = document.createElement('td');
    td.innerHTML = value == undefined ? '-' : value;
    tr.appendChild(td);
}

function make_cells(tr, system) {
    const headers = ["pv-size", "solar-thermal-size",
        "thermal-energy-storage-volume", "operational-expenditure", "capital-expenditure",
        "net-present-cost", "operational-emissions"];

    for (let key of headers) {
        switch (key) {
            case "net-present-cost":
            case "operational-expenditure":
            case "capital-expenditure":
                make_cell(tr, Math.round(system[key]));
                break;
            case "operational-emissions":
                make_cell(tr, Math.round(system[key] / 1000));
                break;
            default:
                make_cell(tr, system[key]);
        }
    }
    return tr;
}

function render_simulation_table() {
    // get results table and clear results
    let table = document.getElementById('sim-table');
    while (table.getElementsByTagName('tr').length > 1) {
        table.removeChild(table.lastChild);
    }

    let sortbyopt = document.getElementById('sortby');
    const sortby_index = Number(sortbyopt.value);

    const sorts = [
        "none",
        "operational-expenditure",
        "capital-expenditure",
        "net-present-cost",
        "operational-emissions"
    ];

    const sortname = sorts[sortby_index];

    // how do we sort? 
    // 1. get the values of sorting criteria from each system and put into an array called 'sortby_values'
    // 2. get an indices array of the sorted 'sortby_values' array
    // 3. use the indices array to add the systems to an array, called 'system_array' in the order determined by the sorting criteria
    const systems = sim_output["systems"];
    let sortby_values = [];
    let system_array = [];

    // not all heating systems have the same criteria, so they must each be formatted differently
    for (let heat_option of heat_options_json) {
        let heat_system = systems[heat_option];
        //console.log(system, heat_option);
        switch (heat_option) {
            case "electric-boiler":
            case "air-source-heat-pump":
            case "ground-source-heat-pump":
                for (const [solar_option, system] of Object.entries(heat_system)) {
                    if (sortby_index > 0) { sortby_values.push(system[sortname]); }
                    system_array.push([heat_options_print[heat_option], solar_options_print[solar_option], system]);
                }
                break;
            case "hydrogen-boiler":
            case "hydrogen-fuel-cell":
                for (const [h2_type, system] of Object.entries(heat_system)) {
                    if (sortby_index > 0) { sortby_values.push(system[sortname]); }
                    system_array.push([heat_options_print[heat_option], h2_type[0].toUpperCase() + h2_type.substring(1), system]);
                }
                break;
            default:
                if (sortby_index > 0) { sortby_values.push(heat_system[sortname]); }
                system_array.push([heat_options_print[heat_option], undefined, heat_system]);
                break;
        }
    }

    if (sortby_index > 0) {
        let indexes = get_sorted_indices(sortby_values);

        // if group by heating system then seperated the sorted indices into their respective heating groups
        const group_systems = document.getElementById('group-systems-input').checked;

        if (group_systems) {
            const group_ranges = [0, 7, 14, 21, 24, 27, 28, 29];
            let grouped_indexes = [[], [], [], [], [], [], []];
            let first_indexes = [];
            let firsts = [true, true, true, true, true, true, true];
            for (let i of indexes) {
                for (const group_index of firsts.keys()) {
                    console.log('group_index, range', group_index, group_ranges[group_index], group_ranges[group_index + 1]);
                    if (i >= group_ranges[group_index] && i < group_ranges[group_index + 1]) {
                        if (firsts[group_index]) { first_indexes.push(group_index); firsts[group_index] = false; }
                        grouped_indexes[group_index].push(i);
                    }
                }
            }
            console.log("grouped_indexes: ", grouped_indexes);
            console.log("first_indexes: ", first_indexes);
            let collapse_groups = document.getElementById('collapse-groups-input').checked;
            if (collapse_groups) {
                for (let j of first_indexes) {
                    let i = grouped_indexes[j][0]; // get only first system from each group
                    fill_table_row(table, system_array[i]);
                };
            } else {
                for (let j of first_indexes) {
                    for (let i of grouped_indexes[j]) { // display all sorted elements but per group
                        fill_table_row(table, system_array[i]);
                    }
                };
            }
        } else {
            for (let i of indexes) { // display all sorted elements (not grouped)
                fill_table_row(table, system_array[i]);
            };
        }
    } else {
        for (const system of system_array) { // display unsorted elements (grouped by default)
            fill_table_row(table, system);
        }
    }
}