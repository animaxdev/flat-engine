#include <cctype>

#include "textinputwidget.h"

#include "../../flat.h"

namespace flat
{
namespace sharp
{
namespace ui
{

TextInputWidget::TextInputWidget(Flat& flat, const std::shared_ptr<const video::font::Font>& font) : TextWidget(font),
	m_flat(flat),
	m_cursorIndex(0),
	m_selectionIndex(0)
{
	enterFocus.on(this, &TextInputWidget::enteredFocus);
	leaveFocus.on(this, &TextInputWidget::leftFocus);
	mouseDown.on(this, &TextInputWidget::onMouseDown);
	mouseMove.on(this, &TextInputWidget::onMouseMove);
}

TextInputWidget::~TextInputWidget()
{
	enterFocus.off(this);
	leaveFocus.off(this);
	mouseDown.off(this);
}

void TextInputWidget::draw(const render::RenderSettings& renderSettings, const ScissorRectangle& parentScissor) const
{
	drawSelection(renderSettings, m_cursorIndex, m_selectionIndex);
	TextWidget::draw(renderSettings, parentScissor);
	drawCursor(renderSettings, m_cursorIndex);
}

CursorType TextInputWidget::getCursorType() const
{
	return CURSOR(IBEAM);
}

bool TextInputWidget::enteredFocus(Widget* widget)
{
	m_inputContext = std::make_unique<input::context::InputContext>(m_flat);
	m_inputContext->getKeyboardInputContext().setEnableKeyRepeat(true);
	m_flat.input->pushContext(m_inputContext);
	m_inputContext->getKeyboardInputContext().keyJustPressed.on(this, &TextInputWidget::keyJustPressed);
	m_inputContext->getKeyboardInputContext().textEdited.on(this, &TextInputWidget::textEdited);
	m_inputContext->getKeyboardInputContext().setEnableTextInput(true);
	return true;
}

bool TextInputWidget::leftFocus(Widget* widget)
{
	if(hasSelectedText())
	{
		unselect();
	}
	m_inputContext->getKeyboardInputContext().keyJustPressed.off(this);
	m_inputContext->getKeyboardInputContext().textEdited.off(this);
	m_inputContext->getKeyboardInputContext().setEnableTextInput(false);
	m_flat.input->popContext(m_inputContext);
	m_inputContext.reset();
	return true;
}

bool TextInputWidget::onMouseDown(Widget* widget, bool&)
{
	if (hasSelectedText())
	{
		unselect();
	}
	auto& mouse = m_flat.input->mouse;
	const float mouseX = getRelativePosition(mouse->getPosition()).x;
	moveCursorAt(getCursorIndexFromPosition(mouseX));
	return true;
}

bool TextInputWidget::onMouseMove(Widget* widget, bool&)
{
	auto& mouse = m_flat.input->mouse;
	if (hasFocus() && mouse->isPressed(M(LEFT)))
	{
		const float mouseX = getRelativePosition(mouse->getPosition()).x;
		m_cursorIndex = getCursorIndexFromPosition(mouseX);
		selectTo(m_selectionIndex);
	}
	return true;
}

bool TextInputWidget::keyJustPressed(input::Key key)
{
	std::string text = getText();
	const bool ctrlPressed = m_inputContext->getKeyboardInputContext().isPressed(K(LCTRL)) || m_inputContext->getKeyboardInputContext().isPressed(K(RCTRL));
	const bool shiftPressed = m_inputContext->getKeyboardInputContext().isPressed(K(LSHIFT)) || m_inputContext->getKeyboardInputContext().isPressed(K(RSHIFT));
	if (key == K(BACKSPACE) && !text.empty())
	{
		if (hasSelectedText())
		{
			changeSelectedText("");
			valueChanged(this);
		}
		else if(m_cursorIndex > 0)
		{
			FLAT_ASSERT(m_cursorIndex - 1 < text.size());
			text = text.erase(m_cursorIndex - 1, 1);
			if (text != getText())
			{
				moveCursor(-1);
				setText(text);
				valueChanged(this);
			}
		}
	}
	else if (key == K(DELETE) && !text.empty())
	{
		if (hasSelectedText())
		{
			changeSelectedText("");
			unselect();
			valueChanged(this);
		}
		else if (m_cursorIndex < text.size())
		{
			text = text.erase(m_cursorIndex, 1);
			if (text != getText())
			{
				setText(text);
				valueChanged(this);
			}
		}
	}
	else if (key == K(RETURN) || key == K(KP_ENTER))
	{
		submit(this);
	}
	else if (key == K(LEFT))
	{
		const CursorIndex currentIndex = m_selectionIndex;
		if(ctrlPressed)
		{
			moveCursorAt(previousWordFrom(m_cursorIndex));
		}
		else
		{
			if (hasSelectedText() && !shiftPressed)
			{
				m_cursorIndex = std::min(m_cursorIndex, m_selectionIndex);
				unselect();
			}
			else
			{
				moveCursor(-1);
			}
		}
		if (shiftPressed)
		{
			selectTo(currentIndex);
		}
	}
	else if (key == K(RIGHT))
	{
		const CursorIndex currentIndex = m_selectionIndex;
		if (ctrlPressed)
		{
			moveCursorAt(nextWordFrom(m_cursorIndex));
		}
		else
		{
			if (hasSelectedText() && !shiftPressed)
			{
				m_cursorIndex = std::max(m_cursorIndex, m_selectionIndex);
				unselect();
			}
			else
			{
				moveCursor(1);
			}
		}
		if (shiftPressed)
		{
			selectTo(currentIndex);
		}
	}
	return true;
}

bool TextInputWidget::textEdited(const std::string& text)
{
	if (!text.empty())
	{
		changeSelectedText(text);
		valueChanged(this);
	}
	return true;
}

void TextInputWidget::moveCursor(int offset)
{
	if (offset > 0 || m_cursorIndex >= static_cast<CursorIndex>(std::abs(offset)))
	{
		m_cursorIndex += offset;
		if (m_cursorIndex < 0)
		{
			m_cursorIndex = 0;
		}
		else if (m_cursorIndex > getText().size())
		{
			m_cursorIndex = getText().size();
		}
		m_selectionIndex = m_cursorIndex;
	}
}

void TextInputWidget::moveCursorAt(CursorIndex index)
{
	FLAT_ASSERT(index >= 0 && index <= getText().size());
	m_cursorIndex = index;
	unselect();
}

void TextInputWidget::selectTo(CursorIndex to)
{
	FLAT_ASSERT(to >= 0 && to <= getText().size())
	m_selectionIndex = to;
	setColor(0u, static_cast<unsigned int>(getText().size()), getTextColor());
	unsigned int first = static_cast<unsigned int>(std::min(m_selectionIndex, m_cursorIndex));
	unsigned int last = static_cast<unsigned int>(std::max(m_selectionIndex, m_cursorIndex));
	setColor(first, last, video::Color::WHITE);
}

void TextInputWidget::unselect()
{
	setColor(0u, static_cast<unsigned int>(getText().size()), getTextColor());
	m_selectionIndex = m_cursorIndex;
}

std::string TextInputWidget::replaceSelectedText(const std::string& text)
{
	CursorIndex min = std::min(m_cursorIndex, m_selectionIndex);
	CursorIndex max = std::max(m_cursorIndex, m_selectionIndex);
	std::string currentText = getText();
	currentText.erase(min, max - min);
	currentText.insert(min, text);
	return currentText;
}

void TextInputWidget::changeSelectedText(const std::string& text)
{
	std::string newText = replaceSelectedText(text);
	if (newText != getText())
	{
		setText(newText);
	}
	moveCursorAt(std::min(m_cursorIndex, m_selectionIndex) + text.size());
}

TextInputWidget::CursorIndex TextInputWidget::nextWordFrom(CursorIndex index) const
{
	const std::string& text = getText();
	CursorIndex nextSpaceIndex = text.find_first_of(" ", index);
	if (nextSpaceIndex == index)
	{
		return index + 1;
	}
	else if (nextSpaceIndex != std::string::npos)
	{
		return nextSpaceIndex;
	}
	else
	{
		return text.size();
	}
}

TextInputWidget::CursorIndex TextInputWidget::previousWordFrom(CursorIndex index) const
{
	if (index > 0)
	{
		CursorIndex previousSpaceIndex = getText().find_last_of(" ", index - 1) + 1;
		if (previousSpaceIndex > 0 && previousSpaceIndex == index)
		{
			return index - 1;
		}
		else if (previousSpaceIndex != std::string::npos)
		{
			return previousSpaceIndex;
		}
	}
	return 0;
}

bool TextInputWidget::hasSelectedText()
{
	return m_cursorIndex != m_selectionIndex;
}

float TextInputWidget::getCursorPositionFromIndex(CursorIndex cursorIndex) const
{
	const size_t textLength = getText().size();
	FLAT_ASSERT_MSG(0 <= cursorIndex && cursorIndex <= textLength, "the cursor index is out of the string's range");
	const std::vector<CharacterVertex>& vertices = getVertices();
	if (cursorIndex == 0)
	{
		return 0.f;
	}
	else
	{
		return vertices[(cursorIndex - 1) * 6 + 1].x;
	}
}

TextInputWidget::CursorIndex TextInputWidget::getCursorIndexFromPosition(float x) const
{
	const size_t textLength = getText().size();
	if (textLength == 0)
	{
		return 0;
	}

	const std::vector<CharacterVertex>& vertices = getVertices();
	if (x < vertices[0].x)
	{
		return 0;
	}
	else if (x > vertices[vertices.size() - 1].x)
	{
		return textLength;
	}
	else
	{
		CursorIndex minIndex = 0;
		CursorIndex maxIndex = textLength;
		while (minIndex + 1 < maxIndex)
		{
			CursorIndex centerIndex = (minIndex + maxIndex) / 2;
			float centerCharacterPositionX = getCursorPositionFromIndex(centerIndex);
			if (x < centerCharacterPositionX)
			{
				maxIndex = centerIndex;
			}
			else
			{
				minIndex = centerIndex;
			}
		}

		if (x - getCursorPositionFromIndex(minIndex) < getCursorPositionFromIndex(maxIndex) - x)
		{
			return minIndex;
		}
		else
		{
			return maxIndex;
		}
	}
}

void TextInputWidget::drawCursor(const render::RenderSettings& renderSettings, CursorIndex cursorIndex) const
{
	if (!hasFocus())
		return;

	renderSettings.modelMatrixUniform.set(m_transform);

	renderSettings.colorUniform.set(getTextColor());
	renderSettings.secondaryColorUniform.set(video::Color::BLACK);
	renderSettings.textureGivenUniform.set(false);
	renderSettings.vertexColorGivenUniform.set(false);

	const video::font::Font* font = getFont().get();
	FLAT_ASSERT(font != nullptr);

	const float characterHeight = font->getAtlasSize().y;
	const float cursorX = getCursorPositionFromIndex(cursorIndex);

	std::array<String::CharacterVertex, 2> cursorVertices = {
		String::CharacterVertex(cursorX, 0.f),
		String::CharacterVertex(cursorX, characterHeight)
	};

	glLineWidth(1);

	glEnableVertexAttribArray(renderSettings.positionAttribute);
	glVertexAttribPointer(renderSettings.positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(CharacterVertex), reinterpret_cast<const float*>(&cursorVertices[0]));
	glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(cursorVertices.size()));

