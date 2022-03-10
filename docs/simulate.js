// GUIDE
// This file contains the js code for the simulator page, of which there are 4 parts:
// UI
// Input Form
// Graphical Results Elements
// Tabular Results Elements

console.log('simulate.js loaded');

// UI 
function toggle_nav_menu() {
    console.log("toggle nav menu");
    let menu = document.getElementById("nav-menu");
    let toggle = document.getElementById("nav-toggle");
    if (menu.classList.contains("open")) {
        for (let div of menu.getElementsByTagName("a")) {
            div.classList.remove("open");
            div.classList.add("close");
        }
        menu.classList.remove("open");
        menu.classList.add("close");
        toggle.classList.remove("open");
    } else {
        for (let div of menu.getElementsByTagName("a")) {
            div.classList.add("open");
            div.classList.remove("close");
        }
        menu.classList.add("open");
        menu.classList.remove("close");
        toggle.classList.add("open");
    }
}

// ---- SIMULATION INPUTS
// -- this section includes the code for the functionality of the simulation inputs


// ---- TERMS
// id = javascript id, as per document.getElementById(id).
// pid = partial id, corresponding to the name of the input lists


// ---- FUNCTIONS GUIDE
// clear_warnings(pid)
// -- clear all warning related to an input pid
// async function check_input(pid, transform, conditions)
// -- input validation
// input_help(pid)
// -- toggle help/info for input pid
// hide_ids(ids), hide_pids(pids), hide_elements(elements)
// -- hide all ids/pids/elements in the list
// unhide_ids(ids), unhide_pids(pids), unhide_elements(elements)
// -- unhide all ids/pids/elements in the list
// clear_value(pid), set_value(pid, value)
// -- clear/set the value of a pid
// validate_element(element), invalidate_element(element), clear_element_validation(element)
// -- add and remove valid and invalid class to element
// async onchange_address()
// -- update warning, epc_calling ... upon selection address change
// check_submit()
// -- check if all inputs are valid so simulation can be submitted
// toggle_advanced_inputs()
// -- toggle advanced inputs but dont reset them
// click_dismiss()
// -- any element with click-dismiss class automatically has click event applied with pointer styling
// set_run_location()
// -- set where simulation will run, client or server-side
// toggle_optimisation()
// -- toggle the checkbox for optimisation
// async get_epc_data()
// -- call the EPC API to get the epc_space_heating and floor_area for a given EPC certificate
// get_check_input_fnc(pid, apply_transform)
// -- returns a function that builds the check_input function, with option of including the transform function
// hide_postcode_related_inputs()
// -- if postcode is not valid then hide address, floor area and epc space heating inputs
// check_postcode_format(postcode)
// -- only call postcodes.io if outcode is of valid format
// async validate_postcode(postcode)
// -- call postcodes.io to get longitude and latitude data for postcode
// async get_address_certificates(postcode)
// -- get a list of addresses and their respective certificate codes for postcode
// show_manual_epc_input()
// -- if postcode is located in scotland or api request failed then fallback to manual floor area and space heating input
// submit_simulation()
// -- ran when submit button clicks. Will submit if all inputs valid.
// async submit_simulation_server()
// -- submit inputs to server to run simulation

// ---- GLOBALS
// let api_url = 'http://localhost:3000';
let api_url = 'https://customapi.heatmyhome.ninja';
let epc_api_url = api_url + '/epc';
let simulate_api_url = api_url + '/simulate';
let submit_status = false;

const input_ranges = { // MIN, MAX, MULTIPLIER
    'temperature': [0, 35, 10],
    'occupants': [1, 20, 1],
    'tes-volume': [0.1, 3.0, 10],
    'epc-space-heating': [0, 999999, 1],
    'floor-area': [25, 1500, 1],
}

let input_values = {
    'postcode': undefined,
    'longitude': undefined,
    'latitude': undefined,
    'epc-space-heating': undefined,
    'floor-area': undefined,
    'temperature': undefined,
    'occupants': undefined,
    'tes-volume': undefined,
    'run-on': 'server-rust',
    'enable-optimisation': true,
}

let scottish_postcode = false;
let epc_api_connection = true;
let epc_api_error = false;
let selected_certificate = undefined;
const input_id_list = ['postcode', 'epc-space-heating', 'floor-area', 'temperature', 'occupants', 'tes-volume', 'neighbour-postcode'];

// ---- INITIALISATION
// apply validation functions to oninput and onchange events for each input (excluding selections and checkboxes)
// run validation incase page was reloaded
for (let input_id of input_id_list) {
    let element = document.getElementById('input-' + input_id);
    if (input_id != 'postcode' && input_id != 'neighbour-postcode') {
        element.addEventListener('change', get_check_input_fnc(input_id, true));
    }
    element.addEventListener('input', get_check_input_fnc(input_id, false));
}

// apply onclick events to all elements with the click-dismiss class
click_dismiss();

// update epc url links
update_epc_urls();
update_neighbours_epc_urls();

// reset run-on select, must add delay otherwise autofill fills old value after reset.
setTimeout(() => {
    console.log('reset');
    document.getElementById('run-location').value = 'server-rust';
}, 10);

// ---- FUNCTIONS

function clear_warnings(pid) {
    let input_box_element = document.getElementById("input-box-" + pid);
    let warn_elements = input_box_element.getElementsByClassName("warn");
    for (let warn_element of warn_elements) {
        warn_element.classList.add("hide");
    }
}

async function check_input(pid, transform, conditions) {
    let input_element = document.getElementById("input-" + pid);
    let help_element = document.getElementById("help-" + pid);
    help_element.classList.add("hide");
    document.getElementById("input-box-" + pid).getElementsByClassName("input-side-button")[0].classList.remove("active");
    // console.log('check_input: ', pid, 'value: ', input_element.value);

    if (input_element.value == "") {
        clear_element_validation(input_element);
        clear_warnings(pid);
    } else {
        if (transform != undefined) {
            input_element.value = transform(input_element.value);
        }

        let is_valid = true;
        for (let condition of conditions) {
            let warn_pid = await condition(input_element.value);
            if (warn_pid != "") {
                is_valid = false;
                clear_warnings(pid);
                if (warn_pid != "none") {
                    console.log("pid: ", pid, ", warn_pid: ", warn_pid, "condition", condition);
                    let warn_element = document.getElementById("warn-" + pid + '-' + warn_pid);
                    warn_element.classList.remove("hide");
                }
                break;
            }
        }

        if (is_valid) {
            validate_element(input_element);
            if (pid != 'neighbour-postcode') {
                if (/[a-z]/i.test(input_element.value)) {
                    input_values[pid] = input_element.value;
                } else { // if parameter does not contain letters assume number
                    input_values[pid] = Number(input_element.value);
                }
                clear_warnings(pid);
            }
        } else {
            invalidate_element(input_element);
        }
    }
    check_submit();
}

function neighbour_text(is_neighbour) {
    if (is_neighbour) {
        return "neighbour-";
    } else {
        return "";
    }
}

function input_help(pid, is_neighbour) {
    console.log(is_neighbour, neighbour_text(is_neighbour));
    let help_button = document.getElementById("input-box-" + neighbour_text(is_neighbour) + pid).getElementsByClassName("input-side-button")[0];

    let help_msg = document.getElementById("help-" + neighbour_text(is_neighbour) + pid);
    if (help_msg.classList.contains("hide")) {
        help_msg.classList.remove("hide");
        help_button.classList.add("active");
    } else {
        help_msg.classList.add("hide");
        help_button.classList.remove("active");
    }
}

function hide_ids(ids) {
    for (let id of ids) {
        document.getElementById(id).classList.add("hide");
    }
}

function unhide_ids(ids) {
    for (let id of ids) {
        document.getElementById(id).classList.remove("hide");
    }
}

function hide_pids(pids) {
    for (let pid of pids) {
        document.getElementById('input-' + pid).classList.add("hide");
    }
}

function unhide_pids(pids) {
    for (let pid of pids) {
        document.getElementById('input-' + pid).classList.remove("hide");
    }
}

function hide_elements(elements) {
    for (let element of elements) {
        element.classList.add("hide");
    }
}

function unhide_elements(elements) {
    for (let element of elements) {
        element.classList.remove("hide");
    }
}

function clear_value(pid) {
    document.getElementById('input-' + pid).value = "";
}

function set_value(pid, value) {
    document.getElementById('input-' + pid).value = value;
}

function validate_element(element) {
    element.classList.add("valid");
    element.classList.remove("invalid");
}

function invalidate_element(element) {
    element.classList.remove("valid");
    element.classList.add("invalid");
}

function clear_element_validation(element) {
    element.classList.remove("valid", "invalid");
}

async function onchange_address() {
    let address_element = document.getElementById("input-address");
    let warn_element = document.getElementById("warn-address-not-listed");
    let epc_box_element = document.getElementById("input-box-epc-space-heating");
    let floor_area_box_element = document.getElementById("input-box-floor-area");
    let searching = document.getElementById("epc-searching");

    clear_value('epc-space-heating');
    get_check_input_fnc('epc-space-heating', false)();
    clear_value('floor-area');
    get_check_input_fnc('floor-area', false)();
    clear_warnings("address");
    document.getElementById("input-postcode").parentElement.getElementsByTagName('p')[0].innerText = 'Postcode';
    hide_ids(['help-address', 'neighbour-container', 'neighbour-title']);

    selected_certificate = undefined;
    switch (address_element.value) {
        case "Select Address":
            clear_element_validation(address_element);
            hide_elements([warn_element, epc_box_element, floor_area_box_element]);
            break;
        case "Address Not Listed":
            invalidate_element(address_element);
            unhide_elements([warn_element, epc_box_element, floor_area_box_element]);
            open_neighbour_menu();
            break;
        default:
            validate_element(address_element);
            hide_elements([warn_element, epc_box_element, floor_area_box_element]);
            unhide_elements([searching]);
            await get_epc_data();
    }
    update_epc_urls();
}

async function onchange_neighbours_address() {
    let address_element = document.getElementById("input-neighbour-address");
    let epc_box_element = document.getElementById("input-box-neighbour-epc-space-heating");
    let floor_area_box_element = document.getElementById("input-box-neighbour-floor-area");
    let searching = document.getElementById("neighbour-epc-searching");

    clear_value('neighbour-epc-space-heating');
    clear_value('neighbour-floor-area');
    clear_element_validation(document.getElementById('input-neighbour-epc-space-heating'));
    clear_element_validation(document.getElementById('input-neighbour-floor-area'));

    hide_ids(['warn-neighbour-no-data', 'warn-neighbour-address-unknown', 'warn-neighbour-address-connection']);

    switch (address_element.value) {
        case "Select Address":
            clear_element_validation(address_element);
            hide_elements([epc_box_element, floor_area_box_element]);
            break;
        default:
            validate_element(address_element);
            hide_elements([epc_box_element, floor_area_box_element]);
            unhide_elements([searching]);
            await get_neighbour_epc_data();
    }
    update_neighbours_epc_urls();
}

