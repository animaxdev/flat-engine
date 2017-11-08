#include <iostream>
#include <fstream>

#include "memorysnapshot.h"
#include "lua.h"
#include "debug.h"
#include "sharedcppreference.h"

namespace flat
{
namespace lua
{
namespace snapshot
{

using LuaMemorySnapshot = flat::lua::SharedCppReference<MemorySnapshot>;

int open(Lua& lua)
{
	lua_State* L = lua.state;
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);

	static const luaL_Reg flat_lua_snapshot_lib_s[] = {
		{"snapshot", l_flat_lua_snapshot_snapshot},
		{"diff",     l_flat_lua_snapshot_diff},

		{nullptr, nullptr}
	};

	lua_getglobal(L, "flat");
	lua_getfield(L, -1, "lua");
	luaL_newlib(L, flat_lua_snapshot_lib_s);
	lua_setfield(L, -2, "snapshot");

	lua.registerClass<LuaMemorySnapshot>("flat.MemorySnapshot");

	lua_pop(L, 2);

	return 0;
}

int l_flat_lua_snapshot_snapshot(lua_State * L)
{
	std::shared_ptr<MemorySnapshot> snapshot = std::make_shared<MemorySnapshot>(L);
	LuaMemorySnapshot::pushNew(L, snapshot);
	return 1;
}

int l_flat_lua_snapshot_diff(lua_State* L)
{
	MemorySnapshot& snapshot1 = LuaMemorySnapshot::get(L, 1);
	MemorySnapshot& snapshot2 = LuaMemorySnapshot::get(L, 2);
	const char* diffFile = luaL_checkstring(L, 3);
	MemorySnapshot diff(snapshot1, snapshot2);
	diff.writeToFile(diffFile);
	return 0;
}

MemorySnapshot::MemorySnapshot(lua_State* L)
{
	m_state = L;
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	MarkSource registryMarkSource("[registry]", MarkSourceType::REGISTRY);
	markObject(-1, registryMarkSource);
	lua_pop(L, 1);
	m_state = nullptr;
}

MemorySnapshot::MemorySnapshot(const MemorySnapshot& first, const MemorySnapshot& second) :
	m_state(nullptr)
{
	for (const std::pair<const void*, ObjectDescription>& markedObject : second.m_markedMap)
	{
		MarkedMap::const_iterator it = first.m_markedMap.find(markedObject.first);
		if (it == first.m_markedMap.end())
		{
			m_markedMap[markedObject.first] = markedObject.second;
		}
	}
}

void MemorySnapshot::writeToFile(const std::string& fileName) const
{
	std::fstream f(fileName, std::ios_base::out);

	for (const std::pair<const void*, ObjectDescription>& markedObject : m_markedMap)
	{
		f << "=========" << std::endl;
		f << markedObject.second.value << std::endl;
		for (const MarkSource& markSource : markedObject.second.sources)
		{
			f << '\t' << markSource.description << std::endl;
		}
	}
}

bool MemorySnapshot::isMarked(int index) const
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	const void* pointer = lua_topointer(m_state, index);
	return m_markedMap.find(pointer) != m_markedMap.end();
}

bool MemorySnapshot::markPointer(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	const void* pointer = lua_topointer(m_state, index);
	markSource.object = pointer;
	markSource.objectType = lua_type(m_state, index);
	MarkedMap::iterator it = m_markedMap.find(pointer);
	if (it != m_markedMap.end())
	{
		it->second.sources.push_back(markSource);
		return true;
	}
	else
	{
		ObjectDescription& description = m_markedMap[pointer];
		description.value = luaL_tolstring(m_state, index, nullptr);
		lua_pop(m_state, 1);
		if (luaL_getmetafield(m_state, index, "__name") == LUA_TSTRING)
		{
			description.value = std::string(lua_tostring(m_state, -1)) + ": " + description.value;
			lua_pop(m_state, 1);
		}
		description.sources.push_back(markSource);
		return false;
	}
}

void MemorySnapshot::markObject(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	switch (lua_type(m_state, index))
	{
	case LUA_TFUNCTION:
		markFunction(index, markSource);
		break;
	case LUA_TTHREAD:
		markThread(index, markSource);
		break;
	case LUA_TTABLE:
		markTable(index, markSource);
		break;
	case LUA_TLIGHTUSERDATA:
		markLightUserData(index, markSource);
		break;
	case LUA_TUSERDATA:
		markUserData(index, markSource);
		break;
	}
}

void MemorySnapshot::markFunction(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	if (!markPointer(index, markSource))
	{
		//std::cout << "FUNCTION" << std::endl;
	}
}

void MemorySnapshot::markThread(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	if (!markPointer(index, markSource))
	{
		//std::cout << "THREAD" << std::endl;
	}
}

void MemorySnapshot::markTable(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	if (!markPointer(index, markSource))
	{
		bool weakKeys = false;
		bool weakValues = false;
		{
			FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
			if (lua_getmetatable(m_state, index))
			{
				MarkSource metatableMarkSource(markSource, "[metatable]", MarkSourceType::METATABLE);
				markTable(-1, metatableMarkSource);

				hasWeakKeysAndValues(-1, weakKeys, weakValues);

				lua_pop(m_state, 1);
			}
		}

		{
			FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
			const int tableIndex = lua_absindex(m_state, index);
			lua_pushnil(m_state);
			while (lua_next(m_state, tableIndex) != 0)
			{
				FLAT_LUA_EXPECT_STACK_GROWTH(m_state, -1);

				if (!weakKeys)
				{
					FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
					MarkSource tableKeyMarkSource(markSource, "[key]", MarkSourceType::TABLEKEY);
					markObject(-2, tableKeyMarkSource);
				}

				if (!weakValues)
				{
					FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
					size_t length = 0;
					std::string key = luaL_tolstring(m_state, -2, nullptr);
					lua_pop(m_state, 1);
					MarkSource tableValueMarkSource(markSource, key, MarkSourceType::TABLEVALUE);
					markObject(-1, tableValueMarkSource);
				}

				lua_pop(m_state, 1);
			}
		}
	}
}

void MemorySnapshot::markLightUserData(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	markPointer(index, markSource);
}

void MemorySnapshot::markUserData(int index, MarkSource markSource)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	if (!markPointer(index, markSource))
	{
		if (lua_getmetatable(m_state, index))
		{
			lua_getfield(m_state, -1, "__name");
			std::string description = "[userdata metatable";
			if (!lua_isnil(m_state, -1))
			{
				description += " ";
				description += lua_tostring(m_state, -1);
			}
			description += "]";
			lua_pop(m_state, 1);
			MarkSource metatableMarkSource(markSource, description, MarkSourceType::METATABLE);
			markTable(-1, metatableMarkSource);
			lua_pop(m_state, 1);
		}

		lua_getuservalue(m_state, index);
		if (!lua_isnil(m_state, -1))
		{
			MarkSource uservalueMarkSource(markSource, "[uservalue]", MarkSourceType::USERVALUE);
			markObject(-1, uservalueMarkSource);
		}
		lua_pop(m_state, 1);
	}
}

