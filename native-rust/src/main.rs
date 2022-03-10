use std::env;
use std::fs::File;
use std::io::{prelude::*, BufReader, LineWriter};
use std::ops::Range;
use std::path::Path;
use std::process::Command;
use std::time::Instant;

mod heat_ninja;

struct Inputs {
    thermostat_temperature: f32,
    latitude: f32,
    longitude: f32,
    num_occupants: u8,
    house_size: f32,
    postcode: String,
    epc_space_heating: f32,
    tes_volume_max: f32,
}

fn import_file_data(
    assets_dir: String,
    latitude: f32,
    longitude: f32,
) -> ([f32; 8760], [f32; 8760], [f32; 8760]) {
    // wasm does not support Rust file io,
    // -> file data must be gathered beforehand then passed into wasm functions

    let import_weather_data = |filepath: String| -> [f32; 8760] {
        //println!("filepath: {}", filepath);
        let file = File::open(&filepath).expect(&format!("cannot read file: {}", &filepath));
        let reader = BufReader::new(file);
        let mut data: [f32; 8760] = [0.0; 8760];
        for (i, line) in reader.lines().enumerate() {
            data[i] = line.unwrap().parse::<f32>().expect("string invalid float.");
        }
        data
    };

    let format_coordinate = |coordinate: f32| -> f32 {
        let rounded: i16 = (coordinate * 2.0).round() as i16;
        if rounded == -0 {
            0.0
        } else {
            (rounded as f32) / 2.0
        }
    };

    let build_weather_file_path = |datatype: &str| -> String {
        format!(
            "{}{}/lat_{:.1}_lon_{:.1}.csv",
            assets_dir,
            datatype,
            format_coordinate(latitude),
            format_coordinate(longitude)
        )
    };

    let agile_tariff_per_hour_over_year: [f32; 8760] =
        import_weather_data(format!("{}agile_tariff.csv", assets_dir));

    let hourly_outside_temperatures_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("outside_temps"));

    let hourly_solar_irradiances_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("solar_irradiances"));

    (
        agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year,
    )
}

#[allow(dead_code)]
fn run_simulation(inputs: &Inputs, config: &heat_ninja::Config) {
    let (
        agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year,
    ) = import_file_data(String::from("assets/"), inputs.latitude, inputs.longitude);

    let _json_results = heat_ninja::run_simulation(
        inputs.thermostat_temperature,
        inputs.latitude,
        inputs.longitude,
        inputs.num_occupants,
        inputs.house_size,
        &inputs.postcode,
        inputs.epc_space_heating,
        inputs.tes_volume_max,
        &agile_tariff_per_hour_over_year,
        &hourly_outside_temperatures_over_year,
        &hourly_solar_irradiances_over_year,
        &config,
    );

    // if config.return_format != heat_ninja::ReturnFormat::NONE {
    //     println!("{}", json_results);
    // }
}

#[allow(dead_code)]
fn run_simulations_using_input_file() {
    // import input list file
    // postcode, latitude, longitude, num_occupants, house_size, temp, epc_space_heating, tes_max
    let filepath = "tests/input_list.csv";
    println!("filepath: {}", filepath);
    let file = File::open(filepath).expect("cannot read file.");
    let reader = BufReader::new(file);
    let all_now = Instant::now();

    let file = File::create("tests/performance.csv").expect("could not open tests/performance.csv");
    let mut file: LineWriter<File> = LineWriter::new(file);

    file.write_fmt(format_args!(
        "Index,Nodes,Gain,Opt-Elapsed,No-Opt-Elapsed,No-Opt-Elapsed/Node\n"
    ))
    .expect("could not write to tests/performance.csv");

    for (i, line) in reader.lines().enumerate() {
        //if i < 28 {continue;}
        let just_now = Instant::now();
        let unwrapped_line = line.unwrap();
        let parts: Vec<&str> = unwrapped_line.split(',').collect();

        let postcode: String = String::from(parts[0]);
        let latitude: f32 = parts[1].parse().unwrap();
        let longitude: f32 = parts[2].parse().unwrap();
        let num_occupants: u8 = parts[3].parse().unwrap();
        let house_size: f32 = parts[4].parse().unwrap();
        let tes_volume_max: f32 = parts[7].parse().unwrap();
        let thermostat_temperature: f32 = parts[5].parse().unwrap();
        let epc_space_heating: f32 = parts[6].parse().unwrap();

        println!("Index: {}, Inputs: {:?}", i, parts);

        let inputs = Inputs {
            postcode,
            latitude,
            longitude,
            num_occupants,
            house_size,
            tes_volume_max,
            thermostat_temperature,
            epc_space_heating,
        };

        run_and_compare(inputs, i, &mut file);

        println!(
            "Elapsed: {} ms, Total: {} s",
            just_now.elapsed().as_millis(),
            all_now.elapsed().as_secs()
        );
    }
}

