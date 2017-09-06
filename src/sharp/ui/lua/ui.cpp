#include "ui.h"
#include "../../../flat/game.h"
#include "../../../lua/lua.h"
#include "../../../lua/sharedluareference.h"
#include "../../../lua/sharedcppreference.h"
#include "../../../lua/table.h"
#include "../canvaswidget.h"
#include "../focusablewidget.h"
#include "../textinputwidget.h"
#include "../textwidget.h"
#include "../widget.h"
#include "../widgetfactory.h"

namespace flat
{
namespace sharp
{
namespace ui
{
namespace lua
{

using LuaWidget = flat::lua::SharedCppReference<Widget>;
using LuaWidgetFactory = flat::lua::SharedCppReference<WidgetFactory>;

static char widgetFactoryRegistryIndex = 'W';

int open(lua_State* L)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	
	// Widget metatable
	static const luaL_Reg Widget_lib_m[] = {
		{"addChild",              l_Widget_addChild},
		{"removeChild",           l_Widget_removeChild},
		{"removeFromParent",      l_Widget_removeFromParent},
		{"removeAllChildren",     l_Widget_removeAllChildren},
		
		{"setSizePolicy",         l_Widget_setSizePolicy},
		{"setSizePolicyX",        l_Widget_setSizePolicyX},
		{"setSizePolicyY",        l_Widget_setSizePolicyY},
		{"getSizePolicy",         l_Widget_getSizePolicy},
		{"setSize",               l_Widget_setSize},
		{"getSize",               l_Widget_getSize},
		
		{"setPositionPolicy",     l_Widget_setPositionPolicy},
		{"getPositionPolicy",     l_Widget_getPositionPolicy},
		{"setPosition",           l_Widget_setPosition},
		{"getPosition",           l_Widget_getPosition},
		{"getRelativePosition",   l_Widget_getRelativePosition},
		
		{"setRotation",           l_Widget_setRotation},
		{"setRotationZ",          l_Widget_setRotationZ},
		{"getRotation",           l_Widget_getRotation},

		{"setMargin",             l_Widget_setMargin},
		{"getMargin",             l_Widget_getMargin},

		{"setPadding",            l_Widget_setPadding},
		{"getPadding",            l_Widget_getPadding},
		
		{"setBackground",         l_Widget_setBackground},
		{"setBackgroundRepeat",   l_Widget_setBackgroundRepeat},
		{"setBackgroundColor",    l_Widget_setBackgroundColor},
		{"setBackgroundPosition", l_Widget_setBackgroundPosition},
		
		{"setVisible",            l_Widget_setVisible},
		{"getVisible",            l_Widget_getVisible},
		{"hide",                  l_Widget_hide},
		{"show",                  l_Widget_show},

		{"setAllowScrollX",       l_Widget_setAllowScrollX},
		{"getAllowScrollX",       l_Widget_getAllowScrollX},
		{"setAllowScrollY",       l_Widget_setAllowScrollY},
		{"getAllowScrollY",       l_Widget_getAllowScrollY},
		{"setAllowScroll",        l_Widget_setAllowScroll},
		{"getAllowScroll",        l_Widget_getAllowScroll},

		{"setRestrictScrollX",    l_Widget_setRestrictScrollX},
		{"getRestrictScrollX",    l_Widget_getRestrictScrollX},
		{"setRestrictScrollY",    l_Widget_setRestrictScrollY},
		{"getRestrictScrollY",    l_Widget_getRestrictScrollY},
		{"setRestrictScroll",     l_Widget_setRestrictScroll},
		{"getRestrictScroll",     l_Widget_getRestrictScroll},
		
		{"click",                 l_Widget_click},
		{"mouseDown",             l_Widget_mouseDown},
		{"mouseUp",               l_Widget_mouseUp},
		{"mouseMove",             l_Widget_mouseMove},
		{"mouseEnter",            l_Widget_mouseEnter},
		{"mouseLeave",            l_Widget_mouseLeave},
		{"scroll",                l_Widget_scroll},
		{"dragged",               l_Widget_dragged},

		{"drag",                  l_Widget_drag},
		{"drop",                  l_Widget_drop},
		
		{"setText",               l_TextWidget_setText},
		{"getText",               l_TextWidget_getText},
		{"setTextColor",          l_TextWidget_setTextColor},

		{"focus",                 l_FocusableWidget_focus},
		{"blur",                  l_FocusableWidget_blur},

		{"draw",                  l_CanvasWidget_draw},
		{"redraw",                l_CanvasWidget_redraw},
		{"clear",                 l_CanvasWidget_clear},
		{"drawLine",              l_CanvasWidget_drawLine},
		{"drawBezier",            l_CanvasWidget_drawBezier},
		
		{nullptr, nullptr}
	};
	
