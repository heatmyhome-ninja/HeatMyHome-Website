if (window.Worker) {
    var myWorker = new Worker('worker.js');
    myWorker.postMessage("initiate");
    //console.log(`Last Updated: ${last_updated}`);
} else {
    console.warn('Web Workers not supported');
}

var sim_string;

myWorker.onmessage = function (e) {
    //console.log('Message received from worker:', e);
    if (e.data[0] == "initiation complete") {
        console.log('C++ Worker Initiated');
        console.log('Last Updated:', e.data[1]);
    } else if (e.data[0] == "simulation complete") {
        let end = performance.now();
        let runtime = ((end - e.data[2]) / 1000.0).toPrecision(3);
        console.log(`C++ Simulation Runtime: ${runtime}s`);
        document.getElementById('sim-runtime').innerHTML = runtime;
        sim_string = e.data[1];
        document.getElementById('results').classList.remove("hide");
        renderSimTable();
    } else {
        console.warn('Message from worker is not linked to any event: ', e.data);
    }
}

// simulation =============================================================================

document.getElementById('sim-postcode').addEventListener('change', (event) => {
    let postcode = document.getElementById('sim-postcode').value;
    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    getJSON('https://api.postcodes.io/postcodes/' + postcode, function (err, data) {
        if (err != null) {
            console.error(err);
            document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
        } else {
            console.log(data);
            document.getElementById('sim-latitude').value = data.result.latitude;
            document.getElementById('sim-longitude').value = data.result.longitude;

            document.getElementById('sim-postcode').style.textDecorationLine = 'none';
        }
    });
});

function submitSimulation() {
    let postcode = document.getElementById('sim-postcode').value;

    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    document.getElementById('results').classList.add("hide");

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

    let latitude = document.getElementById('sim-latitude').value;
    let longitude = document.getElementById('sim-longitude').value;
    let occupants = document.getElementById('sim-occupants').value;
    let floor_area = document.getElementById('sim-floor-area').value;
    let temperature = document.getElementById('sim-temperature').value;
    let space_heating = document.getElementById('sim-space-heating').value;
    let tes_max = document.getElementById('sim-tes-max').value;
    let use_surface_optimisation = document.getElementById('sim-surface-optimisation').checked;

    document.getElementById('sim-runtime').innerHTML = '...';
    //console.log('asking worker to run simulation');

    myWorker.postMessage(["run simulation", [postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max, use_surface_optimisation]]);

    //console.log('sent message to worker');
}

// https://zetcode.com/javascript/jsonurl/
function getJSON(url, callback) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'json';

    xhr.onload = function () {

        var status = xhr.status;

        if (status == 200) {
            callback(null, xhr.response);
        } else {
            callback(status);
        }
    };
    xhr.send();
};

// ================================================================================================================

let sortby_index = 7;
function compare(a, b) {
    const i = sortby_index;
    if (sortby_index == 4) {
        return 0;
    }

    if (a[i] < b[i]) {
        return -1;
    }
    if (a[i] > b[i]) {
        return 1;
    }
    return 0;
}

function renderSimTable() {
    //console.log(sim_string);
    output_json = JSON.parse(sim_string);
    simtable = document.getElementById('sim-table');

    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);

    let sortbyopt = document.getElementById('sortby');
    sortby_index = Number(sortbyopt.value) + 4;

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

    let speci = 0;
    switch (groupby_index) {
        case 0:
            speci = Array(21).keys();
            output_json.sort(compare);
            break;
        case 1:
            speci = [0, 7, 14];
            //speci = Array(21).keys();
            a = output_json.slice(0, 7).sort(compare);
            b = output_json.slice(7, 14).sort(compare);
            c = output_json.slice(14, 21).sort(compare);
            output_json = a.concat(b, c);
            break;
        default: // 2
            b = output_json;
            output_json = [];
            for (let ii of Array(7).keys()) {
                let a = [b[ii], b[ii + 7], b[ii + 14]];
                a.sort(compare);
                //console.log(a);
                output_json.push(a[0]);
                output_json.push(a[1]);
                output_json.push(a[2]);
            }
            speci = [0, 3, 6, 9, 12, 15, 18];
        //part_json.sort(compare);
    }

    let collapseGroupsEle = document.getElementById('collapse-groups-input');
    let collapseGroups = collapseGroupsEle.checked;
    //console.log(collapseGroups);
    if (!collapseGroups) {
        speci = Array(21).keys();
    }
    // let part_json = [];
    // for (let ii of speci) {
    //     part_json.push(output_json[ii]);
    // };
    //console.log(output_json);

    for (let ii of speci) {
        //console.log(ii);
        let tr = document.createElement('tr');
        for (let value of output_json[ii]) {
            let td = document.createElement('td');
            td.innerHTML = value;
            tr.appendChild(td);
        }
        simtable.appendChild(tr);
    }
}

function updateGroupBy() {
    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);
    if (groupby_index == 0) {
        document.getElementById('collapse-groups').classList.add("hide");;
    } else {
        document.getElementById('collapse-groups').classList.remove("hide");;
    }
    renderSimTable();
}

function updateSortBy() {
    renderSimTable();
}

function updateCollapseGroups() {
    renderSimTable();
}
