#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
using namespace std;

struct Process {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;

    Process(int i, int at, int bt) :
        id(i), arrival_time(at), burst_time(bt),
        remaining_time(bt), finish_time(0),
        waiting_time(0), turnaround_time(0) {}
};

class Scheduler {
private:
    vector<Process> processes;
    vector<pair<int, int>> gantt_chart;
    int quantum;

    void calculateMetrics() {
        for (auto& p : processes) {
            p.turnaround_time = p.finish_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
        }
    }

public:
    Scheduler(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file " << filename << endl;
            exit(EXIT_FAILURE);
        }

        int n, at, bt;
        file >> n >> quantum;
        if (file.fail() || n <= 0 || quantum <= 0) {
            cerr << "Error: Invalid file format or invalid parameters." << endl;
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            file >> at >> bt;
            if (file.fail() || at < 0 || bt <= 0) {
                cerr << "Error: Invalid process parameters in file." << endl;
                exit(EXIT_FAILURE);
            }
            processes.emplace_back(i + 1, at, bt);
        }
    }

    void runFCFS() {
        vector<Process> temp = processes;
        sort(temp.begin(), temp.end(),
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        int current_time = 0;
        gantt_chart.clear();

        for (auto& p : temp) {
            if (current_time < p.arrival_time)
                current_time = p.arrival_time;

            gantt_chart.push_back({ current_time, p.id });
            current_time += p.burst_time;

            
            auto& original = processes[p.id - 1];
            original.finish_time = current_time;
        }

        calculateMetrics();
    }

    void runSRT() {
        vector<Process> temp = processes;
        priority_queue<pair<int, Process*>,
            vector<pair<int, Process*>>,
            greater<pair<int, Process*>>> pq;

        int current_time = 0;
        int completed = 0;
        gantt_chart.clear();

        while (completed < processes.size()) {
            
            for (auto& p : temp) {
                if (p.arrival_time == current_time && p.remaining_time > 0) {
                    pq.push({ p.remaining_time, &p });
                }
            }

            if (!pq.empty()) {
                auto current = pq.top();
                pq.pop();

                gantt_chart.push_back({ current_time, current.second->id });
                current.second->remaining_time--;
                current_time++;

                if (current.second->remaining_time == 0) {
                    completed++;
                    processes[current.second->id - 1].finish_time = current_time;
                }
                else {
                    pq.push({ current.second->remaining_time, current.second });
                }
            }
            else {
                current_time++;
            }
        }

        calculateMetrics();
    }

    void runRR() {
        vector<Process> temp = processes;
        queue<Process*> rq;
        int current_time = 0;
        int completed = 0;
        gantt_chart.clear();

        while (completed < processes.size()) {
            
            for (auto& p : temp) {
                if (p.arrival_time == current_time && p.remaining_time > 0) {
                    rq.push(&p);
                }
            }

            if (!rq.empty()) {
                Process* current = rq.front();
                rq.pop();

                gantt_chart.push_back({ current_time, current->id });

                int execute_time = min(quantum, current->remaining_time);
                current->remaining_time -= execute_time;
                current_time += execute_time;

                if (current->remaining_time == 0) {
                    completed++;
                    processes[current->id - 1].finish_time = current_time;
                }
                else {
                    
                    for (auto& p : temp) {
                        if (p.arrival_time > current_time - execute_time &&
                            p.arrival_time <= current_time &&
                            p.remaining_time > 0) {
                            rq.push(&p);
                        }
                    }
                    rq.push(current);
                }
            }
            else {
                current_time++;
            }
        }

        calculateMetrics();
    }

    void displayResults(const string& algorithm) {
        cout << "\n=== " << algorithm << " Results ===\n\n";

        
        cout << "Gantt Chart:\n";
        cout << "-----------\n";
        for (const auto& g : gantt_chart) {
            cout << "Time " << g.first << ": Process " << g.second << "\n";
        }

        
        cout << "\nProcess Metrics:\n";
        cout << setw(10) << "Process"
            << setw(10) << "Arrival"
            << setw(10) << "Burst"
            << setw(10) << "Finish"
            << setw(10) << "Wait"
            << setw(15) << "Turnaround\n";

        double total_wait = 0, total_turnaround = 0;
        for (const auto& p : processes) {
            cout << setw(10) << p.id
                << setw(10) << p.arrival_time
                << setw(10) << p.burst_time
                << setw(10) << p.finish_time
                << setw(10) << p.waiting_time
                << setw(15) << p.turnaround_time << "\n";

            total_wait += p.waiting_time;
            total_turnaround += p.turnaround_time;
        }

        
        double avg_wait = total_wait / processes.size();
        double avg_turnaround = total_turnaround / processes.size();

        cout << "\nAverage Waiting Time: " << fixed << setprecision(2)
            << avg_wait << endl;
        cout << "Average Turnaround Time: " << avg_turnaround << endl;

        
        int total_burst_time = 0;
        int completion_time = processes[0].finish_time;

        for (const auto& p : processes) {
            total_burst_time += p.burst_time;
            completion_time = max(completion_time, p.finish_time);
        }

        double cpu_utilization = (total_burst_time * 100.0) / completion_time;
        cout << "CPU Utilization: " << cpu_utilization << "%\n";
    }
};

int main() {
    Scheduler scheduler("C:\\Users\\GIGABAYTE G5\\Downloads\\pp.txt");

    scheduler.runFCFS();
    scheduler.displayResults("First-Come First-Served (FCFS)");

    scheduler.runSRT();
    scheduler.displayResults("Shortest Remaining Time (SRT)");

    scheduler.runRR();
    scheduler.displayResults("Round-Robin (RR)");

    return 0;
}