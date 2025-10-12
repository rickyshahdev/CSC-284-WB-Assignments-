// Truck.cpp
#include "Truck.h"
#include <iostream>

Truck::Truck()
    : Vehicle(), loadCapacity(1.0) {}

Truck::Truck(const std::string& make, const std::string& model, int year, double mileage,
             double loadCapacity)
    : Vehicle(make, model, year, mileage), loadCapacity(loadCapacity) {}

Truck::~Truck() {
    std::cout << "[Truck::~Truck] " << year << " " << make << " " << model << " being destroyed." << std::endl;
}

double Truck::getLoadCapacity() const { return loadCapacity; }
void Truck::setLoadCapacity(double c) { if (c >= 0.0) loadCapacity = c; }

void Truck::displayInfo() const {
    Vehicle::displayInfo();
    std::cout << "  Type: Truck | Load Capacity: " << loadCapacity << " tons"
              << " | Fuel Efficiency: " << getFuelEfficiency() << " mpg\n" << std::endl;
}

double Truck::getFuelEfficiency() const {
    // Trucks are less efficient: base 12 mpg, penalty for capacity
    double base = 10.0; // realistic lower baseline
    double capacityPenalty = 1.0 + (loadCapacity * 0.15);
    double ageYears = 2025 - year;
    if (ageYears < 0) ageYears = 0;
    double ageFactor = 1.0 - (ageYears / 80.0);
    if (ageFactor < 0.6) ageFactor = 0.6;
    double mileageFactor = 1.0 - (mileage / 350000.0);
    if (mileageFactor < 0.45) mileageFactor = 0.45;
    return (base * mileageFactor * ageFactor) / capacityPenalty;
}
