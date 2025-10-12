// Fleet.h
#ifndef FLEET_H
#define FLEET_H


#include <vector>
#include <memory>
#include "Vehical.h"

class Fleet {
private:
    std::vector<std::unique_ptr<Vehicle>> vehicles;

public:
    Fleet();
    ~Fleet();

    // Prefer taking ownership via unique_ptr
    void addVehicle(std::unique_ptr<Vehicle> v);
    // Convenience overload for raw pointers (wraps into unique_ptr)
    void addVehicle(Vehicle* v);

    void displayAllVehicles() const;
    double getAverageEfficiency() const;
    size_t size() const;
};

#endif // FLEET_H