	LuaWidget::registerClass("flat.Widget", L, Widget_lib_m);
	
	// Widget static methods
	static const luaL_Reg Widget_lib_s[] = {
		{"getRoot",        l_Widget_getRoot},
		
		{"makeImage",      l_Widget_makeImage},
		{"makeFixedSize",  l_Widget_makeFixedSize},
		{"makeExpand",     l_Widget_makeExpand},
		{"makeLineFlow",   l_Widget_makeLineFlow},
		{"makeColumnFlow", l_Widget_makeColumnFlow},
		{"makeText",       l_Widget_makeText},
		{"makeTextInput",  l_Widget_makeTextInput},
		{"makeCanvas",     l_Widget_makeCanvas},
		
		{nullptr, nullptr}
	};
	
	luaL_newlib(L, Widget_lib_s);
	
	lua_pushstring(L, "PositionPolicy");
	static const flat::lua::table::KeyValuePair<int> positionPolicyTable[] = {
		{"LEFT",         Widget::PositionPolicy::LEFT},
		{"CENTER_X",     Widget::PositionPolicy::CENTER_X},
		{"RIGHT",        Widget::PositionPolicy::RIGHT},
		{"FLOW_X",       Widget::PositionPolicy::FLOW_X},
		
		{"TOP",          Widget::PositionPolicy::TOP},
		{"CENTER_Y",     Widget::PositionPolicy::CENTER_Y},
		{"BOTTOM",       Widget::PositionPolicy::BOTTOM},
		{"FLOW_Y",       Widget::PositionPolicy::FLOW_Y},
		
		{"TOP_LEFT",     Widget::PositionPolicy::TOP_LEFT},
		{"TOP_RIGHT",    Widget::PositionPolicy::TOP_RIGHT},
		{"BOTTOM_LEFT",  Widget::PositionPolicy::BOTTOM_LEFT},
		{"BOTTOM_RIGHT", Widget::PositionPolicy::BOTTOM_RIGHT},
		{"CENTER",       Widget::PositionPolicy::CENTER},
		{"COLUMN_FLOW",  Widget::PositionPolicy::COLUMN_FLOW},
		{"LINE_FLOW",    Widget::PositionPolicy::LINE_FLOW},
		
		{nullptr, 0},
	};
	flat::lua::table::pushTable(L, positionPolicyTable);
	lua_settable(L, -3);

	lua_pushstring(L, "SizePolicy");
	static const flat::lua::table::KeyValuePair<int> sizePolicyTable[] = {
		{"COMPRESS",   Widget::SizePolicy::COMPRESS},
		{"COMPRESS_X", Widget::SizePolicy::COMPRESS_X},
		{"COMPRESS_Y", Widget::SizePolicy::COMPRESS_Y},

		{"EXPAND",     Widget::SizePolicy::EXPAND},
		{"EXPAND_X",   Widget::SizePolicy::EXPAND_X},
		{"EXPAND_Y",   Widget::SizePolicy::EXPAND_Y},

		{"FIXED",      Widget::SizePolicy::FIXED},
		{"FIXED_X",    Widget::SizePolicy::FIXED_X},
		{"FIXED_Y",    Widget::SizePolicy::FIXED_Y},

		{nullptr, 0}
	};
	flat::lua::table::pushTable(L, sizePolicyTable);
	lua_settable(L, -3);

