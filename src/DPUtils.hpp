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

	template <typename Element> static int vectorIndexOf(std::vector<Element>& vector, const Element& value) {
		auto index = std::find(vector.begin(), vector.end(), value);
		if (index == vector.end()) return -1;
		return index - vector.begin();
	}

	template <typename Element> static void vectorRemove(std::vector<Element>& vector, const Element& value) {
		vector.erase(vector.begin() + vectorIndexOf(vector, value));
	}

	template <typename Element> static std::vector<Element> heapVector(std::vector<Element> const& vector) {
		auto heapVector = new std::vector<Element>();
		for (auto element : vector) {
			heapVector->push_back(element);
		}
		return heapVector;
	}

	template <typename Result, typename MapFunction, typename Operand> 
	requires std::invocable<MapFunction, Operand> && std::same_as<std::invoke_result_t<MapFunction, Operand>, Result>
	static std::vector<Result> vectorMap(const std::vector<Operand> &vector, MapFunction map) {
		std::vector<Result> result;
		result.reserve(vector.size());

		for (const auto &value : vector) {
			result.push_back(map(value));
		}

		return result;
	}
};