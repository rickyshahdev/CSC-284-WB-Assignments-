// Statistics.h
#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>

// Function templates
template <typename T>
T getMin(const std::vector<T>& values) {
	if (values.empty()) {
		throw std::invalid_argument("getMin: values vector is empty");
	}
	return *std::min_element(values.begin(), values.end());
}

template <typename T>
T getMax(const std::vector<T>& values) {
	if (values.empty()) {
		throw std::invalid_argument("getMax: values vector is empty");
	}
	return *std::max_element(values.begin(), values.end());
}

template <typename T>
double getAverage(const std::vector<T>& values) {
	if (values.empty()) {
		throw std::invalid_argument("getAverage: values vector is empty");
	}
	// Use long double accumulator to reduce rounding/overflow for large sums
	long double sum = std::accumulate(values.begin(), values.end(), static_cast<long double>(0));
	return static_cast<double>(sum / values.size());
}

// Class template
template <typename T>
class Statistics {
private:
	std::vector<T> data;

public:
	void addValue(const T& value) {
		data.push_back(value);
	}

	T getMin() const {
		if (data.empty()) {
			throw std::invalid_argument("Statistics::getMin: no data available");
		}
		return *std::min_element(data.begin(), data.end());
	}

	T getMax() const {
		if (data.empty()) {
			throw std::invalid_argument("Statistics::getMax: no data available");
		}
		return *std::max_element(data.begin(), data.end());
	}

	double getAverage() const {
		if (data.empty()) {
			throw std::invalid_argument("Statistics::getAverage: no data available");
		}
		long double sum = std::accumulate(data.begin(), data.end(), static_cast<long double>(0));
		return static_cast<double>(sum / data.size());
	}

	// optional: access to underlying data
	const std::vector<T>& getData() const { return data; }
};

