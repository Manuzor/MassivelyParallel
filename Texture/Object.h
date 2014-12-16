#pragma once

template<typename WT, ///< Wrapped Type
         typename GT, ///< Getter-Functor Type
         typename ST> ///< Setter-Functor Type
struct PropertyBase
{
  using WrappedType = WT;
  using GetterType  = GT;
  using SetterType  = ST;

  GetterType m_get;
  SetterType m_set;

  void operator=(WrappedType& object) { m_set(object); }
  operator WrappedType&() { return m_get(); }
};

template<typename Type>
using Property = PropertyBase<Type,
                              std::function<Type&()>,
                              std::function<void(Type&)>>;

class Object : public sf::Drawable
{
  // Data
  //////////////////////////////////////////////////////////////////////////
  sf::Texture  m_Texture;
  sf::Sprite   m_Sprite;
  bool m_bInitialized = false;

  // Friends
  //////////////////////////////////////////////////////////////////////////
  friend       sf::Sprite& SpriteOf(      Object& object) { return object.m_Sprite; }
  friend const sf::Sprite& SpriteOf(const Object& object) { return object.m_Sprite; }
  friend       sf::Texture& TextureOf(      Object& object) { return object.m_Texture; }
  friend const sf::Texture& TextureOf(const Object& object) { return object.m_Texture; }
  friend void Initialize(Object& object);

  // sf::Drawable interface
  //////////////////////////////////////////////////////////////////////////
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final override
  {
    MP_Assert(m_bInitialized,
              "You forgot to call 'Initialize(object)' after creating it or changing its state.");
    target.draw(m_Sprite, states);
  }

public:
  Object();
};

MP_ForceInline       sf::Transformable& TransformOf(      Object& object) { return SpriteOf(object); }
MP_ForceInline const sf::Transformable& TransformOf(const Object& object) { return SpriteOf(object); }

MP_ForceInline
void Draw(sf::RenderTarget& target, const Object& object)
{
  target.draw(object);
}