void MemorySnapshot::hasWeakKeysAndValues(int index, bool& weakKeys, bool& weakValues) const
{
	FLAT_LUA_EXPECT_STACK_GROWTH(m_state, 0);
	int metatableIndex = lua_absindex(m_state, index);
	lua_pushliteral(m_state, "__mode");
	lua_rawget(m_state, metatableIndex);
	weakKeys = false;
	weakValues = false;
	if (lua_isstring(m_state, -1))
	{
		const char *mode = lua_tostring(m_state, -1);
		if (strchr(mode, 'k'))
		{
			weakKeys = true;
		}
		if (strchr(mode, 'v'))
		{
			weakValues = true;
		}
	}
	lua_pop(m_state, 1);
}

} // snapshot
} // lua
} // flat



// from https://github.com/cloudwu/lua-snapshot

static void mark_object(lua_State *L, lua_State *dL, const void * parent, const char * desc);

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TABLE 1
#define FUNCTION 2
#define SOURCE 3
#define THREAD 4
#define USERDATA 5
#define MARK 6

static bool
ismarked(lua_State *dL, const void *p) {
	lua_rawgetp(dL, MARK, p);
	if (lua_isnil(dL, -1)) {
		lua_pop(dL, 1);
		lua_pushboolean(dL, 1);
		lua_rawsetp(dL, MARK, p);
		return false;
	}
	lua_pop(dL, 1);
	return true;
}

static const void *
readobject(lua_State *L, lua_State *dL, const void *parent, const char *desc) {
	int t = lua_type(L, -1);
	int tidx = 0;
	switch (t) {
	case LUA_TTABLE:
		tidx = TABLE;
		break;
	case LUA_TFUNCTION:
		tidx = FUNCTION;
		break;
	case LUA_TTHREAD:
		tidx = THREAD;
		break;
	case LUA_TUSERDATA:
		tidx = USERDATA;
		break;
	default:
		return NULL;
	}

	const void * p = lua_topointer(L, -1);
	if (ismarked(dL, p)) {
		lua_rawgetp(dL, tidx, p);
		if (!lua_isnil(dL, -1)) {
			lua_pushstring(dL, desc);
			lua_rawsetp(dL, -2, parent);
		}
		lua_pop(dL, 1);
		lua_pop(L, 1);
		return NULL;
	}

	lua_newtable(dL);
	lua_pushstring(dL, desc);
	lua_rawsetp(dL, -2, parent);
	lua_rawsetp(dL, tidx, p);

	return p;
}

