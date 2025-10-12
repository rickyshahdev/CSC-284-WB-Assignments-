// Truck.h
#ifndef TRUCK_H
#define TRUCK_H

#include "Vehical.h"

class Truck : public Vehicle {
private:
    double loadCapacity; // in tons

public:
    Truck();
    Truck(const std::string& make, const std::string& model, int year, double mileage,
          double loadCapacity);
    virtual ~Truck();

    double getLoadCapacity() const;
    void setLoadCapacity(double c);

    virtual void displayInfo() const override;
    virtual double getFuelEfficiency() const override;
};

#endif // TRUCK_H
