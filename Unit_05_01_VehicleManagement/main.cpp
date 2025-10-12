// main.cpp
#include <iostream>
#include "Vehical.h"
#include "Car.h"
#include "Truck.h"
#include "Motorcycle.h"
#include "Fleet.h"

int main() {
	Fleet fleet;

	// Create vehicles using unique_ptr and add to fleet
	fleet.addVehicle(std::make_unique<Car>("Toyota", "Camry", 2018, 45000.0, 4, false));
	fleet.addVehicle(std::make_unique<Car>("Tesla", "Model 3", 2022, 12000.0, 4, true));
	fleet.addVehicle(std::make_unique<Truck>("Ford", "F-150", 2015, 90000.0, 2.0));
	fleet.addVehicle(std::make_unique<Motorcycle>("Yamaha", "MT-07", 2020, 8000.0, false));

	// Display all vehicles
	fleet.displayAllVehicles();

	// Add a blank line to separate fleet listing from summary output
	std::cout << std::endl;

	// Show total vehicle count
	std::cout << "Total vehicles created: " << Vehicle::getVehicleCount() << std::endl;


	std::cout << "Average fleet fuel efficiency: " << fleet.getAverageEfficiency() << " mpg" << std::endl;

	std::cout << "\nProgram ending - destructors will run in order." << std::endl;
	return 0;
}

