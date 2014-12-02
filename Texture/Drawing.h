#pragma once

template<typename Type>
struct DrawInstruction
{
  const Type* m_pDrawable;
  bool m_bDrawn = false;

  MP_ForceInline ~DrawInstruction() { MP_Assert(m_bDrawn, ""); }

  template<typename SurfaceType>
  MP_ForceInline void On(SurfaceType& surface) { m_bDrawn = true; surface.draw(*m_pDrawable); }

private:
  MP_ForceInline explicit DrawInstruction(const Type& object) : m_pDrawable(&object) {}

  template<typename T>
  friend DrawInstruction<T> MakeDrawInstruction(const T& object);
};

template<typename Type> MP_ForceInline
DrawInstruction<Type> MakeDrawInstruction(const Type& object)
{
  return DrawInstruction<Type>(object);
}
