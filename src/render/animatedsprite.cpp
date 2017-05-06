#include <cmath>
#include "animatedsprite.h"
#include "../misc/aabb2.h"
#include "../video/filetexture.h"

namespace flat
{
namespace render
{

AnimatedSprite::AnimatedSprite() : Sprite(),
	m_tileSizeRatio(1.f, 1.f),
	m_currentLine(0),
	m_currentColumn(0),
	m_lastUpdateTime(0.f),
	m_frameDuration(1.f),
	m_numFrames(1),
	m_animated(false),
	m_animationStarted(false)
{
	
}

AnimatedSprite::~AnimatedSprite()
{
	
}

void AnimatedSprite::getPixel(const Vector2& point, video::Color& color) const
{
	AABB2 aabb;
	getAABB(aabb);
	FLAT_ASSERT(aabb.isInside(point));
	FLAT_ASSERT_MSG(dynamic_cast<const video::FileTexture*>(m_texture.get()) != nullptr, "Can only get pixel color from a file texture");
	const video::FileTexture* texture = static_cast<const video::FileTexture*>(m_texture.get());

	float rx =       (point.x - aabb.min.x) / (aabb.max.x - aabb.min.x);
	float ry = 1.f - (point.y - aabb.min.y) / (aabb.max.y - aabb.min.y);

	if (m_flipX)
		rx = 1.f - rx;

	if (m_flipY)
		ry = 1.f - ry;

	Vector2 pixelPosition(
		(rx + m_currentColumn                   ) * m_tileSizeRatio.x * texture->getSize().x,
		(ry + (getAtlasHeight() - 1 - m_currentLine)) * m_tileSizeRatio.y * texture->getSize().y
	);

	texture->getPixel(pixelPosition, color);
}

void AnimatedSprite::setAtlasSize(int atlasWidth, int atlasHeight)
{
	const flat::video::Texture* texture = getTexture().get();
	FLAT_ASSERT(texture != nullptr);
	const flat::Vector2& textureSize = texture->getSize();
	FLAT_ASSERT(0 < atlasWidth  && atlasWidth  < textureSize.x);
	FLAT_ASSERT(0 < atlasHeight && atlasHeight < textureSize.y);
	FLAT_ASSERT_MSG(
		   textureSize.x / atlasWidth  == floor(textureSize.x / atlasWidth)
		&& textureSize.y / atlasHeight == floor(textureSize.y / atlasHeight),
		"Cannot use size (%d, %d) for atlas %s of size (%d, %d)",
		atlasWidth, atlasHeight, texture->getName().c_str(), static_cast<int>(textureSize.x), static_cast<int>(textureSize.y)
	);
	m_tileSizeRatio.x = 1.f / atlasWidth;
	m_tileSizeRatio.y = 1.f / atlasHeight;
	const float atlasTileWidth = textureSize.x / atlasWidth;
	const float atlasTileHeight = textureSize.y / atlasHeight;
	m_vertices[1].pos.x = atlasTileWidth;
	m_vertices[2].pos.y = atlasTileHeight;
	m_vertices[3].pos.x = atlasTileWidth;
	m_vertices[3].pos.y = atlasTileHeight;
	m_vertices[4].pos.y = atlasTileHeight;
	m_vertices[5].pos.x = atlasTileWidth;
	setLine(0);
	setColumn(0);
}

void AnimatedSprite::setAnimated(bool animated)
{
	if (!m_animated && animated)
	{
		m_animationStarted = false;
	}
	m_animated = animated;
}

void AnimatedSprite::setLine(int line)
{
	FLAT_ASSERT(0 <= line && line < getAtlasHeight());
	m_currentLine = line;
	
	// update uv
	float uvy0 = m_tileSizeRatio.y * line;
	float uvy1 = m_tileSizeRatio.y * (line + 1);
	
	m_vertices[0].uv.y = uvy0;
	m_vertices[1].uv.y = uvy0;
	m_vertices[2].uv.y = uvy1;
	m_vertices[3].uv.y = uvy1;
	m_vertices[4].uv.y = uvy1;
	m_vertices[5].uv.y = uvy0;
}

void AnimatedSprite::setColumn(int column)
{
	FLAT_ASSERT(0 <= column && column < getAtlasWidth());
	m_currentColumn = column;
	
	// update uv
	float uvx0 = m_tileSizeRatio.x * column;
	float uvx1 = m_tileSizeRatio.x * (column + 1);
	
	m_vertices[0].uv.x = uvx0;
	m_vertices[1].uv.x = uvx1;
	m_vertices[2].uv.x = uvx0;
	m_vertices[3].uv.x = uvx1;
	m_vertices[4].uv.x = uvx0;
	m_vertices[5].uv.x = uvx1;
}

void AnimatedSprite::playAnimation(int line, int numFrames, float frameDuration, int numLoops)
{
	setLine(line);
	setColumn(0);
	FLAT_ASSERT(numFrames <= getAtlasWidth());
	setNumFrames(numFrames);
	setFrameDuration(frameDuration);
	setNumLoops(numLoops);
	setAnimated(true);
	m_animationStarted = false;
}

void AnimatedSprite::update(float currentTime)
{
	if (m_animated)
	{
		if (!m_animationStarted)
		{
			m_animationStarted = true;
			m_lastUpdateTime = currentTime;
		}
		
		if (currentTime - m_lastUpdateTime > m_frameDuration)
		{
			int nextColumn = m_currentColumn + 1;
			if (nextColumn >= m_numFrames)
			{
				nextColumn = 0;
				if (m_numLoops != INFINITE_LOOP)
				{
					--m_numLoops;
					if (m_numLoops == 0)
					{
						nextColumn = m_currentColumn;
						setAnimated(false);
					}
				}
			}
			setColumn(nextColumn);
			m_lastUpdateTime += m_frameDuration;
		}
	}
	else
	{
		m_lastUpdateTime = currentTime;
	}
}

int AnimatedSprite::getAtlasWidth() const
{
	return static_cast<int>(1.f / m_tileSizeRatio.x);
}

int AnimatedSprite::getAtlasHeight() const
{
	return static_cast<int>(1.f / m_tileSizeRatio.y);
}

} // render
} // flat



