// Car.h
#ifndef CAR_H
#define CAR_H

#include "Vehical.h"

class Car : public Vehicle {
private:
    int numDoors;
    bool isElectric;

public:
    Car();
    Car(const std::string& make, const std::string& model, int year, double mileage,
        int numDoors, bool isElectric);
    virtual ~Car();

    int getNumDoors() const;
    bool getIsElectric() const;

    void setNumDoors(int d);
    void setIsElectric(bool e);

    virtual void displayInfo() const override;
    virtual double getFuelEfficiency() const override;
};

#endif // CAR_H
