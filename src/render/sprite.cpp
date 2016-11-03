#include "sprite.h"

namespace flat
{
namespace render
{

Sprite::Sprite() :
	m_scale(1.f),
	m_flipX(false),
	m_flipY(false),
	m_isLightCopy(false),
	m_modelMatrixIsDirty(true)
{
	// pos
	m_vertices[0].pos.x = 0.f;
	m_vertices[0].pos.y = 0.f;
	
	m_vertices[1].pos.x = 1.f; // will change according to the texture size
	m_vertices[1].pos.y = 0.f;

	m_vertices[2].pos.x = 0.f;
	m_vertices[2].pos.y = 1.f; // will change according to the texture size
	
	m_vertices[3].pos.x = 1.f; // will change according to the texture size
	m_vertices[3].pos.y = 1.f; // will change according to the texture size
	
	m_vertices[4].pos.x = 0.f;
	m_vertices[4].pos.y = 1.f; // will change according to the texture size

	m_vertices[5].pos.x = 1.f; // will change according to the texture size
	m_vertices[5].pos.y = 0.f;
	
	// uv
	m_vertices[0].uv.x = 0.f;
	m_vertices[0].uv.y = 0.f;
	
	m_vertices[1].uv.x = 1.f;
	m_vertices[1].uv.y = 0.f;
	
	m_vertices[2].uv.x = 0.f;
	m_vertices[2].uv.y = 1.f;
	
	m_vertices[3].uv.x = 1.f;
	m_vertices[3].uv.y = 1.f;

	m_vertices[4].uv.x = 0.f;
	m_vertices[4].uv.y = 1.f;

	m_vertices[5].uv.x = 1.f;
	m_vertices[5].uv.y = 0.f;
}

Sprite::~Sprite()
{
	
}

Sprite* Sprite::lightCopy()
{
	Sprite* sprite = new Sprite(*this);
	sprite->m_isLightCopy = true;
	return sprite;
}

void Sprite::setTexture(const std::shared_ptr<const video::Texture>& texture)
{
	m_texture = texture;
	const video::Texture* t = texture.get();
	FLAT_ASSERT_MSG(t != nullptr, "Cannot set a null texture on a sprite");
	const Vector2& textureSize = t->getSize();
	m_origin = textureSize / 2.f;
	m_modelMatrixIsDirty = true;
	m_vertices[1].pos.x = textureSize.x;
	m_vertices[2].pos.y = textureSize.y;
	m_vertices[3].pos.x = textureSize.x;
	m_vertices[3].pos.y = textureSize.y;
	m_vertices[4].pos.y = textureSize.y;
	m_vertices[5].pos.x = textureSize.x;
}

void Sprite::draw(const RenderSettings& renderSettings, const Matrix4& /*viewMatrix*/) const
{
	const video::Texture* texture = getTexture().get();
	FLAT_ASSERT(texture != nullptr);
	renderSettings.textureUniform.set(texture);
	renderSettings.colorUniform.set(m_color);
	updateModelMatrix();
	renderSettings.modelMatrixUniform.set(m_modelMatrix);
	
	video::Attribute positionAttribute = renderSettings.positionAttribute;
	video::Attribute uvAttribute = renderSettings.uvAttribute;
	
	glEnableVertexAttribArray(positionAttribute);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &m_vertices[0].pos);
	
	glEnableVertexAttribArray(uvAttribute);
	glVertexAttribPointer(uvAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &m_vertices[0].uv);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(uvAttribute);
}

void Sprite::updateModelMatrix() const
{
	if (m_modelMatrixIsDirty)
	{
		m_modelMatrixIsDirty = false;
		m_modelMatrix = Matrix4();
		translateBy(m_modelMatrix, Vector3(m_position, 0.f));
		rotateBy(m_modelMatrix, m_rotation);
		scaleBy(m_modelMatrix, m_scale);
		float scaleX = m_flipX ? -1.f : 1.f;
		float scaleY = m_flipY ? -1.f : 1.f;
		scaleBy(m_modelMatrix, Vector2(scaleX, scaleY));
		translateBy(m_modelMatrix, Vector3(-m_origin, 0.f));
	}
}

} // render
} // flat



