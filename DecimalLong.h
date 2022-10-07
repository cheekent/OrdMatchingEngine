#pragma once

#include <numeric>
#include <cassert>
#include <limits>
#include <cmath>
#include <iostream>
#include <iomanip>

template <size_t mult>
inline constexpr size_t literalPow(size_t number)
{
	return number * literalPow<mult - 1>(number);
}
template <>
inline constexpr size_t literalPow<1>(size_t number)
{
	return number;
}
template <>
inline constexpr size_t literalPow<0>(size_t number)
{
	return 1;
}

template <unsigned decPoint>
class DecimalLong
{
public:
	using value_type = std::int64_t;
	using TMyself = DecimalLong<decPoint>;

	static constexpr size_t DecPoint = decPoint;
	static constexpr size_t DecPointMult = literalPow<DecPoint>(10);

	struct RawValue {
		value_type	value;
	};

	DecimalLong(std::int32_t numPart = 0L, std::uint16_t decPart = 0) :
		m_value(numPart * DecPointMult + decPart)
	{
		assert(decPart < DecPointMult);
	}
	explicit DecimalLong(double val) :
		m_value(static_cast<value_type>(val * DecPointMult))
	{}
	explicit DecimalLong(float val) :
		m_value(static_cast<value_type>(val * DecPointMult))
	{}
	DecimalLong(const TMyself::RawValue& raw) :
		m_value(raw.value)
	{}
	DecimalLong(const DecimalLong& other) :
		m_value(other.m_value)
	{}

	inline DecimalLong& operator=(const TMyself& rhs) {
		m_value = rhs.m_value;
		return *this;
	}

	inline operator double() const {
		return static_cast<double>(m_value / DecPointMult);
	}
	inline operator float() const {
		return static_cast<float>(m_value / DecPointMult);
	}

	inline bool operator==(const TMyself& rhs) const { return m_value == rhs.m_value; }
	inline bool operator<(const TMyself& rhs) const { return m_value < rhs.m_value; }
	inline bool operator<=(const TMyself& rhs) const { return m_value <= rhs.m_value; }
	inline bool operator>(const TMyself& rhs) const { return m_value > rhs.m_value; }
	inline bool operator>=(const TMyself& rhs) const { return m_value >= rhs.m_value; }

	friend inline std::ostream& operator<<(std::ostream& os, const TMyself& val)
	{
		os << val.m_value / DecPointMult;
		if (DecPoint > 0) {
			os << '.';
			os << std::setfill('0') << std::setw(DecPoint);
			os << (val.m_value < 0 ? -val.m_value : val.m_value) % DecPointMult;
		}
		return os;
	}

protected:
	value_type	m_value;
};