function fill_neighbour_address() {
    let address_element = document.getElementById('input-neighbour-address');
    while (address_element.getElementsByTagName('option').length > 0) {
        address_element.removeChild(address_element.lastChild);
    }
    clear_element_validation(address_element);

    let opt1 = document.createElement('option');
    opt1.text = "Select Address";
    opt1.classList.add("color-neutral");
    address_element.appendChild(opt1);

    for (let [address, certificate] of address_certificate_list) {
        //console.log(address, certificate);
        let option_element = document.createElement('option');
        option_element.value = certificate;
        option_element.classList.add("color-black");
        // shorten address and remove any trailing commas or spaces
        option_element.text = address.substring(0, 45).replace(/,\s*$/, "");
        address_element.appendChild(option_element);
    }
    unhide_elements([address_element]);
}

function input_apply(pid) {
    document.getElementById('input-' + pid).value = document.getElementById('input-neighbour-' + pid).value;
    get_check_input_fnc(pid, false)();
}

function open_neighbour_menu() {
    let n_postcode_element = document.getElementById('input-neighbour-postcode');
    n_postcode_element.value = input_values.postcode;
    validate_element(n_postcode_element);
    fill_neighbour_address();
    document.getElementById("input-postcode").parentElement.getElementsByTagName('p')[0].innerText = 'Your Postcode';
    unhide_ids(['neighbour-container', 'neighbour-title']);
    // hide_ids(['warn-address-not-listed', 'warn-address-missing-data']);
}

function submit_simulation() {
    let submit_input = document.getElementById("input-submit");
    if (!submit_input.classList.contains('invalid')) {
        submit_input.classList.add('active');
        hide_ids(['warn-sim-api-connection', 'warn-sim-api-error', 'warn-sim-api-busy']);
        console.log("submit-simulation: ", input_values);

        switch (input_values["run-on"]) {
            case 'server-rust':
                submit_simulation_server();
                break;
            case 'client-rust':
                submit_simulation_rust();
                break;
            case 'client-cpp':
                submit_simulation_cpp();
                break;
        }
    } else {
        unhide_ids(['warn-submit']);
    }
}

// setTimeout(() => {
//     input_values = {
//         'postcode': 'HP160LU',
//         'longitude': -1.5833,
//         'latitude': 52.3833,
//         'epc-space-heating': 3000.0,
//         'floor-area': 60.0,
//         'temperature': 20.0,
//         'occupants': 2,
//         'tes-volume': 0.5,
//         'run-on': 'client-rust',
//         'enable-optimisation': true,
//     }
//     submit_simulation_rust();
// }, 1000);

function submit_simulation_cpp() {
    console.log('submit_simulation_cpp');
    unhide_ids(['submit-waiting']);
    hide_ids(['submit-complete']);
    if (window.Worker) {
        var worker_cpp = new Worker('./worker_cpp.js');

        worker_cpp.onmessage = function (e) {
            if (!e.data) {
                console.error("Cpp simulator failed: ", e.data);
                unhide_ids(['warn-sim-cpp-error']);
            } else {
                worker_cpp.terminate();
                clearTimeout(simulation_timeout);
                console.log('simulator-cpp-json:', JSON.parse(e.data));
                unhide_ids(['submit-complete']);
                hide_ids(['guide']);
                output = JSON.parse(e.data);
                load_output();
            }
            hide_ids(['submit-waiting']);
            document.getElementById("input-submit").classList.remove('active');
        }
        let simulation_timeout = setTimeout(() => {
            console.error("Rust simulator exceeded time limit");
            hide_ids(['submit-waiting']);
            unhide_ids(['warn-sim-client-timeout']);
            document.getElementById("input-submit").classList.remove('active');
        }, 600000);
        worker_cpp.postMessage(input_values);
    } else {
        console.error("Rust-Sim-Worker-Unsupported");
        unhide_ids(['warn-sim-client-worker']);
    }
}

function submit_simulation_rust() {
    console.log('submit_simulation_rust');
    unhide_ids(['submit-waiting']);
    hide_ids(['submit-complete']);
    if (window.Worker) {
        var worker = new Worker('./worker.js', { type: "module" });

        worker.onmessage = function (e) {
            if (!e.data) {
                console.error("Rust simulator failed: ", e.data);
                unhide_ids(['warn-sim-rust-error']);
            } else {
                worker.terminate();
                clearTimeout(simulation_timeout);
                console.log('simulator-rust-json:', JSON.parse(e.data));
                unhide_ids(['submit-complete']);
                hide_ids(['guide']);
                output = JSON.parse(e.data);
                load_output();
            }
            hide_ids(['submit-waiting']);
            document.getElementById("input-submit").classList.remove('active');
        }
        let simulation_timeout = setTimeout(() => {
            console.error("Rust simulator exceeded time limit");
            hide_ids(['submit-waiting']);
            unhide_ids(['warn-sim-client-timeout']);
            document.getElementById("input-submit").classList.remove('active');
        }, 600000);
        worker.postMessage(input_values);
    } else {
        console.error("Rust-Sim-Worker-Unsupported");
        unhide_ids(['warn-sim-client-worker']);
    }
}

let input_url_names = {
    'postcode': 'postcode',
    'epc-space-heating': 'space_heating',
    'floor-area': 'floor_area',
    'temperature': 'temperature',
    'occupants': 'occupants',
    'tes-volume': 'tes_max',
}

let simulator_input_url = undefined;
function generate_input_url() {
    search = Array();
    for (const [key, value] of Object.entries(input_url_names)) {
        // console.log(key, value, input_values[key]);
        search.push(`${value}=${input_values[key]}`);
    }

    simulator_input_url = location.protocol + '//' + location.host + location.pathname + `?${search.join('&')}`;
    console.log('created-simulator-input-url: ', simulator_input_url);
}

let simulator_api_full_url = undefined;
function generate_sim_api_url() {
    search = Array();
    let api_url_names = {
        'postcode': 'postcode',
        'latitude': 'latitude',
        'longitude': 'longitude',
        'epc-space-heating': 'space_heating',
        'floor-area': 'floor_area',
        'temperature': 'temperature',
        'occupants': 'occupants',
        'tes-volume': 'tes_max',
    }

    for (const [key, value] of Object.entries(api_url_names)) {
        // console.log(key, value, input_values[key]);
        search.push(`${value}=${input_values[key]}`);
    }

    simulator_api_full_url = simulate_api_url + `?${search.join('&')}`;
    console.log('created-simulator-api-url: ', simulator_api_full_url);
}

function copy_input_url_to_clipboard() {
    console.log('copied-simulator-url: ', simulator_input_url);
    navigator.clipboard.writeText(simulator_input_url);
}

let loading_inputs_from_url = false;
load_inputs_from_url();
function load_inputs_from_url() {
    const urlParams = new URLSearchParams(window.location.search);
    if (Array.from(urlParams.keys()).length > 0) {
        let url_values = {
            'postcode': undefined,
            'epc-space-heating': undefined,
            'floor-area': undefined,
            'temperature': undefined,
            'occupants': undefined,
            'tes-volume': undefined,
        }

        for (const [form_name, url_name] of Object.entries(input_url_names)) {
            const value = urlParams.get(url_name);
            if (value) {
                url_values[form_name] = value;
            } else {
                console.error("Not all parameters contained in url: ", url_name, value);
                return;
            }
        }

        loading_inputs_from_url = true;
        for (let [key, value] of Object.entries(url_values)) {
            let element = document.getElementById('input-' + key);
            element.value = value;
            get_check_input_fnc(key, true)();
        }
        unhide_pids(['box-epc-space-heating', 'box-floor-area']);

        const url = location.protocol + '//' + location.host + location.pathname;
        console.log('loaded url with parameters:', window.location.href)
        window.history.replaceState({}, '', url);
    }
}

function load_inputs_from_file(file_inputs) {
    let file_values = {
        'postcode': undefined,
        'epc-space-heating': undefined,
        'floor-area': undefined,
        'temperature': undefined,
        'occupants': undefined,
        'tes-volume': undefined,
    }

    for (const name of Object.keys(file_values)) {
        const value = file_inputs[name];
        if (value) {
            file_values[name] = value;
        } else {
            console.error("Not all parameters contained in file: ", name, value);
            return;
        }
    }

    loading_inputs_from_url = true;
    for (let [key, value] of Object.entries(file_values)) {
        let element = document.getElementById('input-' + key);
        element.value = value;
        get_check_input_fnc(key, true)();
    }
    unhide_pids(['box-epc-space-heating', 'box-floor-area']);

    const url = location.protocol + '//' + location.host + location.pathname;
    console.log('loaded url with parameters:', window.location.href)
    window.history.replaceState({}, '', url);
}

async function submit_simulation_server() {
    let submit_input = document.getElementById("input-submit");
    unhide_ids(['submit-waiting']);
    hide_ids(['submit-complete']);

    try {
        const response = await fetch(simulator_api_full_url);
        const json = await response.json();
        if (json['status'] == 200) {
            console.log('simulator-api-json:', json);
            unhide_ids(['submit-complete']);
            hide_ids(['guide']);
            output = json['result'];
            load_output();
        } else {
            throw new Error(json['error']);
        }
    } catch (error) {
        console.error('simulator-api-error: ', error);
        if (error.message == 'Failed to fetch' || error.message == 'Load failed') {
            unhide_ids(['warn-sim-api-connection']);
        } else if (error.message.startsWith('simulation exceeded allowed runtime')) {
            unhide_ids(['warn-sim-api-busy']);
        }
        else {
            unhide_ids(['warn-sim-api-error']);
        }
    }
    hide_ids(['submit-waiting']);
    submit_input.classList.remove('active');
}

function create_simulation_output_file_download() {
    let data = {
        'inputs': input_values,
        'outputs': output,
    }
    let txt = JSON.stringify(data, null, 2);
    let blob = new Blob([txt], { type: "text/plain" });

    let dlink = document.getElementById("download-link");
    let url = window.URL.createObjectURL(blob);
    dlink.href = url;
    dlink.download = "HeatMyHome.txt";
}

function upload_results() {
    const reader = new FileReader();
    reader.onload = function (e) {
        file_data = JSON.parse(this.result);
        load_inputs_from_file(file_data.inputs);
        output = file_data.outputs
        console.log("Loaded data from file: ", file_data);
        load_output();
    }
    reader.readAsText(this.files[0]);
    this.value = null;
}
document.getElementById("load-results").addEventListener('change', upload_results, false);


