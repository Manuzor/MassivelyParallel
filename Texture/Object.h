#pragma once

class Object : public sf::Drawable
{
  // Data
  //////////////////////////////////////////////////////////////////////////
  sf::String  m_sTextureName;
  sf::Texture m_Texture;
  sf::Sprite  m_Sprite;

  // Friends
  //////////////////////////////////////////////////////////////////////////
  friend       sf::Sprite& SpriteOf(      Object& object) { return object.m_Sprite; }
  friend const sf::Sprite& SpriteOf(const Object& object) { return object.m_Sprite; }
  friend       sf::Texture& TextureOf(      Object& object) { return object.m_Texture; }
  friend const sf::Texture& TextureOf(const Object& object) { return object.m_Texture; }
  friend void Initialize(Object& object, const char* szTextureName);
  friend void Initialize(Object& object, mpUInt32 uiWidth, mpUInt32 uiHeight);

  // sf::Drawable interface
  //////////////////////////////////////////////////////////////////////////
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final override
  {
    target.draw(m_Sprite, states);
  }
};

MP_ForceInline
void Initialize(Object& object, sf::Vector2u dimensions)
{
  Initialize(object, dimensions.x, dimensions.y);
}

struct PositionHelper
{

};

MP_ForceInline       sf::Transformable& TransformOf(      Object& object) { return SpriteOf(object); }
MP_ForceInline const sf::Transformable& TransformOf(const Object& object) { return SpriteOf(object); }

MP_ForceInline
void Draw(sf::RenderTarget& target, const Object& object)
{
  target.draw(object);
}