	lua_pushstring(L, "BackgroundRepeat");
	static const flat::lua::table::KeyValuePair<int> backgroundRepeatTable[] = {
		{"SCALED", Widget::BackgroundRepeat::SCALED},
		{"REPEAT", Widget::BackgroundRepeat::REPEAT},

		{ nullptr, 0 },
	};
	flat::lua::table::pushTable(L, backgroundRepeatTable);
	lua_settable(L, -3);
	
	lua_setglobal(L, "Widget");
	

	LuaWidgetFactory::registerClass("flat.WidgetFactory", L);
	Game& game = flat::lua::getGame(L);
	LuaWidgetFactory::pushNew(L, new WidgetFactory(game));
	lua_rawsetp(L, LUA_REGISTRYINDEX, &widgetFactoryRegistryIndex);
	
	return 0;
}

int close(lua_State* L)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);

	lua_pushnil(L);
	lua_rawsetp(L, LUA_REGISTRYINDEX, &widgetFactoryRegistryIndex);

	return 0;
}

// Widget methods

int l_Widget_addChild(lua_State* L)
{
	Widget& parent = getWidget(L, 1);
	Widget& child = getWidget(L, 2);
	parent.addChild(child.getSharedPtr());
	return 0;
}

int l_Widget_removeChild(lua_State* L)
{
	Widget& parent = getWidget(L, 1);
	Widget& child = getWidget(L, 2);
	parent.removeChild(child.getSharedPtr());
	return 0;
}

int l_Widget_removeFromParent(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	widget.removeFromParent();
	return 0;
}

int l_Widget_removeAllChildren(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	widget.removeAllChildren();
	return 0;
}

int l_Widget_setSizePolicy(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::SizePolicy sizePolicy = static_cast<Widget::SizePolicy>(luaL_checkinteger(L, 2));
	widget.setSizePolicy(sizePolicy);
	return 0;
}

int l_Widget_setSizePolicyX(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::SizePolicy sizePolicyX = static_cast<Widget::SizePolicy>(luaL_checkinteger(L, 2));
	widget.setSizePolicyX(sizePolicyX);
	return 0;
}

int l_Widget_setSizePolicyY(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::SizePolicy sizePolicyY = static_cast<Widget::SizePolicy>(luaL_checkinteger(L, 2));
	widget.setSizePolicyY(sizePolicyY);
	return 0;
}

int l_Widget_getSizePolicy(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::SizePolicy sizePolicy = widget.getSizePolicy();
	lua_pushinteger(L, sizePolicy);
	return 1;
}

int l_Widget_setSize(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	float width = static_cast<float>(luaL_checknumber(L, 2));
	float height = static_cast<float>(luaL_checknumber(L, 3));
	Widget::Size size(width, height);
	widget.setSize(size);
	return 0;
}

int l_Widget_getSize(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const Widget::Size& size = widget.getSize();
	lua_pushnumber(L, size.x);
	lua_pushnumber(L, size.y);
	return 2;
}

int l_Widget_setPositionPolicy(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::PositionPolicy positionPolicy = static_cast<Widget::PositionPolicy>(luaL_checkinteger(L, 2));
	widget.setPositionPolicy(positionPolicy);
	return 0;
}

int l_Widget_getPositionPolicy(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::PositionPolicy positionPolicy = widget.getPositionPolicy();
	lua_pushinteger(L, positionPolicy);
	return 1;
}

int l_Widget_setPosition(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	float x = static_cast<float>(luaL_checknumber(L, 2));
	float y = static_cast<float>(luaL_checknumber(L, 3));
	Widget::Position position(x, y);
	widget.setPosition(position);
	return 0;
}

int l_Widget_getPosition(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const Widget::Position& position = widget.getPosition();
	lua_pushnumber(L, position.x);
	lua_pushnumber(L, position.y);
	return 2;
}

int l_Widget_getRelativePosition(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget& other = getWidget(L, 2);
	Vector2 position = widget.getRelativePosition(other);
	lua_pushnumber(L, position.x);
	lua_pushnumber(L, position.y);
	return 2;
}

int l_Widget_setRotation(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	float rotationX = static_cast<float>(luaL_checknumber(L, 2));
	float rotationY = static_cast<float>(luaL_checknumber(L, 3));
	float rotationZ = static_cast<float>(luaL_checknumber(L, 4));
	widget.setRotation(Widget::Rotation(rotationX, rotationY, rotationZ));
	return 0;
}

int l_Widget_setRotationZ(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	float rotationZ = static_cast<float>(luaL_checknumber(L, 2));
	widget.setRotationZ(rotationZ);
	return 0;
}

int l_Widget_getRotation(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const Widget::Rotation& rotation = widget.getRotation();
	lua_pushnumber(L, rotation.x);
	lua_pushnumber(L, rotation.y);
	lua_pushnumber(L, rotation.z);
	return 3;
}

int l_Widget_setMargin(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	int top = lua_gettop(L);
	if (top != 2 && top != 5)
	{
		luaL_error(L, "setMargin must have 1 or 4 parameters, %d given", top - 1);
	}
	float marginTop = static_cast<float>(luaL_checknumber(L, 2));
	Widget::Margin margin(marginTop);
	if (top == 5)
	{
		margin.right  = static_cast<float>(luaL_checknumber(L, 3));
		margin.bottom = static_cast<float>(luaL_checknumber(L, 4));
		margin.left   = static_cast<float>(luaL_checknumber(L, 5));
	}
	widget.setMargin(margin);
	return 0;
}

int l_Widget_getMargin(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const Widget::Margin& margin = widget.getMargin();
	lua_pushnumber(L, margin.top);
	lua_pushnumber(L, margin.right);
	lua_pushnumber(L, margin.bottom);
	lua_pushnumber(L, margin.left);
	return 4;
}

int l_Widget_setPadding(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	int top = lua_gettop(L);
	if (top != 2 && top != 5)
	{
		luaL_error(L, "setPadding must have 1 or 4 parameters, %d given", top - 1);
	}
	float paddingTop = static_cast<float>(luaL_checknumber(L, 2));
	Widget::Padding padding(paddingTop);
	if (top == 5)
	{
		padding.right = static_cast<float>(luaL_checknumber(L, 3));
		padding.bottom = static_cast<float>(luaL_checknumber(L, 4));
		padding.left = static_cast<float>(luaL_checknumber(L, 5));
	}
	widget.setPadding(padding);
	return 0;
}

int l_Widget_getPadding(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const Widget::Padding& padding = widget.getPadding();
	lua_pushnumber(L, padding.top);
	lua_pushnumber(L, padding.right);
	lua_pushnumber(L, padding.bottom);
	lua_pushnumber(L, padding.left);
	return 4;
}

int l_Widget_setBackground(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	const char* backgroundFileName = luaL_checkstring(L, 2);
	Game& game = flat::lua::getGame(L);
	std::shared_ptr<const flat::video::FileTexture> background = game.video->getTexture(backgroundFileName);
	widget.setBackground(background);
	return 0;
}

int l_Widget_setBackgroundRepeat(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	Widget::BackgroundRepeat backgroundRepeat = static_cast<Widget::BackgroundRepeat>(luaL_checkinteger(L, 2));
	widget.setBackgroundRepeat(backgroundRepeat);
	return 0;
}

int l_Widget_setBackgroundColor(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	uint32_t color = static_cast<uint32_t>(luaL_checkinteger(L, 2));
	flat::video::Color backgroundColor(color);
	widget.setBackgroundColor(backgroundColor);
	return 0;
}

int l_Widget_setBackgroundPosition(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	float x = static_cast<float>(luaL_checknumber(L, 2));
	float y = static_cast<float>(luaL_checknumber(L, 3));
	widget.setBackgroundPosition(Vector2(x, y));
	return 0;
}

int l_Widget_setVisible(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool visible = lua_toboolean(L, 2) == 1;
	widget.setVisible(visible);
	return 0;
}

int l_Widget_getVisible(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool visible = widget.getVisible();
	lua_pushboolean(L, visible);
	return 1;
}

int l_Widget_hide(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	widget.hide();
	return 0;
}

int l_Widget_show(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	widget.show();
	return 0;
}

int l_Widget_setAllowScrollX(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool allowScrollX = lua_toboolean(L, 2) == 1;
	widget.setAllowScrollX(allowScrollX);
	return 0;
}

int l_Widget_getAllowScrollX(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getAllowScrollX());
	return 1;
}