function check_submit() {
    let submit = true;
    for (let pid of input_id_list) {
        if (pid != 'neighbour-postcode') {
            let element = document.getElementById('input-' + pid);
            if (!element.classList.contains("valid")) {
                submit = false;
            }
        }
    }
    if (submit) {
        generate_sim_api_url();
        generate_input_url();
        unhide_ids(['input-box-save-url']);
    } else {
        hide_ids(['input-box-save-url']);
    }

    if (submit != submit_status) {
        submit_status = submit;
        // let submit_element = document.getElementById('submit-group');
        // let advanced_element = submit_element.getElementsByClassName("input-side-button")[0];
        let submit_input = document.getElementById("input-submit");

        if (submit) {
            submit_input.classList.remove('invalid');
            hide_ids(['warn-submit']);
        } else {
            submit_input.classList.add('invalid');
        }
    }
}

function toggle_advanced_inputs() {
    let run_location = document.getElementById('run-location');
    let submit_element = document.getElementById('submit-group');
    let button = submit_element.getElementsByClassName("input-side-button")[0];
    if (run_location.classList.contains("hide")) {
        unhide_elements([run_location]);
        button.classList.add("active");
        unhide_ids(['run-location', 'help-advanced']);
        set_run_location();
    } else {
        button.classList.remove("active");
        hide_ids(['run-location', 'help-advanced', 'input-box-optimisation']);
    }
}

function click_dismiss() {
    let elements = document.getElementsByClassName('click-dismiss');
    for (let element of elements) {
        element.addEventListener('click', () => { document.getElementById(element.id).classList.add('hide') });
    }
}

function set_run_location() {
    let element = document.getElementById("run-location");
    let optimisation_element = document.getElementById("input-box-optimisation");
    let value = element.getElementsByTagName("option")[element.selectedIndex].value;
    input_values["run-on"] = value;

    switch (value) {
        case 'server-rust':
            hide_elements([optimisation_element]);
            break;
        default:
            unhide_elements([optimisation_element]);
    }
}

function toggle_optimisation() {
    let element = document.getElementById("input-optimisation");
    let box = document.getElementById("input-box-optimisation");
    let divs = element.getElementsByTagName('div');

    if (box.classList.contains("ticked")) {
        box.classList.remove("ticked");
        input_values["enable-optimisation"] = false;
    } else {
        box.classList.add("ticked");
        input_values["enable-optimisation"] = true;
    }

    for (let div of divs) {
        if (div.classList.contains("checkmark")) {
            div.classList.add("crossmark");
            div.classList.remove("checkmark");
        } else {
            div.classList.remove("crossmark");
            div.classList.add("checkmark");
        }
    }
}

async function get_epc_data() {
    let select = document.getElementById('input-address');
    selected_certificate = select.options[select.selectedIndex].value;
    const full_url = `${epc_api_url}?certificate=${selected_certificate}`;

    try {
        const response = await fetch(full_url);
        const json = await response.json();
        if (json['status'] == 200) {
            console.log('epc-certificate-json: ', json);
            const result = json['result'];

            if (result['space-heating']) {
                set_value('epc-space-heating', result['space-heating'].match(/\d+/)[0]);
                get_check_input_fnc('epc-space-heating', false)();
            } else {
                clear_value('epc-space-heating');
                // unhide_ids(['warn-epc-space-heating-none']);
            }

            if (result['floor-area']) {
                set_value('floor-area', result['floor-area'].match(/\d+/)[0]);
                get_check_input_fnc('floor-area', false)();
            } else {
                clear_value('floor-area');
                // unhide_ids(['warn-floor-area-none']);
            }

            if (result['space-heating'] && result['floor-area']) {
                document.getElementById('help-address').innerHTML = `Your address was used to fill out the space heating estimate and floor
                area parameters from your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>.
                You may wish to adjust the parameters manually.`
                unhide_ids(['help-address']);
            } else if (result['space-heating'] && !result['floor-area']) {
                document.getElementById('help-address').innerHTML = `Your address was used to fill out the space heating estimate
                from your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>. 
                You may wish to adjust the parameter manually.`

                document.getElementById('warn-address-missing-data').innerHTML = ` Your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>
                does not contain a floor area estimate. Enter it manually or select a neighbour's address whose home is similar in size to yours.`;
                unhide_ids(['help-address', 'warn-address-missing-data', 'input-box-epc-space-heating']);
                open_neighbour_menu();
            } else if (!result['space-heating'] && result['floor-area']) {
                document.getElementById('help-address').innerHTML = `Your address was used to fill out the floor area parameter
                from your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>. 
                You may wish to adjust the parameter manually.`

                document.getElementById('warn-address-missing-data').innerHTML = ` Your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>
                does not contain a space heating estimate. Enter it manually or select a neighbour's address whose home has similar heating and hot water requirements to yours.`;
                unhide_ids(['help-address', 'warn-address-missing-data', 'input-box-floor-area']);
                open_neighbour_menu();
            } else { // neither
                document.getElementById('warn-address-missing-data').innerHTML = ` Your home's <a href="" class="epc-url" target="_blank" rel="noopener noreferrer">EPC</a>
                does not contain a space heating or floor area estimate. Enter them manually or select a neighbour's address whose home is of similar size and has similar heating and hot water requirements to yours.`;
                unhide_ids(['warn-address-missing-data', 'input-box-floor-area']);
                open_neighbour_menu();
            }
            hide_ids(['epc-searching', 'warn-address-not-listed']);
            unhide_ids(['input-box-epc-space-heating', 'input-box-floor-area']);
        } else {
            throw new Error(json['error']);
        }
    }
    catch (error) {
        console.error('epc-certificate-json-error: ', error);
        if (error.message == "Failed to fetch") {
            unhide_elements(['warn-address-connection']);
        } else {
            unhide_elements(['warn-address-unknown']);
        }
    }
}

async function get_neighbour_epc_data() {
    let select = document.getElementById('input-neighbour-address');
    let certificate = select.options[select.selectedIndex].value;
    const full_url = `${epc_api_url}?certificate=${certificate}`;

    try {
        const response = await fetch(full_url);
        const json = await response.json();
        if (json['status'] == 200) {
            console.log('epc-certificate-json: ', json);
            const result = json['result'];

            let warn_no_data = document.getElementById('warn-neighbour-no-data');
            let have_valid_space_heating = document.getElementById('input-epc-space-heating').classList.contains('valid');
            let have_valid_floor_area = document.getElementById('input-floor-area').classList.contains('valid');

            console.log(have_valid_space_heating, have_valid_floor_area);
            if (result['space-heating']) {
                set_value('neighbour-epc-space-heating', result['space-heating'].match(/\d+/)[0]);
                validate_element(document.getElementById('input-neighbour-epc-space-heating'));
                unhide_ids(['input-box-neighbour-epc-space-heating']);
            } else {
                if (have_valid_space_heating) {
                    clear_value('neighbour-epc-space-heating');
                    clear_element_validation(document.getElementById('input-neighbour-epc-space-heating'));
                    hide_ids(['input-box-neighbour-epc-space-heating']);
                }
            }


            if (result['floor-area']) {
                set_value('neighbour-floor-area', result['floor-area'].match(/\d+/)[0]);
                validate_element(document.getElementById('input-neighbour-floor-area'));
                unhide_ids(['input-box-neighbour-floor-area']);
            } else {
                if (have_valid_floor_area) {
                    clear_value('neighbour-floor-area');
                    clear_element_validation(document.getElementById('input-neighbour-floor-area'));
                    hide_ids(['input-box-neighbour-floor-area']);
                }
            }

            if (!have_valid_space_heating && !have_valid_floor_area && !result['space-heating'] && !result['floor-area']) {
                warn_no_data.innerHTML = ` Neighbour's <a href="" class="epc-url-neighbour" target="_blank" rel="noopener noreferrer">EPC</a>
                    does not contain a space heating or floor estimate. Select a different neighbour.`;
                warn_no_data.classList.remove('hide');
            } else if (!have_valid_space_heating && !result['space-heating']) {
                warn_no_data.innerHTML = ` Neighbour's <a href="" class="epc-url-neighbour" target="_blank" rel="noopener noreferrer">EPC</a>
                    does not contain a space heating estimate. Select a different neighbour.`;
                warn_no_data.classList.remove('hide');
            } else if (!have_valid_floor_area && !result['floor-area']) {
                warn_no_data.innerHTML = ` Neighbour's <a href="" class="epc-url-neighbour" target="_blank" rel="noopener noreferrer">EPC</a>
                    does not contain a floor area estimate. Select a different neighbour.`;
                warn_no_data.classList.remove('hide');
            }


            hide_ids(['neighbour-epc-searching']);
        } else {
            throw new Error(json['error']);
        }
    }
    catch (error) {
        console.error('epc-certificate-json-error: ', error);
        if (error.message == "Failed to fetch") {
            unhide_elements([document.getElementById('warn-neighbour-address-connection')]);
        } else {
            unhide_elements([document.getElementById('warn-neighbour-address-unknown')]);
        }
        hide_ids(['neighbour-epc-searching']);
    }
}

function get_check_input_fnc(pid, apply_transform) {
    switch (pid) {
        case 'postcode':
            return async () => {
                let searching = document.getElementById("postcode-searching");
                unhide_elements([searching]);
                invalidate_element(document.getElementById('input-postcode'));
                await check_input("postcode",
                    (postcode) => { return postcode.toUpperCase().replace(/\s/g, '').substring(0, 7); },
                    [
                        hide_postcode_related_inputs,
                        (postcode) => { return check_postcode_format(postcode); },
                        async (postcode) => { return validate_postcode(postcode); },
                        async (postcode) => { return get_address_certificates(postcode); },
                        show_manual_epc_input,
                    ]
                );
                hide_elements([searching]);
                if (!epc_api_connection) {
                    unhide_ids(['warn-postcode-epc-connection']);
                } else if (epc_api_error) {
                    unhide_ids(['warn-postcode-epc-api']);
                }
                loading_inputs_from_url = false;
                update_epc_urls();
            };
            break;
        case 'neighbour-postcode':
            return async () => {
                let searching = document.getElementById("neighbour-postcode-searching");
                unhide_elements([searching]);
                await check_input("neighbour-postcode",
                    (postcode) => { return postcode.toUpperCase().replace(/\s/g, '').substring(0, 7); },
                    [
                        hide_neighbours_postcode_related_inputs,
                        (postcode) => { return check_postcode_format(postcode); },
                        async (postcode) => { return validate_neighbours_postcode(postcode); },
                        async (postcode) => { return get_neighbours_address_certificates(postcode); },
                        show_manual_epc_input,
                    ]
                );
                hide_elements([searching]);
                if (!epc_api_connection) {
                    unhide_ids(['warn-neighbour-postcode-epc-connection']);
                } else if (epc_api_error) {
                    unhide_ids(['warn-neighbour-postcode-epc-api']);
                }
                update_neighbours_epc_urls();
            };
            break;
        default:
            const [min_input, max_input, multipler] = input_ranges[pid];
            return () =>
                check_input(pid,
                    apply_transform ? (value) => { return Math.round(Math.min(Math.max(value, min_input), max_input) * multipler) / multipler; } : undefined,
                    [
                        (value) => { if (value >= min_input && value <= max_input) { return ""; } else { return "range" } },
                    ]
                );
    }
}

