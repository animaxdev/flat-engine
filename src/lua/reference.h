#ifndef FLAT_LUA_REFERENCE_H
#define FLAT_LUA_REFERENCE_H

#include <memory>
#include "../debug/assert.h"

namespace flat
{
namespace lua
{

template <int LuaType>
class Reference
{
	public:
		Reference(lua_State* L, int index) :
			m_luaState(L),
			m_luaReference(LUA_NOREF)
		{
			FLAT_ASSERT(lua_type(L, index) == LuaType);
			lua_pushvalue(L, index);
			m_luaReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		
		~Reference()
		{
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_luaReference);
			m_luaReference = LUA_NOREF;
		}
		
		inline lua_State* getLuaState() const { return m_luaState; }
		inline int getLuaReference() const { return m_luaReference; }
		
	private:
		lua_State* m_luaState;
		int m_luaReference;
};

template <int LuaType>
class SharedReference : public std::shared_ptr<Reference<LuaType>>
{
	typedef std::shared_ptr<Reference<LuaType>> Super;
	public:
		SharedReference() {}
		
		SharedReference(lua_State* L, int index)
		{
			set(L, index);
		}
		
		void set(lua_State* L, int index)
		{
			Reference<LuaType>* reference = new Reference<LuaType>(L, index);
			Super::reset(reference);
		}
		
		lua_State* push() const
		{
			Reference<LuaType>* reference = Super::get();
			FLAT_ASSERT(reference);
			lua_State* L = reference->getLuaState();
			int luaReference = reference->getLuaReference();
			FLAT_ASSERT(luaReference != LUA_NOREF);
			lua_rawgeti(L, LUA_REGISTRYINDEX, luaReference);
			return L;
		}
};

} // lua
} // flat

#endif // FLAT_LUA_REFERENCE_H