int l_Widget_setAllowScrollY(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool allowScrollY = lua_toboolean(L, 2) == 1;
	widget.setAllowScrollY(allowScrollY);
	return 0;
}

int l_Widget_getAllowScrollY(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getAllowScrollY());
	return 1;
}

int l_Widget_setAllowScroll(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool allowScrollX = lua_toboolean(L, 2) == 1;
	bool allowScrollY = lua_toboolean(L, 3) == 1;
	widget.setAllowScrollX(allowScrollX);
	widget.setAllowScrollY(allowScrollY);
	return 0;
}

int l_Widget_getAllowScroll(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getAllowScrollX());
	lua_pushboolean(L, widget.getAllowScrollY());
	return 2;
}

int l_Widget_setRestrictScrollX(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool restrictScrollX = lua_toboolean(L, 2) == 1;
	widget.setRestrictScrollX(restrictScrollX);
	return 0;
}

int l_Widget_getRestrictScrollX(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getRestrictScrollX());
	return 1;
}

int l_Widget_setRestrictScrollY(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool restrictScrollY = lua_toboolean(L, 2) == 1;
	widget.setRestrictScrollY(restrictScrollY);
	return 0;
}

int l_Widget_getRestrictScrollY(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getRestrictScrollY());
	return 1;
}