function hide_postcode_related_inputs() {
    scottish_postcode = false;
    epc_api_connection = true;
    epc_api_error = false;
    selected_certificate = undefined;
    document.getElementById("input-postcode").parentElement.getElementsByTagName('p')[0].innerText = 'Postcode';
    hide_ids(['input-address', 'help-scottish-postcode', 'input-box-epc-space-heating', 'input-box-floor-area', 'epc-searching', 'help-address', 'neighbour-container', 'neighbour-title']);
    clear_warnings('address');
    clear_value('epc-space-heating');
    get_check_input_fnc('epc-space-heating', false)();
    clear_value('floor-area');
    get_check_input_fnc('floor-area', false)();
    return "";
}

function hide_neighbours_postcode_related_inputs() {
    scottish_postcode = false;
    epc_api_connection = true;
    epc_api_error = false;
    hide_ids(['input-neighbour-address', 'warn-neighbour-scottish-postcode', 'input-box-neighbour-epc-space-heating', 'input-box-neighbour-floor-area', 'neighbour-epc-searching']);
    clear_warnings('neighbour-address');
    return "";
}

async function validate_postcode(postcode) {
    const postcode_url = 'https://api.postcodes.io/postcodes/' + postcode;
    try {
        const response = await fetch(postcode_url);
        const json = await response.json();
        if (json['status'] == 200) {
            console.log('postcode-api-json:', json);
            if (json.result.latitude != null && json.result.longitude != null) {
                if (json.result.country == "Scotland") {
                    scottish_postcode = true;
                    unhide_ids(['help-scottish-postcode']);
                }
                input_values.latitude = Number(json.result.latitude);
                input_values.longitude = Number(json.result.longitude);
                return "";
            } else {
                throw new Error('Postcode found on API, but does not have an associated latitude and longitude.');
            }
        } else {
            throw new Error(json['error']);
        }
    } catch (error) {
        console.error('postcode-api-error: ', error);
        if (error.message == 'Failed to fetch' || error.message == 'Load failed') {
            return 'io-connection';
        }
        return 'postcodes-io';
    }
}

async function validate_neighbours_postcode(postcode) {
    const postcode_url = 'https://api.postcodes.io/postcodes/' + postcode;
    try {
        const response = await fetch(postcode_url);
        const json = await response.json();
        if (json['status'] == 200) {
            console.log('postcode-api-json:', json);
            if (json.result.country == "Scotland") {
                scottish_postcode = true;
                unhide_ids(['warn-neighbour-scottish-postcode']);
            }
            return "";
        } else {
            throw new Error(json['error']);
        }
    } catch (error) {
        console.error('postcode-api-error: ', error);
        if (error.message == 'Failed to fetch' || error.message == 'Load failed') {
            return 'io-connection';
        }
        return 'postcodes-io';
    }
}

let address_certificate_list = [];
async function get_address_certificates(postcode) {
    address_certificate_list = [];
    if (!scottish_postcode && !loading_inputs_from_url) {
        const full_url = `${epc_api_url}?postcode=${postcode}`;

        try {
            const response = await fetch(full_url);
            const json = await response.json();
            if (json['status'] == 200) {
                console.log('api-address-certificate-json: ', json);
                let address_element = document.getElementById('input-address');
                while (address_element.getElementsByTagName('option').length > 0) {
                    address_element.removeChild(address_element.lastChild);
                }
                clear_element_validation(address_element);

                let opt1 = document.createElement('option');
                opt1.text = "Select Address";
                opt1.classList.add("color-neutral");
                address_element.appendChild(opt1);
                let opt2 = document.createElement('option');
                opt2.text = "Address Not Listed";
                opt2.classList.add("color-warn");
                address_element.appendChild(opt2);

                for (let [address, certificate] of json.result) {
                    //console.log(address, certificate);
                    let option_element = document.createElement('option');
                    option_element.value = certificate;
                    option_element.classList.add("color-black");

                    // capitalise each word
                    address = address.toLowerCase().replace(/(^\w{1})|(\s+\w{1})/g, letter => letter.toUpperCase());

                    address_certificate_list.push([address, certificate]);
                    // shorten address and remove any trailing commas or spaces
                    option_element.text = address.substring(0, 45).replace(/,\s*$/, "");
                    address_element.appendChild(option_element);
                }
                unhide_elements([address_element]);
                return "";
            } else {
                throw new Error(json['error']);
            }
        } catch (error) {
            console.error('api-address-certificate-error: ', error);
            if (error.message == 'Failed to fetch' || error.message == 'Load failed') {
                epc_api_connection = false;
                return "";
            }
            epc_api_error = true;
            return "";
        }
    }
    return "";
}

async function get_neighbours_address_certificates(postcode) {
    if (!scottish_postcode) {
        const full_url = `${epc_api_url}?postcode=${postcode}`;

        try {
            const response = await fetch(full_url);
            const json = await response.json();
            if (json['status'] == 200) {
                console.log('api-address-certificate-json: ', json);
                let address_element = document.getElementById('input-neighbour-address');
                while (address_element.getElementsByTagName('option').length > 0) {
                    address_element.removeChild(address_element.lastChild);
                }
                clear_element_validation(address_element);

                let opt1 = document.createElement('option');
                opt1.text = "Select Address";
                opt1.classList.add("color-neutral");
                address_element.appendChild(opt1);

                for (let [address, certificate] of json.result) {
                    //console.log(address, certificate);
                    let option_element = document.createElement('option');
                    option_element.value = certificate;
                    option_element.classList.add("color-black");

                    // capitalise each word
                    address = address.toLowerCase().replace(/(^\w{1})|(\s+\w{1})/g, letter => letter.toUpperCase());
                    // shorten address and remove any trailing commas or spaces
                    option_element.text = address.substring(0, 45).replace(/,\s*$/, "");
                    address_element.appendChild(option_element);
                }
                unhide_elements([address_element]);
                return "";
            } else {
                throw new Error(json['error']);
            }
        } catch (error) {
            console.error('api-address-certificate-error: ', error);
            if (error.message == 'Failed to fetch' || error.message == 'Load failed') {
                epc_api_connection = false;
                return "";
            }
            epc_api_error = true;
            return "";
        }
    }
    return "";
}

function show_manual_epc_input() {
    if (scottish_postcode || !epc_api_connection || epc_api_error) {
        unhide_ids(['input-box-epc-space-heating', 'input-box-floor-area']);
    } return "";
}

// https://ideal-postcodes.co.uk/guides/uk-postcode-format
const postcode_formats = ["AA9A9AA", "A9A9AA", "A99AA", "A999AA", "AA99AA", "AA999AA"]

function check_postcode_format(postcode) {
    const valid_formats = [0, 1, 2, 3, 4, 5];
    for (let format of postcode_formats) {
        // console.log("format: ", format);

        if (format.length == postcode.length) {
            let valid_format = true;
            for (let i in postcode) {
                if (format[i] == "A" && postcode[i].match(/[a-z]/i)) {
                    // console.log("i", i, "letter: ", postcode[i]);
                } else if (format[i] == "9" && !isNaN(postcode[i])) {
                    // console.log("i", i, "number: ", postcode[i]);
                } else {
                    // console.log("i", i, "wrong: ", postcode[i]);
                    valid_format = false;
                    break;
                }
            }
            if (valid_format) {
                console.log("valid format: ", format);
                return ""
            }
        }
    }
    console.log("no valid format");
    return "none";
}

function update_epc_urls() {
    // GOV EPC, scotland EPC, postcode-specific EPC, address-specific EPC
    // console.log("update_epc_urls: ", scottish_postcode);
    let epc_urls = document.getElementsByClassName('epc-url');
    if (scottish_postcode) {
        for (let url of epc_urls) {
            // url.href = 'https://www.gov.uk/find-energy-certificate';
            // console.log("URL: ", url)
            url.href = 'https://www.scottishepcregister.org.uk';
        }
    } else {
        let postcode_element = document.getElementById('input-postcode');
        if (selected_certificate != undefined) {
            for (let url of epc_urls) {
                url.href = 'https://find-energy-certificate.service.gov.uk/energy-certificate/' + selected_certificate;
            }
        } else if (postcode_element.classList.contains("valid")) {
            for (let url of epc_urls) {
                url.href = 'https://find-energy-certificate.service.gov.uk/find-a-certificate/search-by-postcode?postcode=' + postcode_element.value;
            }
        } else {
            for (let url of epc_urls) {
                url.href = 'https://www.gov.uk/find-energy-certificate';
            }
        }
    }
}

function update_neighbours_epc_urls() {
    // GOV EPC, postcode-specific EPC, address-specific EPC
    let epc_urls = document.getElementsByClassName('epc-url-neighbour');
    let postcode_element = document.getElementById('input-neighbour-postcode');
    let select = document.getElementById('input-neighbour-address');

    if (select.selectedIndex > 0) {
        let certificate = select.options[select.selectedIndex].value;
        // console.log('certificate', certificate);
        for (let url of epc_urls) {
            url.href = 'https://find-energy-certificate.service.gov.uk/energy-certificate/' + certificate;
        }
    } else if (postcode_element.classList.contains("valid")) {
        for (let url of epc_urls) {
            url.href = 'https://find-energy-certificate.service.gov.uk/find-a-certificate/search-by-postcode?postcode=' + postcode_element.value;
        }
    } else {
        for (let url of epc_urls) {
            url.href = 'https://www.gov.uk/find-energy-certificate';
        }
    }
}

// GRAPHICAL OUTPUT MENU

let output = undefined;

const system_order = [
    'electric-boiler',
    'air-source-heat-pump',
    'ground-source-heat-pump',
    'biomass-boiler',
    'gas-boiler',
    'hydrogen-boiler',
    'hydrogen-fuel-cell'
];

const solar_order = [
    'none',
    'photovoltaic',
    'flat-plate',
    'evacuated-tube',
    'flat-plate-and-photovoltaic',
    'evacuated-tube-and-photovoltaic',
    'photovoltaic-thermal-hybrid',
]

