#ifndef FLAT_SHARP_NOTITY_NOTIFY_H
#define FLAT_SHARP_NOTITY_NOTIFY_H

#include "../../lua/uniqueluareference.h"

namespace flat
{
class Flat;
namespace sharp
{
namespace notify
{

class Notify
{
	public:
		Notify(Flat& flat);

		void reset();

		void success(const std::string& message);
		void warn(const std::string& message);
		void error(const std::string& message);
		void info(const std::string& message);

	private:
		Flat& m_flat;

		lua::UniqueLuaReference<LUA_TFUNCTION> m_success;
		lua::UniqueLuaReference<LUA_TFUNCTION> m_warn;
		lua::UniqueLuaReference<LUA_TFUNCTION> m_error;
		lua::UniqueLuaReference<LUA_TFUNCTION> m_info;
};

} // ui
} // sharp
} // flat

#endif // FLAT_SHARP_NOTIFY_NOTIFY_H


