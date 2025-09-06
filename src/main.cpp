#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include <stdio.h>
#include <boost/program_options.hpp>

using namespace std;

namespace po = boost::program_options;

static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a==string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

struct Task {
    string id;
    unsigned long long cycles;
    int core;
};


struct CoreStats {
	double cum_energy = 0.0;
};


vector<Task> read_tasks_csv(const string &path) {
    vector<Task> tasks;
    ifstream in(path);
    if (!in.is_open()) {
        cerr << "ERROR: failed to open tasks file: " << path << "\n";
        return tasks;
    }
    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        // id,cycles
        string id;
        string cycles_s;
        size_t comma = line.find(',');
        if (comma == string::npos) continue;
        id = trim(line.substr(0, comma));
        cycles_s = trim(line.substr(comma+1));
        if (id.empty() || cycles_s.empty()) continue;
        try {
            unsigned long long cycles = stoull(cycles_s);
            tasks.push_back({id, cycles});
        } catch(...) {
            cerr << "WARNING: skipping line (bad cycles): " << line << "\n";
            continue;
        }
    }
    return tasks;
}

void print_usage(const char *prog) {
    cerr << "Usage: " << prog << " --tasks <path> [--freq Hz] [--ipc X] [--alpha A] [--gamma G] [--p_idle P] [--out out.csv]\n";
}

int main(int argc, char* argv[]) {
    try {
        std::string task_file;
        std::string out_file;
        double freq, ipc, alpha, gamma, p_idle;
        int num_cores;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("tasks", po::value<std::string>(&task_file)->required(), "CSV file with tasks")
            ("freq", po::value<double>(&freq)->default_value(1e9), "CPU frequency (Hz)")
            ("ipc", po::value<double>(&ipc)->default_value(1.0), "Instructions per cycle")
            ("alpha", po::value<double>(&alpha)->default_value(1e-9), "Dynamic power coefficient")
            ("gamma", po::value<double>(&gamma)->default_value(1.0), "Frequency exponent")
            ("p_idle", po::value<double>(&p_idle)->default_value(0.5), "Idle power (W)")
            ("cores", po::value<int>(&num_cores)->default_value(1), "Number of CPU cores")
            ("out", po::value<std::string>(&out_file)->required(), "Output CSV file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        po::notify(vm);

        // Read tasks
        std::ifstream infile(task_file);
        if (!infile.is_open()) {
            std::cerr << "Error: Could not open task file " << task_file << "\n";
            return 1;
        }

        std::vector<Task> tasks;
        std::string line;
        bool header_skipped = false;
        while (std::getline(infile, line)) {
            if (!header_skipped) { header_skipped = true; continue; }
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string id, cycles_str, core_str;
            std::getline(ss, id, ',');
            std::getline(ss, cycles_str, ',');
            std::getline(ss, core_str, ',');

            Task t;
            t.id = id;
            t.cycles = std::stoll(cycles_str);
            if (!core_str.empty())
                t.core = std::stoi(core_str);
            else
                t.core = -1; // to be assigned later
            tasks.push_back(t);
        }

        // Assign cores if missing
        int rr = 0;
        for (auto &t : tasks) {
            if (t.core < 0) {
                t.core = rr % num_cores;
                rr++;
            }
        }

        std::vector<CoreStats> cores(num_cores);
        double system_cum_energy = 0.0;

        std::ofstream out(out_file);
        out << "id,core,cycles,time_s,power_w,energy_j,core_cum_energy_j,system_cum_energy_j\n";

        for (auto &t : tasks) {
            double time_s = static_cast<double>(t.cycles) / (freq * ipc);
            double util = 1.0;
            double power_w = p_idle + alpha * pow(freq, gamma) * util;
            double energy_j = power_w * time_s;

            cores[t.core].cum_energy += energy_j;
            system_cum_energy += energy_j;

            out << t.id << ","
                << t.core << ","
                << t.cycles << ","
                << time_s << ","
                << power_w << ","
                << energy_j << ","
                << cores[t.core].cum_energy << ","
                << system_cum_energy << "\n";
        }

        out.close();
        std::cout << "Wrote multi-core results to: " << out_file << "\n";

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