int l_Widget_setRestrictScroll(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	bool restrictScrollX = lua_toboolean(L, 2) == 1;
	bool restrictScrollY = lua_toboolean(L, 3) == 1;
	widget.setRestrictScrollX(restrictScrollX);
	widget.setRestrictScrollY(restrictScrollY);
	return 0;
}

int l_Widget_getRestrictScroll(lua_State* L)
{
	Widget& widget = getWidget(L, 1);
	lua_pushboolean(L, widget.getRestrictScrollX());
	lua_pushboolean(L, widget.getRestrictScrollY());
	return 2;
}

int l_Widget_click(lua_State* L)
{
	return addPropagatedMouseWidgetCallback<Widget>(L, &Widget::click);
}

int l_Widget_mouseDown(lua_State * L)
{
	return addPropagatedMouseWidgetCallback<Widget>(L, &Widget::mouseDown);
}

int l_Widget_mouseUp(lua_State * L)
{
	return addPropagatedMouseWidgetCallback<Widget>(L, &Widget::mouseUp);
}

int l_Widget_mouseMove(lua_State* L)
{
	return addPropagatedMouseWidgetCallback<Widget>(L, &Widget::mouseMove);
}

int l_Widget_mouseEnter(lua_State* L)
{
	return addWidgetCallback<Widget>(L, &Widget::mouseEnter);
}

int l_Widget_mouseLeave(lua_State* L)
{
	return addWidgetCallback<Widget>(L, &Widget::mouseLeave);
}

int l_Widget_scroll(lua_State* L)
{
	return addWidgetCallback<Widget>(L, &Widget::scroll);
}

int l_Widget_dragged(lua_State* L)
{
	return addWidgetCallback<Widget>(L, &Widget::dragged);
}

int l_Widget_drag(lua_State * L)
{
	Widget& widget = getWidget(L, 1);
	widget.drag();
	return 0;
}

int l_Widget_drop(lua_State * L)
{
	Widget& widget = getWidget(L, 1);
	widget.drop();
	return 0;
}

int l_TextWidget_setText(lua_State* L)
{
	TextWidget& textWidget = getTextWidget(L, 1);
	const char* text = luaL_checkstring(L, 2);
	textWidget.setText(text);
	return 0;
}

int l_TextWidget_getText(lua_State* L)
{
	TextWidget& textWidget = getTextWidget(L, 1);
	lua_pushstring(L, textWidget.getText().c_str());
	return 1;
}

int l_TextWidget_setTextColor(lua_State* L)
{
	TextWidget& textWidget = getTextWidget(L, 1);
	uint32_t color = static_cast<uint32_t>(luaL_checkinteger(L, 2));
	flat::video::Color textColor(color);
	textWidget.setTextColor(textColor);
	return 0;
}

