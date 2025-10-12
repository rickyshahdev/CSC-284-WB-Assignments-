// Fleet.cpp

#include "Fleet.h"
#include <iostream>

Fleet::Fleet() {}

Fleet::~Fleet() {
    std::cout << "Destroying Fleet (unique_ptr will free vehicles)..." << std::endl;
    // unique_ptrs in vector will automatically delete owned vehicles
}

void Fleet::addVehicle(std::unique_ptr<Vehicle> v) {
    vehicles.push_back(std::move(v));
}

void Fleet::addVehicle(Vehicle* v) {
    vehicles.emplace_back(v);
}

void Fleet::displayAllVehicles() const {
    std::cout << "--- Fleet Vehicles (" << vehicles.size() << ") ---\n" << std::endl;
    for (const auto& v : vehicles) {
        v->displayInfo();
    }
}

double Fleet::getAverageEfficiency() const {
    if (vehicles.empty()) return 0.0;
    double total = 0.0;
    for (const auto& v : vehicles) total += v->getFuelEfficiency();
    return total / vehicles.size();
}

size_t Fleet::size() const { return vehicles.size(); }