#[allow(dead_code)]
fn compare_result_folders(
    file_name_fmt_a: fn(usize) -> String,
    file_name_fmt_b: fn(usize) -> String,
    index_range: Range<usize>,
) {
    for file_index in index_range {
        println!("File Index: {}", file_index);
        let file_name_a: String = file_name_fmt_a(file_index);
        let file_name_b: String = file_name_fmt_b(file_index);

        compare_result_files(file_name_a, file_name_b, file_index);
    }
}

#[allow(dead_code)]
fn compare_result_files(file_name_a: String, file_name_b: String, file_index: usize) {
    let file_a = File::open(&file_name_a).expect("cannot read file.");
    let reader_a = BufReader::new(&file_a);

    let file_b = File::open(&file_name_b).expect("cannot read file.");
    let reader_b = BufReader::new(&file_b);

    for ((line_index, result_a), result_b) in reader_a.lines().enumerate().zip(reader_b.lines()) {
        //println!("{}, {}, {}", line_index, line_a.unwrap(), line_b.unwrap());
        let line_a = result_a.expect(&format!(
            "could not read line: {} of {}",
            line_index, &file_name_a
        ));
        let line_b = result_b.expect(&format!(
            "could not read line: {} of {}",
            line_index, &file_name_b
        ));
        if line_a != line_b {
            panic!(
                "line {} of files: {}, {}, do not match: \n{}\n{}\n. Surface: {}",
                line_index,
                file_name_a,
                file_name_b,
                line_a,
                line_b,
                file_index * 21 + line_index - 3,
            );
        }
    }
}

#[allow(dead_code)]
fn run_simulation_with_default_parameters() {
    let config: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results_as_csv: false,
        use_multithreading: true,
        save_results_as_csv: false,
        save_results_as_json: false,
        save_all_nodes_as_csv: false,
        print_results_as_json: false,
        save_surfaces: false,
        file_index: 60,
        use_surface_optimisation: true,
        return_format: heat_ninja::ReturnFormat::JSON,
    };

    let inputs = Inputs {
        thermostat_temperature: 20.0,
        latitude: 52.3833,
        longitude: -1.5833,
        num_occupants: 2,
        house_size: 60.0,
        postcode: String::from("CV47AL"),
        epc_space_heating: 3000.0,
        tes_volume_max: 0.5,
    };

    run_simulation(&inputs, &config);
    // run_python_simulation(&inputs, 1);
}

