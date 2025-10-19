// main.cpp
#include <iostream>
#include <vector>
#include "Statistics.h"

int main() {
	try {
		// Test with int
		Statistics<int> intStats;
		intStats.addValue(10);
		intStats.addValue(20);
		intStats.addValue(30);

		std::vector<int> intVec = {10, 20, 30};

		std::cout << "Int Stats (class): Min = " << intStats.getMin()
				  << ", Max = " << intStats.getMax()
				  << ", Avg = " << intStats.getAverage() << "\n";

		std::cout << "Int Stats (functions): Min = " << getMin(intVec)
				  << ", Max = " << getMax(intVec)
				  << ", Avg = " << getAverage(intVec) << "\n";

		// Test with double
		Statistics<double> doubleStats;
		doubleStats.addValue(3.14);
		doubleStats.addValue(2.71);
		doubleStats.addValue(4.5);

		std::vector<double> doubleVec = {3.14, 2.71, 4.5};

		std::cout << "Double Stats (class): Min = " << doubleStats.getMin()
				  << ", Max = " << doubleStats.getMax()
				  << ", Avg = " << doubleStats.getAverage() << "\n";

		std::cout << "Double Stats (functions): Min = " << getMin(doubleVec)
				  << ", Max = " << getMax(doubleVec)
				  << ", Avg = " << getAverage(doubleVec) << "\n";

		// Demonstrate using float as well
		Statistics<float> floatStats;
		floatStats.addValue(1.5f);
		floatStats.addValue(2.25f);
		floatStats.addValue(0.75f);

		std::vector<float> floatVec = {1.5f, 2.25f, 0.75f};

		std::cout << "Float Stats (class): Min = " << floatStats.getMin()
				  << ", Max = " << floatStats.getMax()
				  << ", Avg = " << floatStats.getAverage() << "\n";

		std::cout << "Float Stats (functions): Min = " << getMin(floatVec)
				  << ", Max = " << getMax(floatVec)
				  << ", Avg = " << getAverage(floatVec) << "\n";

	} catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << "\n";
		return 1;
	}

	return 0;
}

