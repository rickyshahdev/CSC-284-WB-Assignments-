// Car.cpp
#include "Car.h"
#include <iostream>

Car::Car()
    : Vehicle(), numDoors(4), isElectric(false) {}

Car::Car(const std::string& make, const std::string& model, int year, double mileage,
         int numDoors, bool isElectric)
    : Vehicle(make, model, year, mileage), numDoors(numDoors), isElectric(isElectric) {}

Car::~Car() {
    std::cout << "[Car::~Car] " << year << " " << make << " " << model << " being destroyed." << std::endl;
}

int Car::getNumDoors() const { return numDoors; }
bool Car::getIsElectric() const { return isElectric; }

void Car::setNumDoors(int d) { if (d > 0) numDoors = d; }
void Car::setIsElectric(bool e) { isElectric = e; }

void Car::displayInfo() const {
    Vehicle::displayInfo();
    std::cout << "  Type: Car | Doors: " << numDoors
              << " | Electric: " << (isElectric ? "Yes" : "No")
              << " | Fuel Efficiency: " << getFuelEfficiency() << " mpg\n" << std::endl;
}

double Car::getFuelEfficiency() const {
    // Revised calculation:
    // Electric cars have a high 'equivalent' efficiency.
    if (isElectric) return 120.0; // sample electric efficiency

    double base = 32.0; // slightly higher modern base
    // older cars degrade linearly with age up to 30 years
    double ageYears = 2025 - year;
    if (ageYears < 0) ageYears = 0;
    double ageFactor = 1.0 - (ageYears / 60.0); // gentle degradation
    if (ageFactor < 0.5) ageFactor = 0.5;
    // mileage penalty: heavier penalty after 100k miles
    double mileageFactor = 1.0 - (mileage / 250000.0);
    if (mileageFactor < 0.5) mileageFactor = 0.5;
    return base * ageFactor * mileageFactor;
}