const h2_order = ['green', 'blue', 'grey'];

const full_order = {
    'electric-boiler': solar_order,
    'air-source-heat-pump': solar_order,
    'ground-source-heat-pump': solar_order,
    'hydrogen-boiler': h2_order,
    'hydrogen-fuel-cell': h2_order,
    'biomass-boiler': undefined,
    'gas-boiler': undefined,
}

const system_colors = {
    'electric-boiler': 'red',
    'air-source-heat-pump': 'blue',
    'ground-source-heat-pump': 'green',
    'hydrogen-boiler': 'cyan',
    'hydrogen-fuel-cell': 'purple',
    'biomass-boiler': 'orange',
    'gas-boiler': 'black',
};

let colors = {
    'red': 'red',
    'orange': 'rgb(255, 106, 0)',
    'green': 'green',
    'cyan': 'rgb(0, 148, 255)',
    'blue': 'blue',
    'purple': 'rgb(255, 106, 255)',
    'black': 'black'
}

const solar_markers = {
    'none': 'cross',
    'photovoltaic': 'diamond',
    'flat-plate': 'circle',
    'evacuated-tube': 'square',
    'flat-plate-and-photovoltaic': 'uparrow',
    'evacuated-tube-and-photovoltaic': 'downarrow',
    'photovoltaic-thermal-hybrid': 'star',
};

const subsystem_help_links = {
    'none': `None`,
    'photovoltaic': `<a href="education/solar-technologies.html#pv-panels" target="_blank" rel="noopener noreferrer">Photovoltaic Panels</a>`,
    'flat-plate': `<a href="education/solar-technologies.html#flat-plate" target="_blank" rel="noopener noreferrer">Flat Plate Solar Thermal Collectors</a>`,
    'evacuated-tube': `<a href="education/solar-technologies.html#evacuated-tube" target="_blank" rel="noopener noreferrer">Evacuated Tube Solar Thermal Collectors</a>`,
    'flat-plate-and-photovoltaic': `<a href="education/solar-technologies.html#pv-panels" target="_blank" rel="noopener noreferrer">Photovoltaic Panels</a> & <a href="education/solar-technologies.html#flat-plate" target="_blank" rel="noopener noreferrer">Flat Plate  Solar Thermal Collectors</a>`,
    'evacuated-tube-and-photovoltaic': '<a href="education/solar-technologies.html#pv-panels" target="_blank" rel="noopener noreferrer">Photovoltaic Panels</a> & <a href="education/solar-technologies.html#evacuated-tube" target="_blank" rel="noopener noreferrer">Evacuated Tube  Solar Thermal Collectors</a>',
    'photovoltaic-thermal-hybrid': `<a href="education/solar-technologies.html#pvt" target="_blank" rel="noopener noreferrer">Photovoltaic-Thermal-Hybrid</a>`,
    'green': 'Green',
    'blue': 'Blue',
    'grey': 'Grey',
}

const h2_markers = {
    'green': 'circle',
    'blue': 'square',
    'grey': 'uparrow',
};

const system_markers = {
    'electric-boiler': solar_markers,
    'air-source-heat-pump': solar_markers,
    'ground-source-heat-pump': solar_markers,
    'hydrogen-boiler': h2_markers,
    'hydrogen-fuel-cell': h2_markers,
    'biomass-boiler': ['square'],
    'gas-boiler': ['square'],
};

const display_names = {
    'electric-boiler': 'Electric Boiler',
    'air-source-heat-pump': 'Air Source Heat Pump',
    'ground-source-heat-pump': 'Ground Source Heat Pump',
    'hydrogen-boiler': 'Hydrogen Boiler',
    'hydrogen-fuel-cell': 'Hydrogen Fuel Cell',
    'biomass-boiler': 'Biomass Boiler',
    'gas-boiler': 'Gas Boiler',
    'none': 'None',
    'photovoltaic': 'Photovoltaic Panels',
    'flat-plate': 'Flat Plates',
    'evacuated-tube': 'Evacuated Tubes',
    'flat-plate-and-photovoltaic': 'Photovoltaic Panels & Evacuated Tubes',
    'evacuated-tube-and-photovoltaic': 'Photovoltaic Panels & Flat Plates',
    'photovoltaic-thermal-hybrid': 'Photovoltaic Thermal Hybrid',
    'green': 'Green-Sourced Hydrogen',
    'blue': 'Blue-Sourced Hydrogen',
    'grey': 'Grey-Sourced Hydrogen',
};

const info_display_names = {
    'thermal-energy-storage-volume': 'Water Tank Volume',
    'operational-expenditure': 'Yearly Cost',
    'capital-expenditure': 'Upfront Cost',
    'net-present-cost': 'Lifetime Cost',
    'operational-emissions': 'Yearly Emissions'
}

const info_display_units = {
    'pv-size': (value) => { return `${value}m<sup>2</sup>` },
    'solar-thermal-size': (value) => { return `${value}m<sup>2</sup>` },
    'thermal-energy-storage-volume': (value) => { return `${value}m<sup>3</sup>` },
    'operational-expenditure': (value) => { return `${display_sign(value)}£${Math.round(Math.abs(value))}` },
    'capital-expenditure': (value) => { return `${display_sign(value)}£${Math.round(Math.abs(value))}` },
    'net-present-cost': (value) => { return `${display_sign(value)}£${Math.round(Math.abs(value))}` },
    'operational-emissions': (value) => { return `${display_sign(value)}${Math.round(Math.abs(value / 1000))} kgCO<sub>2</sub>eq` }
}

const system_visible = {
    'electric-boiler': true,
    'air-source-heat-pump': true,
    'ground-source-heat-pump': true,
    'hydrogen-boiler': false,
    'hydrogen-fuel-cell': false,
    'biomass-boiler': true,
    'gas-boiler': true,
};

function set_system_visibility() {
    for (const [system_name, is_visible] of Object.entries(system_visible)) {
        let parent = document.getElementById('system-' + system_name);
        if (!is_visible) {
            parent.getElementsByClassName('header')[0].getElementsByClassName('show')[0].src = './images/hidden.png';
            chart.data.datasets[system_order.indexOf(system_name)].hidden = true;
        }
    }
    chart.update();
}

// MENU

function display_sign(value) {
    if (value < 0) {
        return "-";
    } else {
        return "";
    }
}

function build_parent_header(system_name, color) {
    let header_div = document.createElement('div');
    header_div.classList.add('header');

    header_div.addEventListener('click', () => { toggle_expand_parent_direct(header_div) });

    if (system_name == system_order[system_order.length - 1]) {
        header_div.classList.add('border-bottom');
    }

    let marker_div = document.createElement('div');
    marker_div.style.borderColor = colors[color];
    marker_div.classList.add('marker');

    let name_div = document.createElement('div');
    name_div.innerHTML = `<a href="education/heating-technologies.html#${system_name}" target="_blank" rel="noopener noreferrer">${display_names[system_name]}</a>`;
    name_div.classList.add('name');

    let a_tags = name_div.getElementsByTagName('a');
    for (let a_tag of a_tags) {
        a_tag.addEventListener('click', (event) => {
            event.stopPropagation();
        });
    }

    let visible_img = document.createElement('img');
    visible_img.src = "./images/visible.png";
    visible_img.classList.add('show');
    visible_img.addEventListener('click', (event) => {
        event.stopPropagation();
        if (visible_img.src.endsWith("hidden.png")) {
            visible_img.src = "./images/visible.png";
            // console.log("show: ", visible_img.parentElement.parentElement.id);

            let system_name = visible_img.parentElement.parentElement.id.substr(7);
            console.log(system_name);

            chart.data.datasets[system_order.indexOf(system_name)].hidden = false;
            apply_limits();
            chart.update();
        } else {
            visible_img.src = "./images/hidden.png";
            // console.log("hide: ", visible_img.parentElement.parentElement.id);
            let system_name = visible_img.parentElement.parentElement.id.substr(7);
            // console.log(system_name);

            chart.data.datasets[system_order.indexOf(system_name)].hidden = true;
            apply_limits();
            chart.update();
        }
    });

    let expand_div = document.createElement('div');
    expand_div.classList.add('expand');
    // expand_div.addEventListener('click', () => toggle_expand_parent(expand_div));

    let expand_inner_div = document.createElement('div');
    expand_inner_div.classList.add();
    expand_div.appendChild(expand_inner_div);

    header_div.appendChild(marker_div);
    header_div.appendChild(name_div);
    header_div.appendChild(visible_img);
    header_div.appendChild(expand_div);
    return header_div;
}

function build_child_header(subsystem_name, shape, color) {
    let header_div = document.createElement('div');
    header_div.classList.add('header');
    header_div.addEventListener('click', (event) => { toggle_expand_child_direct(header_div) });

    let marker_img = document.createElement('img');
    marker_img.src = `./images/${shape}-${color}.png`;
    marker_img.classList.add('marker');

    let name_div = document.createElement('div');
    name_div.innerHTML = subsystem_help_links[subsystem_name];
    name_div.classList.add('name');

    let a_tags = name_div.getElementsByTagName('a');
    for (let a_tag of a_tags) {
        a_tag.addEventListener('click', (event) => {
            event.stopPropagation();
        });
    }

    let expand_div = document.createElement('div');
    expand_div.classList.add('expand');
    // expand_div.addEventListener('click', () => toggle_expand_child(expand_div));

    let expand_inner_div = document.createElement('div');
    expand_inner_div.classList.add();
    expand_div.appendChild(expand_inner_div);

    header_div.appendChild(marker_img);
    header_div.appendChild(name_div);
    header_div.appendChild(expand_div);
    return header_div;
}