#[allow(dead_code)]
fn run_and_compare(inputs: Inputs, file_index: usize, file: &mut LineWriter<File>) {
    let mut config: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results_as_csv: false,
        use_surface_optimisation: true,
        use_multithreading: true,
        file_index: file_index,
        save_results_as_csv: true,
        save_all_nodes_as_csv: false,
        save_results_as_json: false,
        print_results_as_json: false,
        return_format: heat_ninja::ReturnFormat::NONE,
        save_surfaces: true,
    };

    let total_nodes: u16 =
        ((180 * ((inputs.house_size / 8.0) as u16) - 30) as f32 * inputs.tes_volume_max) as u16;

    let now = Instant::now();
    run_simulation(&inputs, &config);
    let opt_elapsed: u128 = now.elapsed().as_millis();

    let now = Instant::now();
    config.use_surface_optimisation = false;
    run_simulation(&inputs, &config);
    let no_opt_elapsed: u128 = now.elapsed().as_millis();

    println!(
        "Nodes: {:.0}, % Gain: {:.2}, Elapsed: {} vs {} ms, Elapsed/Node: {} ms",
        total_nodes,
        no_opt_elapsed as f32 / opt_elapsed as f32,
        opt_elapsed,
        no_opt_elapsed,
        no_opt_elapsed as f32 / total_nodes as f32
    );

    file.write_fmt(format_args!(
        "{},{:.0},{:.2},{},{},{:.4}\n",
        file_index,
        total_nodes,
        no_opt_elapsed as f32 / opt_elapsed as f32,
        opt_elapsed,
        no_opt_elapsed,
        no_opt_elapsed as f32 / total_nodes as f32
    ))
    .expect("could not write to tests/performance.csv");

    compare_result_files(
        String::from(format!("tests/results/o{}.csv", file_index)),
        String::from(format!("tests/results/{}.csv", file_index)),
        file_index,
    );
}

#[allow(dead_code)]
fn surf_test() {
    let min_num_segments: usize = 3;
    let target_step: usize = 7;
    let x_size = 35;
    let target_num_segments = 4;

    let x_num_segments = target_num_segments
        .max(x_size / target_step)
        .min(x_size - 1);

    let x_num_segments2 = (x_size / target_step).max(min_num_segments.min(x_size - 1));

    let linearly_space = |range: usize, num_segments: usize| -> Vec<usize> {
        let mut points: Vec<usize> = Vec::with_capacity(num_segments + 1);
        let step: f32 = (range as f32) / (num_segments as f32);
        let mut i: f32 = 0.0;
        loop {
            let j: usize = i.round() as usize;
            points.push(j);
            if j >= range {
                break;
            }
            i += step;
        }
        println!("Points: {:?}", points);
        points
    };
    println!("num segs: {}, {}", x_num_segments, x_num_segments2);

    // calculate initial points to search on surface
    let _is: Vec<usize> = linearly_space(x_size - 1, x_num_segments);
}

