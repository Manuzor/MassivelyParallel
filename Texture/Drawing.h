#pragma once

template<typename Type>
struct DrawInstruction
{
  const Type* m_pSurface;
  bool m_bCalled = false;

  MP_ForceInline ~DrawInstruction() { MP_Assert(m_bCalled, "You forgot to call Draw!"); }

  template<typename DrawableType>
  MP_ForceInline void Draw(DrawableType& drawable) { m_bCalled = true; m_pSurface->draw(drawable); }

private:
  MP_ForceInline explicit DrawInstruction(const Type& object) : m_pSurface(&object) {}

  template<typename T>
  friend DrawInstruction<T> MakeDrawInstruction(const T& object);
};

template<typename Type> MP_ForceInline
DrawInstruction<Type> MakeDrawInstruction(const Type& object)
{
  return DrawInstruction<Type>(object);
}
