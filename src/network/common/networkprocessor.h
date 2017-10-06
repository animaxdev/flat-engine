#ifndef FLAT_NETWORK_COMMON_NETWORKPROCESSOR_H
#define FLAT_NETWORK_COMMON_NETWORKPROCESSOR_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "../../misc/vector.h"

namespace flat
{
namespace network
{
namespace common
{

class NetworkProcessor
{
	private:
		enum class Delimiter : std::uint8_t
		{
			UNDEFINED,
			VECTOR,
			MAP,
			UINT8,
			UINT16,
			UINT32,
			INT8,
			INT16,
			INT32,
			FLOAT,
			VECTOR2,
			VECTOR3,
			VECTOR4
		};

		template <class T>
		static constexpr Delimiter delimiterForType = Delimiter::UNDEFINED;

		template <> static constexpr Delimiter delimiterForType<std::uint8_t>  = Delimiter::UINT8;
		template <> static constexpr Delimiter delimiterForType<std::uint16_t> = Delimiter::UINT16;
		template <> static constexpr Delimiter delimiterForType<std::uint32_t> = Delimiter::UINT32;
		template <> static constexpr Delimiter delimiterForType<std::int8_t>   = Delimiter::INT8;
		template <> static constexpr Delimiter delimiterForType<std::int16_t>  = Delimiter::INT16;
		template <> static constexpr Delimiter delimiterForType<std::int32_t>  = Delimiter::INT32;
		template <> static constexpr Delimiter delimiterForType<float>         = Delimiter::FLOAT;
		template <> static constexpr Delimiter delimiterForType<Vector2>       = Delimiter::VECTOR2;
		template <> static constexpr Delimiter delimiterForType<Vector3>       = Delimiter::VECTOR3;
		template <> static constexpr Delimiter delimiterForType<Vector4>       = Delimiter::VECTOR4;

	public:
		virtual bool isReading() const { return false; }
		virtual bool isWriting() const { return false; }

		template <class T>
		bool processWithDelimiter(T& value) const;

		virtual bool processDelimiter(Delimiter delimiter) const = 0;

		virtual bool process(bool& value) const = 0;
		virtual bool process(std::uint8_t& value) const = 0;
		virtual bool process(std::uint16_t& value) const = 0;
		virtual bool process(std::uint32_t& value) const = 0;
		virtual bool process(std::int8_t& value) const = 0;
		virtual bool process(std::int16_t& value) const = 0;
		virtual bool process(std::int32_t& value) const = 0;
		virtual bool process(Vector2& value) const = 0;
		virtual bool process(Vector3& value) const = 0;
		virtual bool process(Vector4& value) const = 0;

};

class NetworkReader : public NetworkProcessor
{
	public:
		bool isReading() const override { return true; }
};

class NetworkWriter : public NetworkProcessor
{
	public:
		bool isWriting() const override { return true; }
};

template<class T>
inline bool NetworkProcessor::processWithDelimiter(T& value) const
{
	processDelimiter(delimiterForType<T>);
	process(value);
	return true;
}

template <class T>
inline bool NetworkProcessor::processWithDelimiter(std::vector<T>& value) const
{
	processDelimiter(Delimiter::VECTOR);
	processDelimiter(delimiterForType<T>);
	if (isReading())
	{
		std::uint16_t size;
		process(size);
		value.reserve(size);
		for (std::uint16_t i = 0; i < size; ++i)
		{
			T& item = value.emplace_back();
			process(item);
		}
	}
	else
	{
		MC_ASSERT(isWriting());
		std::uint16_t size = static_cast<std::uint16_t>(value.size());
		process(size);
		for (T& item : value)
		{
			process(item);
		}
	}
	return true;
}

template <class K, class V>
inline bool NetworkProcessor::processWithDelimiter(std::unordered_map<K, V>& value) const
{
	processDelimiter(Delimiter::MAP);
	processDelimiter(delimiterForType<K>);
	processDelimiter(delimiterForType<V>);
	if (isReading())
	{
		std::uint16_t size;
		process(size);
		value.reserve(size);
		for (std::uint16_t i = 0; i < size; ++i)
		{
			K key;
			process(key);
			V value;
			process(value);
			value[key] = value;
		}
	}
	else
	{
		MC_ASSERT(isWriting());
		std::uint16_t size = static_cast<std::uint16_t>(value.size());
		process(size);
		for (std::pair<K, V>& item : value)
		{
			process(item.first);
			process(item.second);
		}
	}
	return true;
}

} // common
} // network
} // flat


#endif // FLAT_NETWORK_COMMON_NETWORKPROCESSOR_H