#[allow(dead_code)]
fn test_surfaces() {
    let target_step: usize = 100;
    let target_num_segments = 3;
    let perf_file =
        File::create("tests/perf_surf.csv").expect("could not open tests/perf_surf.csv");
    let mut perf_file: LineWriter<File> = LineWriter::new(perf_file);
    perf_file
        .write_fmt(format_args!(
            "Index,XNodes,YNodes,TotalNodes,NodesSearched,Gain\n"
        ))
        .expect("could not write to tests/performance.csv");

    for file_index in 0..20852 {
        let filepath = format!("tests/surfaces/{}.csv", file_index);

        //println!("filepath: {}", filepath);
        if Path::new(&filepath).exists() {
            let file = File::open(filepath).expect("cannot read file.");
            let reader = BufReader::new(file);
            let mut zs_all: Vec<f32> = Vec::new();
            let mut x_size = 0;
            let mut y_size = 0;
            let mut min_z_all = f32::MAX;
            for (i, line_result) in reader.lines().enumerate() {
                let line = line_result.expect("could not read line.");
                let parts = line.split(',');
                for part in parts {
                    let z = part.parse::<f32>().expect("string invalid float.");
                    if z < min_z_all {
                        min_z_all = z
                    };
                    zs_all.push(z);
                    if i == 0 {
                        x_size += 1
                    }
                }
                y_size += 1;
            }

            let mut surface_optimiser = |x_size: usize, y_size: usize| {
                // println!("SURFACE OPTIMISER");
                let mut min_z: f32 = f32::MAX;
                let mut max_mx: f32 = 0.0;
                let mut max_my: f32 = 0.0;

                let z_size = x_size * y_size;
                let mut zs: Vec<f32> = Vec::with_capacity(z_size);
                for _ in 0..z_size {
                    zs.push(f32::MAX);
                }

                let gradient_factor: f32 = if z_size > 200 { 0.12 } else { 0.38 };

                let x_num_segments = target_num_segments
                    .max(x_size / target_step)
                    .min(x_size - 1);
                let y_num_segments = target_num_segments
                    .max(y_size / target_step)
                    .min(y_size - 1);

                let linearly_space = |range: usize, num_segments: usize| -> Vec<usize> {
                    let mut points: Vec<usize> = Vec::with_capacity(num_segments + 1);
                    let step: f32 = (range as f32) / (num_segments as f32);
                    let mut i: f32 = 0.0;
                    loop {
                        let j: usize = i.round() as usize;
                        points.push(j);
                        if j >= range {
                            break;
                        }
                        i += step;
                    }
                    // println!("Points: {:?}", points);
                    points
                };

                // calculate initial points to search on surface
                let is: Vec<usize> = linearly_space(x_size - 1, x_num_segments);
                let js: Vec<usize> = linearly_space(y_size - 1, y_num_segments);

                #[derive(Debug)]
                struct Rect {
                    i1: usize,
                    i2: usize,
                    j1: usize,
                    j2: usize,
                }

                let mut get_or_calculate = |i: usize, j: usize, min_z: &mut f32| -> f32 {
                    let k: usize = i + x_size * j;
                    if zs[k] == f32::MAX {
                        let z = zs_all[k];
                        if z < *min_z {
                            *min_z = z
                        };
                        zs[k] = z;
                    }
                    zs[k]
                };

                // combine 1D x and y indices into a 2D mesh
                // currently adjacent nodes not removed (they should be calculated and removed)
                let mut index_rects: Vec<Rect> =
                    Vec::with_capacity(x_num_segments * y_num_segments);

                // println!("Segs: {}, {}, is: {:?}, {:?}", x_num_segments, y_num_segments, &is, &js);

                for j in 0..y_num_segments {
                    for i in 0..x_num_segments {
                        index_rects.push(Rect {
                            i1: is[i],
                            i2: is[i + 1],
                            j1: js[j],
                            j2: js[j + 1],
                        });
                    }
                }

                // calculate z for each position and set the min_z and steepest gradient for x & y
                for r in &index_rects {
                    let z11 = get_or_calculate(r.i1, r.j1, &mut min_z);
                    let z21 = get_or_calculate(r.i2, r.j1, &mut min_z);
                    let z12 = get_or_calculate(r.i1, r.j2, &mut min_z);
                    let z22 = get_or_calculate(r.i2, r.j2, &mut min_z);

                    let mx: f32 = ((z11 - z21) / ((r.i2 - r.i1) as f32)).abs();
                    let my: f32 = ((z11 - z12) / ((r.j2 - r.j1) as f32)).abs();
                    if mx > max_mx {
                        max_mx = mx
                    };
                    if my > max_my {
                        max_my = my
                    };

                    let mx: f32 = ((z22 - z21) / ((r.i2 - r.i1) as f32)).abs();
                    let my: f32 = ((z22 - z12) / ((r.j2 - r.j1) as f32)).abs();
                    if mx > max_mx {
                        max_mx = mx
                    };
                    if my > max_my {
                        max_my = my
                    };
                }
                max_mx *= gradient_factor;
                max_my *= gradient_factor;

                while !index_rects.is_empty() {
                    let mut next_rects: Vec<Rect> = Vec::new();
                    let min_z_of_iter: f32 = min_z;
                    // println!("Rect Group");
                    for r in &index_rects {
                        // calculate distance between indices
                        // println!("{:?}", r);
                        let di: usize = r.i2 - r.i1;
                        let dj: usize = r.j2 - r.j1;
                        let z11 = get_or_calculate(r.i1, r.j1, &mut min_z);
                        let z21 = get_or_calculate(r.i2, r.j1, &mut min_z);
                        let z12 = get_or_calculate(r.i1, r.j2, &mut min_z);
                        let z22 = get_or_calculate(r.i2, r.j2, &mut min_z);

                        // get node with lowest npc
                        let min_local_z: f32 = {
                            let mut m: f32 = z11;
                            if z21 < m {
                                m = z21
                            };
                            if z12 < m {
                                m = z12
                            };
                            if z22 < m {
                                m = z22
                            };
                            m
                        };

                        // estimate minimum npc between nodes
                        let min_z_estimate: f32 =
                            min_local_z - (max_mx * (di as f32) + max_my * (dj as f32));
                        // if segment could have npc lower than the current min subdivide
                        if min_z_estimate < min_z_of_iter {
                            // subdivide segments that can be divided further, otherwise leave at unit length
                            let i12: usize = if di == 1 { r.i2 } else { r.i1 + di / 2 };
                            if i12 != r.i2 {
                                get_or_calculate(i12, r.j1, &mut min_z);
                                get_or_calculate(i12, r.j2, &mut min_z);
                            }
                            let j12: usize = if dj == 1 { r.j2 } else { r.j1 + dj / 2 };
                            if j12 != r.j2 {
                                get_or_calculate(r.i1, j12, &mut min_z);
                                get_or_calculate(r.i2, j12, &mut min_z);
                            }
                            if i12 != r.i2 && j12 != r.j2 {
                                get_or_calculate(i12, j12, &mut min_z);
                            }

                            let sub_i1: bool = i12 - r.i1 > 1;
                            let sub_i2: bool = r.i2 - i12 > 1;
                            let sub_j1: bool = j12 - r.j1 > 1;
                            let sub_j2: bool = r.j2 - j12 > 1;

                            // one of the dimensions must have a length > 1 if the rect is to be subdivided further
                            if sub_i1 || sub_j1 {
                                next_rects.push(Rect {
                                    i1: r.i1,
                                    j1: r.j1,
                                    i2: i12,
                                    j2: j12,
                                })
                            };
                            if sub_i2 || sub_j1 {
                                next_rects.push(Rect {
                                    i1: i12,
                                    j1: r.j1,
                                    i2: r.i2,
                                    j2: j12,
                                })
                            };
                            if sub_i1 || sub_j2 {
                                next_rects.push(Rect {
                                    i1: r.i1,
                                    j1: j12,
                                    i2: i12,
                                    j2: r.j2,
                                })
                            };
                            if sub_i2 || sub_j2 {
                                next_rects.push(Rect {
                                    i1: i12,
                                    j1: j12,
                                    i2: r.i2,
                                    j2: r.j2,
                                })
                            };
                        }
                    }
                    index_rects = next_rects;
                }

                let nodes_searched = {
                    let mut nodes_searched: i32 = 0;
                    for z in &zs {
                        if *z != f32::MAX {
                            nodes_searched += 1;
                        }
                    }
                    nodes_searched
                };

                perf_file
                    .write_fmt(format_args!(
                        "{}, {}, {}, {}, {}, {}\n",
                        file_index,
                        x_size,
                        y_size,
                        z_size,
                        nodes_searched,
                        z_size as f32 / nodes_searched as f32
                    ))
                    .expect("could not write to tests/perf_surf.csv");
                // println!("{}, {}, {}, {}, {}, {}",
                //     file_index, x_size, y_size, z_size, nodes_searched,
                //     z_size as f32 / nodes_searched as f32);

                if min_z != min_z_all {
                    println!(
                        "{}, {}, {}, {}, {}, {}",
                        file_index,
                        x_size,
                        y_size,
                        z_size,
                        nodes_searched,
                        z_size as f32 / nodes_searched as f32
                    );
                    for j in 0..y_size {
                        for i in 0..x_size {
                            let k: usize = i + x_size * j;
                            if zs[k] != f32::MAX {
                                print!("# ");
                            } else {
                                print!("- ");
                            }
                        }
                        print!("\n");
                    }
                    panic!("failed to find min {}, {}", min_z, min_z_all);
                }
            };

            if x_size > 3 && y_size > 3 && (x_size * y_size > 50) {
                surface_optimiser(x_size, y_size);
            } else {
                let z_size = x_size * y_size;
                perf_file
                    .write_fmt(format_args!(
                        "{}, {}, {}, {}, {}, {}\n",
                        file_index, x_size, y_size, z_size, z_size, 1.0
                    ))
                    .expect("could not write to tests/perf_surf.csv");
            }
        }
    }
}

