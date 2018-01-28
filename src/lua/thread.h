#ifndef FLAT_LUA_THREAD_H
#define FLAT_LUA_THREAD_H

#include "sharedluareference.h"
#include "uniqueluareference.h"

namespace flat
{
namespace lua
{

class Thread
{
	public:
		Thread();

		void set(lua_State* L, int index);

		bool start(lua_State* L, int numArgs);
		void update(lua_State* L);

		void stop(lua_State* L);

		void reset(lua_State* L);

		inline bool isRunning() const { return m_status == LUA_YIELD; }
		inline bool isFinished() const { return m_status == LUA_OK; }
		inline bool isEmpty() const { return m_thread.isEmpty(); }

		FLAT_DEBUG_ONLY(const flat::lua::UniqueLuaReference<LUA_TTHREAD>& getThread() const { return m_thread; })

	protected:
		flat::lua::SharedLuaReference<LUA_TFUNCTION> m_function;
		flat::lua::UniqueLuaReference<LUA_TTHREAD> m_thread;
		int m_status;
};

} // lua
} // flat

#endif // FLAT_LUA_THREAD_H


