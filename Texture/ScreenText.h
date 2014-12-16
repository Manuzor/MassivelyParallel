#pragma once

class ScreenText
{
  // Data
  //////////////////////////////////////////////////////////////////////////
  sf::Text m_Text;
  sf::Font m_Font;

  // Friends
  //////////////////////////////////////////////////////////////////////////
  friend       sf::Text& TextOf(      ScreenText& self) { return self.m_Text; }
  friend const sf::Text& TextOf(const ScreenText& self) { return self.m_Text; }
  friend       sf::Font& FontOf(      ScreenText& self) { return self.m_Font; }
  friend const sf::Font& FontOf(const ScreenText& self) { return self.m_Font; }
  friend       sf::Transformable& TransformOf(      ScreenText& self) { return self.m_Text; }
  friend const sf::Transformable& TransformOf(const ScreenText& self) { return self.m_Text; }

public:
  ScreenText() { m_Text.setFont(m_Font); }
};

void Append(sf::Text& text, const sf::String& string)
{
  text.setString(text.getString() + string);
}

void Append(sf::Text& text, mpUInt32 unicodeCodePoint)
{
  text.setString(text.getString() + unicodeCodePoint);
}

/// \brief Appends \a unicodeCodePoint to the text of \a input
void Append(ScreenText& screenText, mpUInt32 unicodeCodePoint)
{
  Append(TextOf(screenText), unicodeCodePoint);
}

/// \brief Appends \a string to the text of \a text
void Append(ScreenText& screenText, const sf::String& string)
{
  Append(TextOf(screenText), string);
}

void Draw(sf::RenderTarget& target, const ScreenText& screenText)
{
  target.draw(TextOf(screenText));
}

void Clear(ScreenText& screenText)
{
  TextOf(screenText).setString("");
}

void AlignWith(ScreenText& self,
               const sf::Transformable& target,
               const sf::Vector2f& offset = { 0.0f, 0.0f })
{
  TransformOf(self).setPosition(target.getPosition() + offset);
}
