#include <iostream>
#include <vector>

using namespace std;
// Define an enumeration for traffic control indicators
enum TrafficControlIndicators { RED = 0, GREEN = 1, YELLOW = 2 };

// Global vector to hold traffic signal sequences
vector<string> trafficSignalSequences;

// Function to display traffic light sequences
void displayTrafficLights(int c) {
    for (int i = 0; i < c; i++) {
        int x = i % trafficSignalSequences.size();
        cout << trafficSignalSequences[x] << endl;
    }
}

int main() {
    // Initialize the traffic signal sequences
    trafficSignalSequences.push_back("Red");
    trafficSignalSequences.push_back("Green");
    trafficSignalSequences.push_back("Yellow");

    int n;
    // Get user input for the number of cycles
    cout << "how many cycles to run?:";
    cin >> n;
    // Display the traffic light sequences
    displayTrafficLights(n);

}