static const char *
keystring(lua_State *L, int index, char * buffer) {
	int t = lua_type(L, index);
	switch (t) {
	case LUA_TSTRING:
		return lua_tostring(L, index);
	case LUA_TNUMBER:
		sprintf_s(buffer, 32, "[%lg]", lua_tonumber(L, index));
		break;
	case LUA_TBOOLEAN:
		sprintf_s(buffer, 32, "[%s]", lua_toboolean(L, index) ? "true" : "false");
		break;
	case LUA_TNIL:
		sprintf_s(buffer, 32, "[nil]");
		break;
	default:
		sprintf_s(buffer, 32, "[%s:%p]", lua_typename(L, t), lua_tostring(L, index));
		break;
	}
	return buffer;
}

static void
mark_table(lua_State *L, lua_State *dL, const void * parent, const char * desc) {
	const void * t = readobject(L, dL, parent, desc);
	if (t == NULL)
		return;

	bool weakk = false;
	bool weakv = false;
	if (lua_getmetatable(L, -1)) {
		lua_pushliteral(L, "__mode");
		lua_rawget(L, -2);
		if (lua_isstring(L, -1)) {
			const char *mode = lua_tostring(L, -1);
			if (strchr(mode, 'k')) {
				weakk = true;
			}
			if (strchr(mode, 'v')) {
				weakv = true;
			}
		}
		lua_pop(L, 1);

		luaL_checkstack(L, LUA_MINSTACK, NULL);
		mark_table(L, dL, t, "[metatable]");
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (weakv) {
			lua_pop(L, 1);
		}
		else {
			char temp[32];
			const char * desc = keystring(L, -2, temp);
			mark_object(L, dL, t, desc);
		}
		if (!weakk) {
			lua_pushvalue(L, -1);
			mark_object(L, dL, t, "[key]");
		}
	}

	lua_pop(L, 1);
}

static void
mark_userdata(lua_State *L, lua_State *dL, const void * parent, const char *desc) {
	const void * t = readobject(L, dL, parent, desc);
	if (t == NULL)
		return;
	if (lua_getmetatable(L, -1)) {
		mark_table(L, dL, t, "[metatable]");
	}

	lua_getuservalue(L, -1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 2);
	}
	else {
		mark_table(L, dL, t, "[uservalue]");
		lua_pop(L, 1);
	}
}

static void
mark_function(lua_State *L, lua_State *dL, const void * parent, const char *desc) {
	const void * t = readobject(L, dL, parent, desc);
	if (t == NULL)
		return;

	int i;
	for (i = 1;; i++) {
		const char *name = lua_getupvalue(L, -1, i);
		if (name == NULL)
			break;
		mark_object(L, dL, t, name[0] ? name : "[upvalue]");
	}
	if (lua_iscfunction(L, -1)) {
		if (i == 1) {
			// light c function
			lua_pushnil(dL);
			lua_rawsetp(dL, FUNCTION, t);
		}
		lua_pop(L, 1);
	}
	else {
		lua_Debug ar;
		lua_getinfo(L, ">S", &ar);
		luaL_Buffer b;
		luaL_buffinit(dL, &b);
		luaL_addstring(&b, ar.short_src);
		char tmp[16];
		sprintf_s(tmp, 16, ":%d", ar.linedefined);
		luaL_addstring(&b, tmp);
		luaL_pushresult(&b);
		lua_rawsetp(dL, SOURCE, t);
	}
}