function build_child_info(subsystem_name, system_properties) {
    function make_namevalue(name, value) {
        const name_link_list = {
            'Photovolatic Panel Area': '<a href="./education/solar-technologies.html#pv-panels" target="_blank" rel="noopener noreferrer">Photovolatic Panel</a> Area',
            'Flat Plate Area': '<a href="./education/solar-technologies.html#flat-plate" target="_blank" rel="noopener noreferrer">Flat Plate</a> Area',
            'Evacuated Tube Area': '<a href="./education/solar-technologies.html#evacuated-tube" target="_blank" rel="noopener noreferrer">Evacuated Tube</a> Area',
            'Photovoltaic-Thermal-Hybrid Area': '<a href="./education/solar-technologies.html#pvt" target="_blank" rel="noopener noreferrer">Photovoltaic-Thermal-Hybrid</a> Area',
            'Yearly Cost': '<a href="./documentation/simulator-outputs.html#operational-expenditure" target="_blank" rel="noopener noreferrer">Yearly Cost</a>',
            'Upfront Cost': '<a href="./documentation/simulator-outputs.html#capital-expenditure" target="_blank" rel="noopener noreferrer">Upfront Cost</a>',
            'Lifetime Cost': '<a href="./documentation/simulator-outputs.html#net-present-cost" target="_blank" rel="noopener noreferrer">Lifetime Cost</a>',
            'Yearly Emissions': '<a href="./documentation/simulator-outputs.html#operational-emissions" target="_blank" rel="noopener noreferrer">Yearly Emissions</a>',
            'Water Tank Volume': '<a href="./documentation/simulator-outputs.html#tes-volume" target="_blank" rel="noopener noreferrer">Water Tank Volume</a>',
        };

        let namevalue_div = document.createElement('div');
        namevalue_div.classList.add('name-value');
        let label = document.createElement('label');
        // console.log('name', name, 'href', name_link_list[name]);
        label.innerHTML = name_link_list[name];

        let p = document.createElement('p');
        p.innerHTML = value;

        namevalue_div.appendChild(label);
        namevalue_div.appendChild(p);
        return namevalue_div;
    }

    let header_div = document.createElement('div');
    header_div.classList.add('info');

    for (const [property_name, property_value] of Object.entries(system_properties)) {
        let united_value = info_display_units[property_name](property_value);
        if (property_name == 'pv-size') {
            switch (subsystem_name) {
                case 'photovoltaic':
                case 'flat-plate-and-photovoltaic':
                case 'evacuated-tube-and-photovoltaic':
                    header_div.appendChild(make_namevalue('Photovolatic Panel Area', united_value));
                    break;
            }
        } else if (property_name == 'solar-thermal-size') {
            switch (subsystem_name) {
                case 'flat-plate':
                case 'flat-plate-and-photovoltaic':
                    header_div.appendChild(make_namevalue('Flat Plate Area', united_value));
                    break;
                case 'evacuated-tube':
                case 'evacuated-tube-and-photovoltaic':
                    header_div.appendChild(make_namevalue('Evacuated Tube Area', united_value));
                    break;
                case 'photovoltaic-thermal-hybrid':
                    header_div.appendChild(make_namevalue('Photovoltaic-Thermal-Hybrid Area', united_value));
                    break;
            }
        } else {
            header_div.appendChild(make_namevalue(info_display_names[property_name], united_value));
        }
    }

    return header_div;
}

function build_system_menu() {
    let system_menu = document.getElementById('system-menu');
    system_menu.innerHTML = '';
    let animation_duration = 500; //duration in ms
    let menu_scroll_fnc = EASE_IN_OUT_QUINT(animation_duration);
    uss.setYStepLengthCalculator(
        menu_scroll_fnc,
        system_menu
    );

    for (const system_name of system_order) {
        sub_systems = output.systems[system_name];
        // console.log(system_name, sub_systems);

        let color = system_colors[system_name];

        let system_div = document.createElement('div');
        system_div.id = 'system-' + system_name;
        system_div.classList.add('parent', 'hide');
        system_div.appendChild(build_parent_header(system_name, color));

        let markers = system_markers[system_name];
        let children_div = document.createElement('div');
        children_div.classList.add('children');
        // console.log('markers:', markers);
        if (!Array.isArray(markers)) {
            for (const [subsystem_name, marker] of Object.entries(markers)) {
                let system_properties = sub_systems[subsystem_name];
                // console.log(subsystem_name, system_properties);
                let subsystem_div = document.createElement('div');
                subsystem_div.id = 'system-' + system_name + '-' + subsystem_name;
                subsystem_div.classList.add('child', 'hide');

                subsystem_div.appendChild(build_child_header(subsystem_name, markers[subsystem_name], color));
                subsystem_div.appendChild(build_child_info(subsystem_name, system_properties));
                children_div.appendChild(subsystem_div);
            }
        } else {
            // console.log(system_name, sub_systems);
            children_div.appendChild(build_child_info(system_name, sub_systems));
        }
        system_div.appendChild(children_div);
        system_menu.appendChild(system_div);
    }
}

function hide_all_menu_items() {
    let system_menu = document.getElementById('system-menu');
    let parents = system_menu.getElementsByClassName('parent');
    for (let parent of parents) {
        parent.classList.add('hide');
    }

    let childs = system_menu.getElementsByClassName('child');
    for (let child of childs) {
        child.classList.add('hide');
    }
}

function toggle_expand_parent_direct(header_div) {
    let parent = header_div.parentElement;
    if (parent.classList.contains('hide')) {
        hide_all_menu_items();
        parent.classList.remove('hide');
    } else {
        parent.classList.add('hide');
    }
}

function toggle_expand_parent(expand_div) {
    let parent = expand_div.parentElement.parentElement;

    if (parent.classList.contains('hide')) {
        hide_all_menu_items();
        parent.classList.remove('hide');
    } else {
        parent.classList.add('hide');
    }
}

function toggle_expand_child_direct(header_div) {
    let child = header_div.parentElement;

    if (child.classList.contains('hide')) {
        let children = child.parentElement;
        let childs = children.getElementsByClassName('child');
        for (let c of childs) {
            c.classList.add('hide');
        }

        child.classList.remove('hide');
    } else {
        child.classList.add('hide');
    }
}

function toggle_expand_child(expand_div) {
    let child = expand_div.parentElement.parentElement;

    if (child.classList.contains('hide')) {
        let children = child.parentElement;
        let childs = children.getElementsByClassName('child');
        for (let c of childs) {
            c.classList.add('hide');
        }

        child.classList.remove('hide');
    } else {
        child.classList.add('hide');
    }
}

function open_child(id) {
    hide_all_menu_items();
    let child = document.getElementById(id);
    child.classList.remove('hide');
    let parent = child.parentElement.parentElement;
    parent.classList.remove('hide');
    // uss.scrollIntoView(child);
    uss.scrollIntoViewIfNeeded(child);
}

// GRAPH
let marker_img_size = 12;
let marker_names = ['cross', 'diamond', 'circle', 'square', 'uparrow', 'downarrow', 'star'];
let marker_images = {
    'red': load_images('red'),
    'orange': load_images('orange'),
    'green': load_images('green'),
    'cyan': load_images('cyan'),
    'blue': load_images('blue'),
    'purple': load_images('purple'),
    'black': load_images('black')
};

const axes_label_map = {
    'capital-expenditure': "Upfront Cost (£1000's)",
    'operational-expenditure': "Yearly Cost (£'s)",
    'net-present-cost': "Lifetime Cost (£1000's)",
    'operational-emissions': "Yearly Emissions (tonnesCO2eq)",
}

const divisor_map = {
    'capital-expenditure': 1000,
    'operational-expenditure': 1,
    'net-present-cost': 1000,
    'operational-emissions': 1000000,
}


let point_radius = 6;
function load_images(color) {
    let marker_images = {};
    //let marker_names = ['cross', 'diamond', 'circle', 'square', 'uparrow', 'downarrow', 'star'];
    for (let [i, name] of marker_names.entries()) {
        if (name == 'cross') {
            marker_images[name] = new Image(marker_img_size * 1.5, marker_img_size * 1.5);
        } else if (name == 'star') {
            marker_images[name] = new Image(marker_img_size * 1.5, marker_img_size * 1.5);
        } else {
            marker_images[name] = new Image(marker_img_size, marker_img_size);
        }

        marker_images[name].src = `./images/${name}-${color}.png`
    }
    return marker_images;
}

function replace_graph_data() {
    let x_select = document.getElementById("x-param");
    let x_param = x_select.options[x_select.selectedIndex].value;
    // console.log("x-param: ", x_param);

    let y_select = document.getElementById("y-param");
    let y_param = y_select.options[y_select.selectedIndex].value;
    // console.log("y-param: ", y_param);

    let x_divisor = divisor_map[x_param];
    let y_divisor = divisor_map[y_param];

    chart.options.scales.x.title.text = axes_label_map[x_param];
    chart.options.scales.y.title.text = axes_label_map[y_param];
    let i = 0;

    for (let system_name of system_order) {
        let dataset_data = [];

        switch (system_name) {
            case 'biomass-boiler':
            case 'gas-boiler':
                let values = output.systems[system_name];
                let x = values[x_param] / x_divisor;
                let y = values[y_param] / y_divisor;
                dataset_data.push({
                    'x': x,
                    'y': y
                });
                break;
            default:
                for (const subsystem_name of Object.keys(system_markers[system_name])) {
                    let values = output.systems[system_name][subsystem_name];
                    let x = values[x_param] / x_divisor;
                    let y = values[y_param] / y_divisor;
                    dataset_data.push({
                        'x': x,
                        'y': y
                    });
                }
        }
        chart.data.datasets[i].data = dataset_data;
        i += 1;
    }

    apply_limits();
    chart.update();
}

function apply_limits() {
    let limits = {
        x: {
            'min': undefined,
            'max': undefined,
        },
        y: {
            'min': undefined,
            'max': undefined,
        }
    }

    for (let dataset of chart.data.datasets) {
        for (const data of dataset.data) {
            let x = data.x;
            let y = data.y

            if (!dataset.hidden) {
                if (limits.x.min == undefined || x < limits.x.min) { limits.x.min = x };
                if (limits.x.max == undefined || x > limits.x.max) { limits.x.max = x };
                if (limits.y.min == undefined || y < limits.y.min) { limits.y.min = y };
                if (limits.y.max == undefined || y > limits.y.max) { limits.y.max = y };
            }
        }
    }

    const min_padding_map = {
        'capital-expenditure': 1,
        'operational-expenditure': 1,
        'net-present-cost': 1,
        'operational-emissions': 0.1,
    }

    let x_select = document.getElementById("x-param");
    let x_param = x_select.options[x_select.selectedIndex].value;
    let y_select = document.getElementById("y-param");
    let y_param = y_select.options[y_select.selectedIndex].value;

    let mindr = { x: min_padding_map[x_param], y: min_padding_map[y_param] };

    const dr = 0.1;
    let dx = limits.x.max - limits.x.min;
    limits.x.max += Math.max(dx * dr, mindr.x);
    limits.x.min -= Math.max(dx * dr, mindr.x);

    let dy = limits.y.max - limits.y.min;
    limits.y.max += Math.max(dy * dr, mindr.y);
    limits.y.min -= Math.max(dy * dr, mindr.y);

    limits.x.max = Math.ceil(limits.x.max / mindr.x) * mindr.x;
    limits.x.min = Math.floor(limits.x.min / mindr.x) * mindr.x;
    limits.y.max = Math.ceil(limits.y.max / mindr.y) * mindr.y;
    limits.y.min = Math.floor(limits.y.min / mindr.y) * mindr.y;

    chart.options.plugins.zoom.limits = limits;

    chart.options.scales.x.min = limits.x.min;
    chart.options.scales.x.max = limits.x.max;
    chart.options.scales.y.min = limits.y.min;
    chart.options.scales.y.max = limits.y.max;
}

