// Vehical.h
#ifndef VEHICAL_H
#define VEHICAL_H

#include <string>

class Vehicle {
protected:
    std::string make;
    std::string model;
    int year;
    double mileage;

    // Protected so derived classes can access if needed
    static int vehicleCount;

public:
    // Default constructor
    Vehicle();

    // Parameterized constructor
    Vehicle(const std::string& make, const std::string& model, int year, double mileage = 0.0);

    // Virtual destructor (also prints a message to demonstrate destructor order)
    virtual ~Vehicle();

    // Accessors
    std::string getMake() const;
    std::string getModel() const;
    int getYear() const;
    double getMileage() const;

    // Mutators (with simple validation)
    void setMake(const std::string& m);
    void setModel(const std::string& m);
    void setYear(int y);
    void setMileage(double mi);

    // Display info (virtual so derived classes can extend)
    virtual void displayInfo() const;

    // Pure virtual: derived classes must implement fuel efficiency
    virtual double getFuelEfficiency() const = 0;

    // Static accessor for vehicle count
    static int getVehicleCount();
};

#endif // VEHICAL_H
