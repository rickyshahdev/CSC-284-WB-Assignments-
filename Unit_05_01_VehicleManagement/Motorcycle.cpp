// Motorcycle.cpp
#include "Motorcycle.h"
#include <iostream>

Motorcycle::Motorcycle()
    : Vehicle(), hasSidecar(false) {}

Motorcycle::Motorcycle(const std::string& make, const std::string& model, int year, double mileage,
                       bool hasSidecar)
    : Vehicle(make, model, year, mileage), hasSidecar(hasSidecar) {}

Motorcycle::~Motorcycle() {
    std::cout << "[Motorcycle::~Motorcycle] " << year << " " << make << " " << model << " being destroyed." << std::endl;
}

bool Motorcycle::getHasSidecar() const { return hasSidecar; }
void Motorcycle::setHasSidecar(bool s) { hasSidecar = s; }

void Motorcycle::displayInfo() const {
    Vehicle::displayInfo();
    std::cout << "  Type: Motorcycle | Sidecar: " << (hasSidecar ? "Yes" : "No")
              << " | Fuel Efficiency: " << getFuelEfficiency() << " mpg\n" << std::endl;
}

double Motorcycle::getFuelEfficiency() const {
    // Motorcycles are efficient: base 55-70 depending on model
    double base = 65.0;
    if (hasSidecar) base *= 0.85;
    double ageYears = 2025 - year;
    if (ageYears < 0) ageYears = 0;
    double ageFactor = 1.0 - (ageYears / 70.0);
    if (ageFactor < 0.65) ageFactor = 0.65;
    double mileageFactor = 1.0 - (mileage / 200000.0);
    if (mileageFactor < 0.6) mileageFactor = 0.6;
    return base * ageFactor * mileageFactor;
}