static void
mark_thread(lua_State *L, lua_State *dL, const void * parent, const char *desc) {
	const void * t = readobject(L, dL, parent, desc);
	if (t == NULL)
		return;
	int level = 0;
	lua_State *cL = lua_tothread(L, -1);
	if (cL == L) {
		level = 1;
	}
	else {
		// mark stack
		int top = lua_gettop(cL);
		luaL_checkstack(cL, 1, NULL);
		int i;
		char tmp[16];
		for (i = 0; i<top; i++) {
			lua_pushvalue(cL, i + 1);
			sprintf_s(tmp, 16, "[%d]", i + 1);
			mark_object(cL, dL, cL, tmp);
		}
	}
	lua_Debug ar;
	luaL_Buffer b;
	luaL_buffinit(dL, &b);
	while (lua_getstack(cL, level, &ar)) {
		char tmp[128];
		lua_getinfo(cL, "Sl", &ar);
		luaL_addstring(&b, ar.short_src);
		if (ar.currentline >= 0) {
			char tmp[16];
			sprintf_s(tmp, 16, ":%d ", ar.currentline);
			luaL_addstring(&b, tmp);
		}

		int i, j;
		for (j = 1; j>-1; j -= 2) {
			for (i = j;; i += j) {
				const char * name = lua_getlocal(cL, &ar, i);
				if (name == NULL)
					break;
				snprintf(tmp, sizeof(tmp), "%s : %s:%d", name, ar.short_src, ar.currentline);
				mark_object(cL, dL, t, tmp);
			}
		}

		++level;
	}
	luaL_pushresult(&b);
	lua_rawsetp(dL, SOURCE, t);
	lua_pop(L, 1);
}

static void
mark_object(lua_State *L, lua_State *dL, const void * parent, const char *desc) {
	luaL_checkstack(L, LUA_MINSTACK, NULL);
	int t = lua_type(L, -1);
	switch (t) {
	case LUA_TTABLE:
		mark_table(L, dL, parent, desc);
		break;
	case LUA_TUSERDATA:
		mark_userdata(L, dL, parent, desc);
		break;
	case LUA_TFUNCTION:
		mark_function(L, dL, parent, desc);
		break;
	case LUA_TTHREAD:
		mark_thread(L, dL, parent, desc);
		break;
	default:
		lua_pop(L, 1);
		break;
	}
}

static int
count_table(lua_State *L, int idx) {
	int n = 0;
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) {
		++n;
		lua_pop(L, 1);
	}
	return n;
}

static void
gen_table_desc(lua_State *dL, luaL_Buffer *b, const void * parent, const char *desc) {
	char tmp[32];
	size_t l = sprintf_s(tmp, 32, "%p : ", parent);
	luaL_addlstring(b, tmp, l);
	luaL_addstring(b, desc);
	luaL_addchar(b, '\n');
}

static void
pdesc(lua_State *L, lua_State *dL, int idx, const char * typename_) {
	lua_pushnil(dL);
	while (lua_next(dL, idx) != 0) {
		luaL_Buffer b;
		luaL_buffinit(L, &b);
		const void * key = lua_touserdata(dL, -2);
		if (idx == FUNCTION) {
			lua_rawgetp(dL, SOURCE, key);
			if (lua_isnil(dL, -1)) {
				luaL_addstring(&b, "cfunction\n");
			}
			else {
				size_t l = 0;
				const char * s = lua_tolstring(dL, -1, &l);
				luaL_addlstring(&b, s, l);
				luaL_addchar(&b, '\n');
			}
			lua_pop(dL, 1);
		}
		else if (idx == THREAD) {
			lua_rawgetp(dL, SOURCE, key);
			size_t l = 0;
			const char * s = lua_tolstring(dL, -1, &l);
			luaL_addlstring(&b, s, l);
			luaL_addchar(&b, '\n');
			lua_pop(dL, 1);
		}
		else {
			luaL_addstring(&b, typename_);
			luaL_addchar(&b, '\n');
		}
		lua_pushnil(dL);
		while (lua_next(dL, -2) != 0) {
			const void * parent = lua_touserdata(dL, -2);
			const char * desc = luaL_checkstring(dL, -1);
			gen_table_desc(dL, &b, parent, desc);
			lua_pop(dL, 1);
		}
		luaL_pushresult(&b);
		lua_rawsetp(L, -2, key);
		lua_pop(dL, 1);
	}
}

static void
gen_result(lua_State *L, lua_State *dL) {
	int count = 0;
	count += count_table(dL, TABLE);
	count += count_table(dL, FUNCTION);
	count += count_table(dL, USERDATA);
	count += count_table(dL, THREAD);
	lua_createtable(L, 0, count);
	pdesc(L, dL, TABLE, "table");
	pdesc(L, dL, USERDATA, "userdata");
	pdesc(L, dL, FUNCTION, "function");
	pdesc(L, dL, THREAD, "thread");
}

static int
l_snapshot(lua_State *L) {
	int i;
	lua_State *dL = luaL_newstate();
	for (i = 0; i<MARK; i++) {
		lua_newtable(dL);
	}
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	mark_table(L, dL, NULL, "[registry]");
	gen_result(L, dL);
	lua_close(dL);
	return 1;
}
