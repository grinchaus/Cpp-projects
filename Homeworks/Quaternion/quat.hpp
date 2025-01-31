#pragma once

#include <cmath>

template< typename T >
struct matrix_t
{
	T data[16];
};

template< typename T >
struct vector3_t
{
	T x, y, z;
};

template< typename T >
class Quat
{
  public:
	Quat() : m_value{ 0, 0, 0, 0 } {}
	Quat(T a, T b, T c, T d) : m_value{ b, c, d, a } {}
	Quat(T angle, bool radians, const vector3_t< T > &vec)
	{
		T norma = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
		if (norma == 0)
		{
			Quat();
			return;
		}
		vector3_t< T > res{ vec.x / norma, vec.y / norma, vec.z / norma };
		T halfAngle = !radians ? (angle * ((T)std::acos(-1.0))) / 180 : angle;
		m_value[3] = std::cos(halfAngle / 2);
		m_value[0] = std::sin(halfAngle / 2) * res.x;
		m_value[1] = std::sin(halfAngle / 2) * res.y;
		m_value[2] = std::sin(halfAngle / 2) * res.z;
	}

	Quat< T > operator+(const Quat< T > &other) const
	{
		Quat< T > result = *this;
		result += other;
		return result;
	}

	Quat< T > &operator+=(const Quat< T > &other)
	{
		for (int i = 0; i < 4; i++)
		{
			m_value[i] += other.m_value[i];
		}
		return *this;
	}

	Quat< T > operator-(const Quat< T > &other) const
	{
		Quat< T > result = *this;
		result -= other;
		return result;
	}

	Quat< T > &operator-=(const Quat< T > &other)
	{
		for (int i = 0; i < 4; i++)
		{
			m_value[i] -= other.m_value[i];
		}
		return *this;
	}

	Quat< T > operator*(const Quat< T > &other) const
	{
		T x = m_value[3] * other.m_value[3] - m_value[0] * other.m_value[0] - m_value[1] * other.m_value[1] -
			  m_value[2] * other.m_value[2];
		T y = m_value[3] * other.m_value[0] + m_value[0] * other.m_value[3] + m_value[1] * other.m_value[2] -
			  m_value[2] * other.m_value[1];
		T z = m_value[3] * other.m_value[1] + m_value[1] * other.m_value[3] + m_value[2] * other.m_value[0] -
			  m_value[0] * other.m_value[2];
		T w = m_value[3] * other.m_value[2] + m_value[2] * other.m_value[3] + m_value[0] * other.m_value[1] -
			  m_value[1] * other.m_value[0];
		return Quat< T >(x, y, z, w);
	}

	Quat< T > operator*(T scalar) const
	{
		Quat< T > result;
		for (int i = 0; i < 4; ++i)
		{
			result.m_value[i] = m_value[i] * scalar;
		}
		return result;
	}

	Quat< T > operator*(const vector3_t< T > &vec) const
	{
		T x = -m_value[0] * vec.x - m_value[1] * vec.y - m_value[2] * vec.z;
		T y = m_value[3] * vec.x + m_value[1] * vec.z - m_value[2] * vec.y;
		T z = m_value[3] * vec.y - m_value[0] * vec.z + m_value[2] * vec.x;
		T w = m_value[3] * vec.z + m_value[0] * vec.y - m_value[1] * vec.x;
		return Quat< T >(x, y, z, w);
	}

	bool operator==(const Quat< T > &other) const
	{
		for (int i = 0; i < 4; i++)
		{
			if (m_value[i] != other.m_value[i])
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const Quat< T > &other) const { return !(*this == other); }

	Quat< T > operator~() const
	{
		T x = -m_value[0];
		T y = -m_value[1];
		T z = -m_value[2];
		T w = m_value[3];
		return { w, x, y, z };
	}

	explicit operator T() const
	{
		T modulus = std::sqrt(m_value[0] * m_value[0] + m_value[1] * m_value[1] + m_value[2] * m_value[2] + m_value[3] * m_value[3]);
		return modulus;
	}

	matrix_t< T > matrix() const
	{
		T x = m_value[3];
		T y = m_value[0];
		T z = m_value[1];
		T w = m_value[2];
		return { x, -y, -z, -w, y, x, -w, z, z, w, x, -y, w, -z, y, x };
	}

	matrix_t< T > rotation_matrix() const
	{
		T norma = T(*this);
		if (norma == 0)
		{
			return { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		}
		T x = m_value[0] / norma;
		T y = m_value[1] / norma;
		T z = m_value[2] / norma;
		T w = m_value[3] / norma;
		return {
			1 - 2 * y * y - 2 * z * z,
			2 * x * y + 2 * z * w,
			2 * x * z - 2 * y * w,
			0,
			2 * x * y - 2 * z * w,
			1 - 2 * x * x - 2 * z * z,
			2 * y * z + 2 * x * w,
			0,
			2 * x * z + 2 * y * w,
			2 * y * z - 2 * x * w,
			1 - 2 * x * x - 2 * y * y,
			0,
			0,
			0,
			0,
			1
		};
	}

	T angle(const bool degrees = true) const
	{
		T norma = T(*this);
		if (norma == 0)
		{
			return norma;
		}
		T halfAngle = 2 * std::acos(m_value[3] / norma);
		return !degrees ? halfAngle * 180 / ((T)std::acos(-1.0)) : halfAngle;
	}

	vector3_t< T > apply(const vector3_t< T > &vec) const
	{
		T x = m_value[0];
		T y = m_value[1];
		T z = m_value[2];
		T w = m_value[3];
		T norma = T(*this);
		if (norma == 0)
		{
			return { 0, 0, 0 };
		}
		Quat< T > q{ w / norma, x / norma, y / norma, z / norma };
		Quat< T > res = q * vec * (~q);
		return { res.m_value[0], res.m_value[1], res.m_value[2] };
	}

	const T *data() const { return m_value; }

  private:
	T m_value[4];
};