int l_FocusableWidget_focus(lua_State* L)
{
	return addWidgetCallback<FocusableWidget>(L, &FocusableWidget::enterFocus);
}

int l_FocusableWidget_blur(lua_State* L)
{
	return addWidgetCallback<FocusableWidget>(L, &FocusableWidget::leaveFocus);
}

int l_CanvasWidget_draw(lua_State * L)
{
	return addWidgetCallback<CanvasWidget>(L, &Widget::layoutFinished);
}

int l_CanvasWidget_redraw(lua_State * L)
{
	CanvasWidget& canvasWidget = getCanvasWidget(L, 1);
	canvasWidget.layoutFinished(&canvasWidget);
	return 0;
}

int l_CanvasWidget_clear(lua_State * L)
{
	CanvasWidget& canvasWidget = getCanvasWidget(L, 1);
	uint32_t color = static_cast<uint32_t>(luaL_checkinteger(L, 2));
	flat::video::Color clearColor(color);
	canvasWidget.clear(clearColor);
	return 0;
}

int l_CanvasWidget_drawLine(lua_State* L)
{
	CanvasWidget& canvasWidget = getCanvasWidget(L, 1);
	uint32_t color = static_cast<uint32_t>(luaL_checkinteger(L, 2));
	float width = static_cast<float>(luaL_checknumber(L, 3));
	flat::Vector2 from = flat::lua::getVector2(L, 4);
	flat::Vector2 to = flat::lua::getVector2(L, 5);
	flat::video::Color lineColor(color);
	canvasWidget.drawLine(lineColor, width, from, to);
	return 0;
}

int l_CanvasWidget_drawBezier(lua_State * L)
{
	CanvasWidget& canvasWidget = getCanvasWidget(L, 1);
	uint32_t color = static_cast<uint32_t>(luaL_checkinteger(L, 2));
	float width = static_cast<float>(luaL_checknumber(L, 3));
	luaL_checktype(L, 4, LUA_TTABLE);
	size_t numControlPoints = lua_rawlen(L, 4);
	std::vector<Vector2> controlPoints;
	controlPoints.reserve(numControlPoints);
	for (int i = 1; i <= numControlPoints; ++i)
	{
		lua_rawgeti(L, 4, i);
		flat::Vector2 controlPoint = flat::lua::getVector2(L, -1);
		controlPoints.push_back(controlPoint);
		lua_pop(L, 1);
	}
	flat::video::Color lineColor(color);
	canvasWidget.drawBezier(lineColor, width, controlPoints);
	return 0;
}

// static Widget functions

int l_Widget_getRoot(lua_State* L)
{
	RootWidget* root = flat::lua::getGame(L).ui->root.get();
	FLAT_ASSERT(root != nullptr);
	pushWidget(L, root->getSharedPtr());
	return 1;
}

