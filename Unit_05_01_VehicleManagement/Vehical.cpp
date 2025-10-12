// Vehical.cpp
#include "Vehical.h"
#include <iostream>

int Vehicle::vehicleCount = 0;

Vehicle::Vehicle()
    : make("Unknown"), model("Unknown"), year(0), mileage(0.0) {
    ++vehicleCount;
}

Vehicle::Vehicle(const std::string& make, const std::string& model, int year, double mileage)
    : make(make), model(model), year(year), mileage(mileage) {
    ++vehicleCount;
}

Vehicle::~Vehicle() {
    std::cout << "[Vehicle::~Vehicle] " << year << " " << make << " " << model << " being destroyed." << std::endl;
    --vehicleCount;
}

std::string Vehicle::getMake() const { return make; }
std::string Vehicle::getModel() const { return model; }
int Vehicle::getYear() const { return year; }
double Vehicle::getMileage() const { return mileage; }

void Vehicle::setMake(const std::string& m) { if (!m.empty()) make = m; }
void Vehicle::setModel(const std::string& m) { if (!m.empty()) model = m; }
void Vehicle::setYear(int y) { if (y >= 1886) year = y; /* first gasoline car 1886 */ }
void Vehicle::setMileage(double mi) { if (mi >= 0.0) mileage = mi; }

void Vehicle::displayInfo() const {
    std::cout << "Vehicle: " << year << " " << make << " " << model
              << " | Mileage: " << mileage << std::endl;
}

int Vehicle::getVehicleCount() { return vehicleCount; }