#[allow(dead_code)]
fn compare_rust_and_python(inputs: &Inputs, file_index: usize, file: &mut LineWriter<File>) {
    let config: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results_as_csv: false,
        use_surface_optimisation: false,
        use_multithreading: true,
        file_index: file_index,
        save_results_as_csv: true,
        save_results_as_json: false,
        save_all_nodes_as_csv: false,
        print_results_as_json: false,
        return_format: heat_ninja::ReturnFormat::NONE,
        save_surfaces: true,
    };

    let total_nodes: u16 =
        ((180 * ((inputs.house_size / 8.0) as u16) - 30) as f32 * inputs.tes_volume_max) as u16;

    let now = Instant::now();
    run_simulation(&inputs, &config);
    let rust_elapsed: u128 = now.elapsed().as_millis();

    let now = Instant::now();
    // run_python_simulation(&inputs, file_index);
    let python_elapsed: u128 = now.elapsed().as_millis();

    println!("Nodes: {:.0}, % Gain: {:.2}, Elapsed: Rust:  {}, Python: {} ms, Elapsed/Node: Rust: {}, Python: {} ms", total_nodes,
             rust_elapsed as f32 / python_elapsed as f32, rust_elapsed, python_elapsed, rust_elapsed as f32 / total_nodes as f32,
             python_elapsed as f32 / total_nodes as f32);

    file.write_fmt(format_args!(
        "{},{:.0},{:.0},{:.0},{:.4},{:.4}\n",
        total_nodes,
        rust_elapsed as f32 / python_elapsed as f32,
        rust_elapsed,
        python_elapsed,
        rust_elapsed as f32 / total_nodes as f32,
        python_elapsed as f32 / total_nodes as f32
    ))
    .expect("could not write to tests/rust_python_performance.csv");

    compare_result_files(
        String::from(format!("tests/results/{}.csv", file_index)),
        String::from(format!("tests/results/py_{}.csv", file_index)),
        file_index,
    );
}

