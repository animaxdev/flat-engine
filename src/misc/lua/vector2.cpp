#include "vector2.h"
#include "../../lua/sharedcppreference.h"

namespace flat
{
namespace misc
{
namespace lua
{

using LuaVector2 = flat::lua::SharedCppValue<Vector2>;

int openVector2(lua_State* L)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);

	static const luaL_Reg Vector2_lib_m[] = {
		{"x",       l_Vector2_x},
		{"y",       l_Vector2_y},

		{"length",  l_Vector2_length},
		{"length2", l_Vector2_length2},

		{"__add",   l_Vector2_add},
		{"__sub",   l_Vector2_sub},

		{nullptr, nullptr}
	};
	LuaVector2::registerClass("Flat.Vector2", L, Vector2_lib_m);

	// constructor: Flat.Vector2(x, y)
	lua_getglobal(L, "Flat");
	lua_pushcfunction(L, l_Vector2);
	lua_setfield(L, -2, "Vector2");

	lua_pop(L, 1);

	return 0;
}

int l_Vector2(lua_State* L)
{
	float x = static_cast<float>(luaL_checknumber(L, 1));
	float y = static_cast<float>(luaL_checknumber(L, 2));
	pushVector2(L, Vector2(x, y));
	return 1;
}

int l_Vector2_x(lua_State* L)
{
	Vector2& vector2 = getVector2(L, 1);
	if (lua_isnoneornil(L, 2))
	{
		lua_pushnumber(L, vector2.x);
		return 1;
	}
	else
	{
		vector2.x = static_cast<float>(luaL_checknumber(L, 2));
		return 0;
	}
}

int l_Vector2_y(lua_State* L)
{
	Vector2& vector2 = getVector2(L, 1);
	if (lua_isnoneornil(L, 2))
	{
		lua_pushnumber(L, vector2.y);
		return 1;
	}
	else
	{
		vector2.y = static_cast<float>(luaL_checknumber(L, 2));
		return 0;
	}
}

int l_Vector2_length(lua_State* L)
{
	Vector2& vector2 = getVector2(L, 1);
	lua_pushnumber(L, length(vector2));
	return 1;
}

int l_Vector2_length2(lua_State* L)
{
	Vector2& vector2 = getVector2(L, 1);
	lua_pushnumber(L, length2(vector2));
	return 1;
}

int l_Vector2_add(lua_State* L)
{
	pushVector2(L, getVector2(L, 1) + getVector2(L, 2));
	return 1;
}

int l_Vector2_sub(lua_State* L)
{
	pushVector2(L, getVector2(L, 1) - getVector2(L, 2));
	return 1;
}

Vector2& getVector2(lua_State* L, int index)
{
	return LuaVector2::get(L, index);
}

void pushVector2(lua_State* L, const Vector2& vector2)
{
	LuaVector2::pushNew(L, vector2);
}



} // lua
} // misc
} // game