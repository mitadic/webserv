#pragma once

#include "RequestUtils.hpp"

// Default template
template<typename T>
struct MaxValue {
    static T value() {
        return T(); // fallback for unsupported types, returns default value
    }
};

// Specialization for int
template<>
struct MaxValue<int> {
    static int value() {
        return INT_MAX;
    }
};
// Specialization for long long
template<>
struct MaxValue<int64_t> {
    static int64_t value() {
        return INT64_MAX;
    }
};
// Specialization for uint16_t
template<>
struct MaxValue<uint16_t> {
    static uint16_t value() {
        return UINT16_MAX;
    }
};

template<typename T>
T get_max_value() {
    return MaxValue<T>::value();
}


/** @param s the const string reference to extract from
 * @param num the <typename T> reference to store the result into
 * @return 0 for OK, 1 to signalize failure  
 * Returns 1 on any non-digit [.+-]. Tolerates leading 0s */
template <typename T>
int	webserv_atoi_set(const std::string& s, T& num)
{
    if (s.empty() || s.length() > 18) // capping at 18 digits for <signed long long> practicality, though RFC allows "any whole positive number"
		return 1;

	long long res = 0;
	std::string::const_iterator it = s.begin();
	for (; it != s.end() && *it == '0'; it++)  // RFC allows leading 0
		;
	if (it == s.end())
	{
		num = 0;
		return OK;
	}

	for (; it != s.end(); it++)
	{
		if (!std::isdigit(*it) || (res * 10 > get_max_value<T>()))  // RFC disallows +-, as well as any nondigits
			return 1;
		res = res * 10 + (*it - '0');
	}
	num = res;
	return OK;
}