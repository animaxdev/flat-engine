#include <iostream>
#include "../geometry/rectangle.h"
#include "sprite.h"

namespace flat
{
namespace util
{

Sprite::Sprite() :
	m_texture(NULL),
	m_scale(1.f),
	m_updateModelMatrix(true)
{
	
}

Sprite::~Sprite()
{
	
}

Sprite* Sprite::lightCopy()
{
	return new Sprite(*this);
}

void Sprite::draw(const RenderSettings& renderSettings, const geometry::Matrix4& viewMatrix)
{
	renderSettings.textureUniform.setTexture(m_texture);
	updateModelMatrix();
	renderSettings.modelMatrixUniform.setMatrix4(m_modelMatrix);
	geometry::Rectangle rectangle(m_texture->getSize() / -2.f, m_texture->getSize());
	rectangle.draw(renderSettings.positionAttribute, renderSettings.uvAttribute);
}

void Sprite::updateModelMatrix()
{
	if (m_updateModelMatrix)
	{
		m_updateModelMatrix = false;
		m_modelMatrix.setIdentity();
		m_modelMatrix.rotateZ(m_rotation.getZ());
		m_modelMatrix.rotateY(m_rotation.getY());
		m_modelMatrix.rotateX(m_rotation.getX());
		m_modelMatrix.scale(m_scale);
		m_modelMatrix.translate(m_position);
	}
}

} // util
} // flat


