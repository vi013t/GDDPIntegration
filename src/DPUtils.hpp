#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPUtils {
public:
	static std::vector<std::string> substring(std::string string, std::string delim);

	/**
	 * Returns whether a value exists in a given vector.
	 * 
	 * @param vector The vector to search
	 * @param value The value to search for
	 *
	 * @return Whether the value exists in the vector.
	 */
	template <typename T> static bool isInVector(const std::vector<T>& vector, const T& value) {
		return std::find(vector.begin(), vector.end(), value) != vector.end();
	}

	/**
	 * Appends a value to a vector if it isn't already present.
	 * 
	 * @param vector The vector to append to
	 * @param value The value to append
	 */
	template <typename T> static void addToVectorIfAbsent(std::vector<T>& vector, const T& value) {
		if (!isInVector(vector, value)) {
			vector.push_back(value);
		}
	}
};