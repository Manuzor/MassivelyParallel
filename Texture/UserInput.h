#pragma once

class UserInput
{
  // Data
  //////////////////////////////////////////////////////////////////////////
  sf::Text m_Text;
  sf::Font m_Font;

  // Friends
  //////////////////////////////////////////////////////////////////////////
  friend sf::Text& TextOf(UserInput& input);
  friend void Initialize(UserInput& input, const char* szFontFileName);
};

MP_ForceInline
sf::Text& TextOf(UserInput& input) { return input.m_Text; }
MP_ForceInline
const sf::Text& TextOf(const UserInput& input) { return TextOf(const_cast<UserInput&>(input)); }

void Initialize(UserInput& input, const char* szFontFileName)
{
  MP_LogBlock("Initializing UserInput", szFontFileName);
  if (!input.m_Font.loadFromFile(szFontFileName))
  {
    MP_ReportError("Failed to load font.");
  }

  TextOf(input).setFont(input.m_Font);
}

void operator +=(sf::Text& lhs, const sf::String& rhs) { lhs.setString(lhs.getString() + rhs); }

/// \brief Appends \a unicodeCodePoint to the text of \a input
void Append(UserInput& input, mpUInt32 unicodeCodePoint)
{
    TextOf(input) += unicodeCodePoint;
}

void Draw(sf::RenderTarget& target, const UserInput& input)
{
  target.draw(TextOf(input));
}

void Clear(UserInput& input)
{
  TextOf(input).setString("");
}
