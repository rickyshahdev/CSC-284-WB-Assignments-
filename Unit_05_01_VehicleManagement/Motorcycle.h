// Motorcycle.h
#ifndef MOTORCYCLE_H
#define MOTORCYCLE_H

#include "Vehical.h"

class Motorcycle : public Vehicle {
private:
    bool hasSidecar;

public:
    Motorcycle();
    Motorcycle(const std::string& make, const std::string& model, int year, double mileage,
               bool hasSidecar);
    virtual ~Motorcycle();

    bool getHasSidecar() const;
    void setHasSidecar(bool s);

    virtual void displayInfo() const override;
    virtual double getFuelEfficiency() const override;
};

#endif // MOTORCYCLE_H
