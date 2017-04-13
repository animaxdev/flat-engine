#ifndef FLAT_LUA_THREAD_H
#define FLAT_LUA_THREAD_H

#include "sharedluareference.h"

namespace flat
{
namespace lua
{

class Thread
{
	public:
		Thread();

		void set(lua_State* L, int index);

		void start(int numArgs);
		void update();

		void stop();

		inline bool isRunning() const { return !m_thread.isEmpty() && m_status == LUA_YIELD; }
		inline bool isFinished() const { return m_status == LUA_OK; }

	protected:
		flat::lua::SharedLuaReference<LUA_TFUNCTION> m_function;
		flat::lua::SharedLuaReference<LUA_TTHREAD> m_thread;
		int m_status;
};

} // lua
} // flat

#endif // FLAT_LUA_THREAD_H