function build_graph_data() {
    function datafill(n) {
        let data = [];
        for (let i = 0; i < n; i++) {
            data.push({ x: i, y: i });
        }
        return data;
    }

    let data = {};
    data['datasets'] = [];
    for (let system_name of system_order) {

        let color = system_colors[system_name];
        let rgb = colors[color];
        let markers = system_markers[system_name];

        let point_styles = [];
        if (!Array.isArray(markers)) {
            for (let [subsystem_name, marker] of Object.entries(markers)) {
                point_styles.push(marker_images[color][marker])
            }
        } else {
            point_styles.push(marker_images[color][markers[0]]);
            // console.log(point_styles);
        }

        // console.log(point_styles);
        data['datasets'].push({
            data: datafill(point_styles.length),
            radius: point_radius,
            label: system_name,
            backgroundColor: rgb,
            borderColor: rgb,
            pointStyle: point_styles,
        });
    }

    // console.log(data);
    return data;
}

function resize_markers() {
    let canvas = document.getElementById('chart');
    let width = Number(canvas.style.width.substring(0, canvas.style.width.length - 2));

    marker_img_size = Math.max(width / 60, 8);

    // console.log(canvas.width, marker_img_size);
    for (let dataset of chart.data.datasets) {
        let system_name = dataset.label;
        let color = system_colors[system_name];
        let markers = system_markers[system_name];


        marker_images = {
            'red': load_images('red'),
            'orange': load_images('orange'),
            'green': load_images('green'),
            'cyan': load_images('cyan'),
            'blue': load_images('blue'),
            'purple': load_images('purple'),
            'black': load_images('black')
        };

        let point_styles = [];
        if (!Array.isArray(markers)) {
            for (let [subsystem_name, marker] of Object.entries(markers)) {
                point_styles.push(marker_images[color][marker])
            }
        } else {
            point_styles.push(marker_images[color][markers[0]]);
        }
        dataset.pointStyle = point_styles;
    }
    chart.update();
}

let config = {
    type: 'scatter',
    data: build_graph_data(),
    options: {
        onClick: (e) => {
            let activePoints = chart.getElementsAtEventForMode(e, 'nearest', { intersect: true }, false)

            // make sure click was on an actual point
            if (activePoints.length > 0) {
                let context = activePoints[0];


                let system_id = system_order[context.datasetIndex];
                if (system_id == 'gas-boiler' || system_id == 'biomass-boiler') {
                    open_child('system-' + system_id)
                } else {
                    let subsystem_id = full_order[system_id][context.index];
                    open_child('system-' + system_id + '-' + subsystem_id);
                }
            }

        },
        elements: {
            point: {
                pointStyle: 'circle',
            }
        },
        scales: {
            x: {
                type: 'linear',
                position: 'bottom',
                title: {
                    display: true,
                    text: 'Upfront Cost (£)',
                    font: {
                        size: 14,
                        family: 'Sora'
                    }
                },
                ticks: {
                    font: {
                        size: 12,
                        family: 'Sora'
                    }
                },
                'min': 0,
                'max': 20,
                ticks: {
                    includeBounds: false,
                }
            },
            y: {
                title: {
                    display: true,
                    text: 'Yearly Emissions (kgCO2eq/yr)',
                    font: {
                        size: 14,
                        family: 'Sora'
                    }
                },
                ticks: {
                    font: {
                        size: 12,
                        family: 'Sora'
                    }
                },
                'min': 0,
                'max': 1000,
                ticks: {
                    includeBounds: false,
                }
            }
        },
        aspectRatio: 1,
        plugins: {
            zoom: {
                zoom: {
                    wheel: {
                        enabled: true,
                    },
                    pinch: {
                        enabled: true
                    },
                    mode: 'xy',
                },
                pan: {
                    enabled: true,
                    threshold: 10,
                },
                limits: {
                    x: {
                        'min': 0,
                        'max': 20,
                    },
                    y: {
                        'min': 0,
                        'max': 1000,
                    }
                }
            },

            legend: {
                display: false,
                labels: {
                    font: {
                        size: 20
                    }
                }
            },
            tooltip: {
                callbacks: {
                    label: function (context) {
                        // console.log(context);

                        if (context.dataset.data.length == 1) {
                            return `${display_names[context.dataset.label]}`
                        } else {
                            //console.log(display_names, output.systems, context.dataset.label, context.dataIndex);
                            //console.log(full_order[context.dataset.label][context.dataIndex], display_names[full_order[context.dataset.label][context.dataIndex]]);
                            //return `${display_names[context.dataset.label]}, ${display_names[Object.keys(output.systems[context.dataset.label])[context.dataIndex]]}`
                            return `${display_names[context.dataset.label]}, ${display_names[full_order[context.dataset.label][context.dataIndex]]}`
                        }

                    }
                },
                usePointStyle: true
            }
        }
    }
};

config.options.animation = {
    numbers: { duration: 0 },
    colors: {
        type: "color",
        duration: 1000,
        from: "transparent",
    }
}

let chart = undefined;

function load_output() {
    console.log('load_output');
    create_simulation_output_file_download();
    unhide_ids(['output-info-toggle', 'results-guide', 'table-area', 'input-box-download-link']);
    hide_ids(['input-form']);
    build_system_menu();
    document.getElementById("y-param").selectedIndex = 1;
    const ctx = document.getElementById('chart').getContext('2d');
    if (chart == undefined) {
        chart = new Chart(ctx, config);
    }
    replace_graph_data();
    set_system_visibility();
    apply_limits();
    chart.update();

    table_data = rebuild_data();
    build_table();
}

function guide_state(doHide) {
    if (doHide) {
        guide.classList.add('hide');
        unhide_ids(['guide', 'input-form']);
        hide_ids(['results-guide', 'submit-complete'])
    } else {
        unhide_ids(['results-guide']);
        hide_ids(['input-form']);
    }
}

// TABLE

const table_system_names = {
    'electric-boiler': 'EB',
    'air-source-heat-pump': 'ASHP',
    'ground-source-heat-pump': 'GSHP',
    'biomass-boiler': 'Biomass Boiler',
    'gas-boiler': 'Gas Boiler',
    'hydrogen-boiler': 'H<sub>2</sub>B',
    'hydrogen-fuel-cell': 'H<sub>2</sub>FC',
};

const long_table_system_names = {
    'electric-boiler': 'Electric Boiler',
    'air-source-heat-pump': 'Air Source Heat Pump',
    'ground-source-heat-pump': 'Ground Source Heat Pump',
    'biomass-boiler': 'Biomass Boiler',
    'gas-boiler': 'Gas Boiler',
    'hydrogen-boiler': 'Hydrogen Boiler',
    'hydrogen-fuel-cell': 'Hydrogen Fuel Cell',
};


const table_subsystem_names = {
    'none': () => { return 'None' },
    'photovoltaic': (properties) => { return [`PV<sub>${properties['pv-size']}</sub>`]; },
    'flat-plate': (properties) => { return [`FP<sub>${properties['solar-thermal-size']}</sub>`]; },
    'evacuated-tube': (properties) => { return [`ET<sub>${properties['solar-thermal-size']}</sub>`]; },
    'flat-plate-and-photovoltaic': (properties) => {
        return [`PV<sub>${properties['pv-size']}</sub>`, `FP<sub>${properties['solar-thermal-size']}</sub>`];
    },
    'evacuated-tube-and-photovoltaic': (properties) => {
        return [`PV<sub>${properties['pv-size']}</sub>`, `ET<sub>${properties['solar-thermal-size']}</sub>`];
    },
    'photovoltaic-thermal-hybrid': (properties) => { return [`PVT<sub>${properties['pv-size']}</sub>`]; },
    'green': () => { return 'Green' },
    'blue': () => { return 'Blue' },
    'grey': () => { return 'Grey' },
};


const long_table_subsystem_names = {
    'none': () => { return 'None' },
    'photovoltaic': (properties) => { return [`${properties['pv-size']}m<sup>2</sup> PV`]; },
    'flat-plate': (properties) => { return [`${properties['solar-thermal-size']}m<sup>2</sup> FP`]; },
    'evacuated-tube': (properties) => { return [`${properties['solar-thermal-size']}m<sup>2</sup> ET`]; },
    'flat-plate-and-photovoltaic': (properties) => {
        return [`${properties['pv-size']}m<sup>2</sup> PV`, `${properties['solar-thermal-size']}m<sup>2</sup> FP`];
    },
    'evacuated-tube-and-photovoltaic': (properties) => {
        return [`${properties['pv-size']}m<sup>2</sup> PV`, `${properties['solar-thermal-size']}m<sup>2</sup> ET`];
    },
    'photovoltaic-thermal-hybrid': (properties) => { return [`${properties['pv-size']}m<sup>2</sup> PVT`]; },
    'green': () => { return 'Green' },
    'blue': () => { return 'Blue' },
    'grey': () => { return 'Grey' },
};

let table_data = undefined;


function rebuild_data() {
    let data = {
        'name': [],
        'long-name': [],
        'thermal-energy-storage-volume': [],
        'capital-expenditure': [],
        'operational-expenditure': [],
        'net-present-cost': [],
        'operational-emissions': [],
    };

    for (let system_name of Object.keys(table_system_names)) {
        switch (system_name) {
            case 'electric-boiler':
            case 'air-source-heat-pump':
            case 'ground-source-heat-pump':
            case 'hydrogen-boiler':
            case 'hydrogen-fuel-cell':
                for (let [subsystem_name, properties] of Object.entries(output.systems[system_name])) {
                    for (let property_name of Object.keys(data)) {
                        if (property_name == 'name') {
                            data[property_name].push([table_system_names[system_name]].concat(table_subsystem_names[subsystem_name](properties)));

                        } else if (property_name == 'long-name') {
                            data['long-name'].push([table_system_names[system_name]].concat(long_table_subsystem_names[subsystem_name](properties)));
                        } else {
                            data[property_name].push(properties[property_name]);
                        }
                    }
                }
                break;
            case 'biomass-boiler':
            case 'gas-boiler':
                for (let property_name of Object.keys(data)) {
                    if (property_name == 'name' || property_name == 'long-name') {
                        data[property_name].push([table_system_names[system_name]]);
                    } else {
                        data[property_name].push(output.systems[system_name][property_name]);
                    }
                }
                break;
        }
    }
    // console.log(data);
    return data;
}

