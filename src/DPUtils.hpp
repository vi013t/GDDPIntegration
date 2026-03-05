#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPUtils {
public:
	static std::vector<std::string> substring(std::string string, std::string delim);

	template <typename T> static bool isInVector(std::vector<T>& vector, T value) {
		return std::find(vector.begin(), vector.end(), value) != vector.end();
	}

	template <typename T> static void addToVectorIfAbsent(std::vector<T>& vector, T value) {
		if (!isInVector(vector, value)) {
			vector.push_back(value);
		}
	}
};