#[allow(dead_code)]
fn run_python_simulation(inputs: &Inputs, file_index: usize) {
    const PYSIM_PATH: &str = "tests/pysim.py";
    //const PYSIM_PATH: &str =  "C:/dev/heat_ninja/src/sim_v1.py";
    let output_path: String = format!("tests/results/py_{}.csv", file_index);
    println!("test_python_sim");
    let mut cmd: Command = Command::new("python");
    cmd.args([
        PYSIM_PATH,
        &inputs.num_occupants.to_string(),
        &inputs.postcode,
        &inputs.latitude.to_string(),
        &inputs.longitude.to_string(),
        &inputs.epc_space_heating.to_string(),
        &inputs.house_size.to_string(),
        &inputs.tes_volume_max.to_string(),
        &inputs.thermostat_temperature.to_string(),
        &output_path,
    ]);
    let cmd2 = cmd.output().expect("failed to execute process");
    println!("stdout {}", String::from_utf8(cmd2.stdout).unwrap());
    println!("stderr {}", String::from_utf8(cmd2.stderr).unwrap());
    println!("test_python_sim done");
}

fn main() {
    let path = env::current_dir().expect("Could not locate current directory");
    println!("The current directory is {}", path.display());
    let now = Instant::now();

     run_simulation_with_default_parameters();
    //surf_test();
    //run_simulations_using_input_file();
    //test_surfaces();

    println!(
        "Elapsed: {} ms",
        now.elapsed().as_millis(),
    );
}