function sort_indices(array, sortfnc) {
    let index_value_array = [];
    let i = 0;
    // console.log(array);
    for (let item of array) {
        index_value_array.push([item, i]);
        i += 1;
    }

    index_value_array.sort(sortfnc);
    let indices = [];
    for (let item of index_value_array) {
        indices.push(item[1]);
    }

    let index_value_array2 = [];
    i = 0;
    // console.log(indices);
    for (let item of indices) {
        index_value_array2.push([item, i]);
        i += 1;
    }

    index_value_array2.sort(sortfnc);
    let indices2 = [];
    for (let item of index_value_array2) {
        indices2.push(item[1]);
    }

    // console.log(index_value_array, indices2);
    return indices2;
}

function build_table() {
    const table_headers = {
        'name': 'System',
        'thermal-energy-storage-volume': '<div>m<sup>3</sup></div>',
        'capital-expenditure': '£',
        'operational-expenditure': '£/yr',
        'net-present-cost': '£/life',
        'operational-emissions': '<div>CO<sub>2</sub></div>',
    };

    const long_table_headers = {
        'name': 'System',
        'thermal-energy-storage-volume': 'Water Tank Volume (m<sup>3</sup>)',
        'capital-expenditure': 'Upfront Cost (£)',
        'operational-expenditure': 'Yearly Cost (£)',
        'net-present-cost': 'Lifetime Cost (£)',
        'operational-emissions': 'Yearly Emissions <div>(kgCO<sub>2</sub>Eq)</div>',
    };

    function money_fmt(v) {
        if (v > 1000 && window.innerWidth < 500) {
            return `${Math.round(v / 1000)}k`
        } else {
            return numberWithCommas(Math.round(v));
        }
    }

    function numberWithCommas(x) {
        return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
    }

    const info_format = {
        'thermal-energy-storage-volume': (v) => {
            if (v == undefined) {
                return 'N/A';
            } else {
                return v;
            }
        },
        'capital-expenditure': (v) => { return money_fmt(v) },
        'operational-expenditure': (v) => { return money_fmt(v) },
        'net-present-cost': (v) => { return money_fmt(v) },
        'operational-emissions': (v) => { return Math.round(v / 1000) },
    }

    let table = document.getElementById('table');
    table.innerHTML = '';
    for (let [column_name, column_values] of Object.entries(table_data)) {
        if (column_name == 'long-name') { continue; }
        let column = document.createElement('div');
        column.classList.add('column');
        let h1 = document.createElement('h1');
        let h1p = document.createElement('p');
        h1p.innerHTML = long_table_headers[column_name];
        // if (window.innerWidth > 700) {
        //     h1p.innerHTML = long_table_headers[column_name];
        // } else {
        //     h1p.innerHTML = table_headers[column_name];
        // }
        console.log(column_name);

        h1.appendChild(h1p);
        h1.addEventListener('click', () => {
            let sortby = document.getElementById("sortby");
            sortby.value = column_name;
            sort_table(column_name);
        });
        column.appendChild(h1);
        if (column_name == 'name') {
            let names = window.innerWidth < 700 ? table_data.name : table_data['long-name'];
            // console.log(names);
            for (let name of Object.values(names)) {
                let name_div = document.createElement('div');
                name_div.classList.add('name', 'data');
                for (let part of name) {
                    let subname = document.createElement('p');
                    subname.innerHTML = part;
                    if (name.length == 1) {
                        subname.classList.add('only-child')
                    }
                    name_div.appendChild(subname);
                }
                column.appendChild(name_div);
            }
        } else {
            for (let item of column_values) {
                let p = document.createElement('p');
                p.classList.add('data');
                p.innerHTML = info_format[column_name](item);
                column.appendChild(p);
            }
        }
        table.appendChild(column);
    }


}

function onchange_sortby() {
    let sortby = document.getElementById("sortby");
    let name = sortby.options[sortby.selectedIndex].value;
    console.log(name);
    sort_table(name);
}


function sort_table(name) {
    // console.log('name', name);
    let order = name == 'name' ? Array.from(Array(29).keys()) : sort_indices([...table_data[name]], (a, b) => {
        return a[0] < b[0] ? -1 : 1;
    });

    let table = document.getElementById('table');
    let columns = table.getElementsByClassName('column');
    for (let column of columns) {
        let i = 0;
        for (let t of column.getElementsByClassName('data')) {
            t.style.order = order[i];
            i += 1;
        }
    }
}

let on_resize_end = undefined;
function reportWindowSize() {
    if (!document.getElementById('output-info-toggle').classList.contains('hide')) {
        resize_markers();
    }
    if (!document.getElementById('table-area').classList.contains('hide')) {
        if (on_resize_end) {
            clearTimeout(on_resize_end);
        }
        on_resize_end = setTimeout(() => build_table(), 100);
    }

}

window.onresize = reportWindowSize;
reportWindowSize();

output = {
    "demand": {
        "boiler": {
            "hot-water": 1460.07,
            "space": 1515.96,
            "total": 2976.03,
            "peak-hourly": 8.36874
        },
        "heat-pump": {
            "hot-water": 1460.07,
            "space": 1552.12,
            "total": 3012.19,
            "peak-hourly": 1.56076
        }
    },
    "systems": {
        "electric-boiler": {
            "none": {
                "pv-size": 0,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 232.091,
                "capital-expenditure": 678.913,
                "net-present-cost": 4092.94,
                "operational-emissions": 644775
            },
            "photovoltaic": {
                "pv-size": 14,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -35.5481,
                "capital-expenditure": 3758.91,
                "net-present-cost": 3236.01,
                "operational-emissions": 214478
            },
            "flat-plate": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 173.86,
                "capital-expenditure": 3256.41,
                "net-present-cost": 5813.87,
                "operational-emissions": 507729
            },
            "evacuated-tube": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 157.84,
                "capital-expenditure": 3366.41,
                "net-present-cost": 5688.21,
                "operational-emissions": 463252
            },
            "flat-plate-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -39.0105,
                "capital-expenditure": 5896.41,
                "net-present-cost": 5322.57,
                "operational-emissions": 182792
            },
            "evacuated-tube-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -64.4958,
                "capital-expenditure": 6006.41,
                "net-present-cost": 5057.69,
                "operational-emissions": 115537
            },
            "photovoltaic-thermal-hybrid": {
                "pv-size": 4,
                "solar-thermal-size": 4,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 102.037,
                "capital-expenditure": 5323.91,
                "net-present-cost": 6824.87,
                "operational-emissions": 364656
            }
        },
        "air-source-heat-pump": {
            "none": {
                "pv-size": 0,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 79.5922,
                "capital-expenditure": 6237.67,
                "net-present-cost": 7408.46,
                "operational-emissions": 232146
            },
            "photovoltaic": {
                "pv-size": 14,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -197.882,
                "capital-expenditure": 9317.67,
                "net-present-cost": 6406.86,
                "operational-emissions": -182272
            },
            "flat-plate": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 58.3877,
                "capital-expenditure": 8815.17,
                "net-present-cost": 9674.04,
                "operational-emissions": 191942
            },
            "evacuated-tube": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 52.8353,
                "capital-expenditure": 8925.17,
                "net-present-cost": 9702.37,
                "operational-emissions": 178927
            },
            "flat-plate-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -171.235,
                "capital-expenditure": 11455.2,
                "net-present-cost": 8936.33,
                "operational-emissions": -153667
            },
            "evacuated-tube-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -186.652,
                "capital-expenditure": 11565.2,
                "net-present-cost": 8819.54,
                "operational-emissions": -177541
            },
            "photovoltaic-thermal-hybrid": {
                "pv-size": 2,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 37.9702,
                "capital-expenditure": 10245.2,
                "net-present-cost": 10803.7,
                "operational-emissions": 158842
            }
        },
        "ground-source-heat-pump": {
            "none": {
                "pv-size": 0,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 53.459,
                "capital-expenditure": 7937.67,
                "net-present-cost": 8724.04,
                "operational-emissions": 154280
            },
            "photovoltaic": {
                "pv-size": 14,
                "solar-thermal-size": 0,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -247.103,
                "capital-expenditure": 11017.7,
                "net-present-cost": 7382.83,
                "operational-emissions": -260026
            },
            "flat-plate": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 39.7321,
                "capital-expenditure": 10515.2,
                "net-present-cost": 11099.6,
                "operational-emissions": 134456
            },
            "evacuated-tube": {
                "pv-size": 0,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 36.1047,
                "capital-expenditure": 10625.2,
                "net-present-cost": 11156.3,
                "operational-emissions": 128096
            },
            "flat-plate-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -211.235,
                "capital-expenditure": 13155.2,
                "net-present-cost": 10047.9,
                "operational-emissions": -216005
            },
            "evacuated-tube-and-photovoltaic": {
                "pv-size": 12,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": -223.614,
                "capital-expenditure": 13265.2,
                "net-present-cost": 9975.85,
                "operational-emissions": -229065
            },
            "photovoltaic-thermal-hybrid": {
                "pv-size": 2,
                "solar-thermal-size": 2,
                "thermal-energy-storage-volume": 0.1,
                "operational-expenditure": 19.2099,
                "capital-expenditure": 11945.2,
                "net-present-cost": 12227.7,
                "operational-emissions": 101650
            }
        },
        "hydrogen-boiler": {
            "grey": {
                "operational-expenditure": 162.028,
                "capital-expenditure": 2120,
                "net-present-cost": 4503.41,
                "operational-emissions": 1263160
            },
            "blue": {
                "operational-expenditure": 307.523,
                "capital-expenditure": 2120,
                "net-present-cost": 6643.61,
                "operational-emissions": 198402
            },
            "green": {
                "operational-expenditure": 608.432,
                "capital-expenditure": 2120,
                "net-present-cost": 11069.9,
                "operational-emissions": 1314410
            }
        },
        "hydrogen-fuel-cell": {
            "grey": {
                "operational-expenditure": 157.018,
                "capital-expenditure": 25157.8,
                "net-present-cost": 27467.5,
                "operational-emissions": 1224100
            },
            "blue": {
                "operational-expenditure": 298.015,
                "capital-expenditure": 25157.8,
                "net-present-cost": 29541.6,
                "operational-emissions": 192267
            },
            "green": {
                "operational-expenditure": 589.62,
                "capital-expenditure": 25157.8,
                "net-present-cost": 33831,
                "operational-emissions": 1273770
            }
        },
        "gas-boiler": {
            "operational-expenditure": 132.268,
            "capital-expenditure": 1620,
            "net-present-cost": 3565.64,
            "operational-emissions": 605126
        },
        "biomass-boiler": {
            "operational-expenditure": 135.905,
            "capital-expenditure": 9750,
            "net-present-cost": 11749.1,
            "operational-emissions": 297603
        }
    }
}
// load_output();




