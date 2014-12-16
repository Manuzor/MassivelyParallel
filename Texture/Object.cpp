#include "PCH.h"
#include "Object.h"

Object::Object()
{
  m_Texture.setSmooth(true);
  m_Texture.setRepeated(true);
  m_Sprite.setTexture(m_Texture);
}

void Initialize(Object& object)
{
  if(object.m_bInitialized)
    return;

  /// \todo do some initialization here.

  object.m_bInitialized = true;
}