	glDisableVertexAttribArray(renderSettings.positionAttribute);
}

void TextInputWidget::drawSelection(const render::RenderSettings& renderSettings, CursorIndex first, CursorIndex last) const
{
	if (!hasFocus() || first == last)
		return;

	renderSettings.modelMatrixUniform.set(m_transform);

	renderSettings.colorUniform.set(video::Color(uint32_t(0x4286f4FF)));
	renderSettings.secondaryColorUniform.set(video::Color::BLACK);
	renderSettings.textureGivenUniform.set(false);
	renderSettings.vertexColorGivenUniform.set(false);

	const video::font::Font* font = getFont().get();
	FLAT_ASSERT(font != nullptr);

	const float characterHeight = font->getAtlasSize().y;
	const float firstX = getCursorPositionFromIndex(first);
	const float lastX = getCursorPositionFromIndex(last);

	std::array<String::CharacterVertex, 6> cursorVertices = {
		String::CharacterVertex(firstX, 0.f),
		String::CharacterVertex(lastX, 0.f),
		String::CharacterVertex(firstX, characterHeight),
		String::CharacterVertex(firstX, characterHeight),
		String::CharacterVertex(lastX, 0.f),
		String::CharacterVertex(lastX, characterHeight)
	};

	glLineWidth(1);

	glEnableVertexAttribArray(renderSettings.positionAttribute);
	glVertexAttribPointer(renderSettings.positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(CharacterVertex), reinterpret_cast<const float*>(&cursorVertices[0]));
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cursorVertices.size()));

	glDisableVertexAttribArray(renderSettings.positionAttribute);
}

} // ui
} // sharp
} // flat