int l_Widget_makeImage(lua_State* L)
{
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::string fileName = luaL_checkstring(L, 1);
	std::shared_ptr<Widget> widget = widgetFactory.makeImage(fileName);
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeFixedSize(lua_State* L)
{
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	float width = static_cast<float>(luaL_checknumber(L, 1));
	float height = static_cast<float>(luaL_checknumber(L, 2));
	Widget::Size size(width, height);
	std::shared_ptr<Widget> widget = widgetFactory.makeFixedSize(size);
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeExpand(lua_State * L)
{
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeExpand();
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeLineFlow(lua_State* L)
{
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeLineFlow();
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeColumnFlow(lua_State* L)
{
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeColumnFlow();
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeText(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	int fontSize = static_cast<int>(luaL_checkinteger(L, 3));
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeText(text, fileName, fontSize);
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeTextInput(lua_State* L)
{
	const char* fileName = luaL_checkstring(L, 1);
	int fontSize = static_cast<int>(luaL_checkinteger(L, 2));
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeTextInput(fileName, fontSize);
	pushWidget(L, widget);
	return 1;
}

int l_Widget_makeCanvas(lua_State* L)
{
	int width = static_cast<int>(luaL_checkinteger(L, 1));
	int height = static_cast<int>(luaL_checkinteger(L, 2));
	flat::Vector2 size(width, height);
	WidgetFactory& widgetFactory = getWidgetFactory(L);
	std::shared_ptr<Widget> widget = widgetFactory.makeCanvas(size);
	pushWidget(L, widget);
	return 1;
}

// private

Widget& getWidget(lua_State* L, int index)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	return LuaWidget::get(L, index);
}

TextWidget& getTextWidget(lua_State* L, int index)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	Widget& widget = getWidget(L, index);
	TextWidget* textWidget = dynamic_cast<TextWidget*>(&widget);
	if (!textWidget)
	{
		luaL_error(L, "TextWidget required, Widget given");
	}
	return *textWidget;
}

FocusableWidget& getFocusableWidget(lua_State* L, int index)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	Widget& widget = getWidget(L, index);
	if (!widget.canBeFocused())
	{
		luaL_error(L, "FocusableWidget required, Widget given");
	}
	FocusableWidget* focusableWidget = dynamic_cast<FocusableWidget*>(&widget);
	FLAT_ASSERT(focusableWidget != nullptr);
	return *focusableWidget;
}

CanvasWidget& getCanvasWidget(lua_State * L, int index)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
	Widget& widget = getWidget(L, index);
	CanvasWidget* canvasWidget = dynamic_cast<CanvasWidget*>(&widget);
	if (!canvasWidget)
	{
		luaL_error(L, "CanvasWidget required, Widget given");
	}
	return *canvasWidget;
}

template <>
Widget& getWidgetOfType(lua_State* L, int index)
{
	return getWidget(L, index);
}

template <>
TextWidget& getWidgetOfType(lua_State* L, int index)
{
	return getTextWidget(L, index);
}

template <>
FocusableWidget& getWidgetOfType(lua_State* L, int index)
{
	return getFocusableWidget(L, index);
}

template <>
CanvasWidget& getWidgetOfType(lua_State* L, int index)
{
	return getCanvasWidget(L, index);
}

template <class T>
int addWidgetCallback(lua_State* L, Slot<Widget*> T::* slot)
{
	T& widget = getWidgetOfType<T>(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	FLAT_ASSERT(L == flat::lua::getMainThread(L));
	flat::lua::SharedLuaReference<LUA_TFUNCTION> callback(L, 2);
	(widget.*slot).on(
		[L, callback](Widget* w)
		{
			FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
			callback.push(L);
			luaL_checktype(L, -1, LUA_TFUNCTION);
			pushWidget(L, w->getSharedPtr());
			lua_call(L, 1, 0);
			return true;
		}
	);
	return 0;
}

template<class T>
int addPropagatedMouseWidgetCallback(lua_State* L, Slot<Widget*, bool&> T::* slot)
{
	T& widget = getWidgetOfType<T>(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	FLAT_ASSERT(L == flat::lua::getMainThread(L));
	flat::lua::SharedLuaReference<LUA_TFUNCTION> ref(L, 2);
	const auto& mouse = flat::lua::getGame(L).input->mouse;
	(widget.*slot).on(
		[L, ref, &mouse](Widget* w, bool& eventHandled)
		{
			FLAT_LUA_EXPECT_STACK_GROWTH(L, 0);
			Vector2 relativePosition = w->getRelativePosition(mouse->getPosition());
			ref.push(L);
			luaL_checktype(L, -1, LUA_TFUNCTION);
			pushWidget(L, w->getSharedPtr());
			lua_pushnumber(L, relativePosition.x);
			lua_pushnumber(L, relativePosition.y);
			lua_call(L, 3, 1);
			eventHandled = eventHandled || lua_toboolean(L, -1);
			lua_pop(L, 1);
			return true;
		}
	);
	return 0;
}

void pushWidget(lua_State* L, const std::shared_ptr<Widget>& widget)
{
	FLAT_LUA_EXPECT_STACK_GROWTH(L, 1);
	if (widget)
	{
		LuaWidget::pushNew(L, widget);
	}
	else
	{
		lua_pushnil(L);
	}
}

WidgetFactory& getWidgetFactory(lua_State* L)
{
	return flat::lua::getGame(L).ui->factory;
}

} // lua
} // ui
} // sharp
} // flat



