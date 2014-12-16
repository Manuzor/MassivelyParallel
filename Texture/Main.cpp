#include "PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

#include "Object.h"
#include "ScreenText.h"

struct ProfileScope
{
  mpTime m_Begin;
  std::string m_Name;

  ProfileScope(const char* szName) : m_Begin(mpTime::Now()), m_Name(szName)
  {
  }

  ~ProfileScope()
  {
    mpLog::Dev("'%s' finished in %f seconds", m_Name.c_str(), (mpTime::Now() - m_Begin).GetSeconds());
  }
};

#define Profile(szName) ::ProfileScope MP_Concat(_profile_, __LINE__)(szName)
#define ProfiledLogBlock(szName) MP_LogBlock("Profiling: %s", szName); ::ProfileScope MP_Concat(_profile_, __LINE__)(szName)

class Main : public mpApplication
{
  sf::RenderWindow  m_Window;
  mpTime            m_Beginning;
  ScreenText        m_UserInput;
  ScreenText        m_Info;

  Object m_Original;
  ScreenText m_OriginalCaption;
  Object m_Result;
  ScreenText m_ResultCaption;

  /// CL Stuff
  mpPlatform m_Platform = mpPlatform::Get();
  mpDevice m_Device = mpDevice::GetGPU(m_Platform, 0);
  mpContext m_Context;
  mpCommandQueue m_Queue;
  mpProgram m_Program;
  mpKernel m_Kernel_BlendX;
  mpKernel m_Kernel_BlendY;

  /// The maximum dimensions of the render window.
  const sf::VideoMode m_MaxVideo = sf::VideoMode::getDesktopMode();

  /// The minimum distance between objects and the window borders;
  const float m_fMargin = 20.0f;

  /// The number of objects in x and y directions
  const sf::Vector2u m_NumObjects = { 2,   // horizontally
                                      1 }; // vertically

  const sf::Vector2u m_NumTilings = { 3,   // horizontally
                                      2 }; // vertically

  virtual void PreStartup() final override
  {
    m_Beginning = mpTime::Now();
  }

  virtual void PostStartup() final override
  {
    // Initialize CL stuff
    m_Context.Initialize(m_Device);
    m_Queue.Initialize(m_Context, m_Device);
    MP_Verify(m_Program.LoadAndBuild(m_Context, m_Device, "Kernels/Blend.cl"));
    m_Kernel_BlendX.Initialize(m_Queue, m_Program, "BlendX");
    m_Kernel_BlendY.Initialize(m_Queue, m_Program, "BlendY");

    auto& texture = TextureOf(m_Original);

    // Init original image.
    {
      SFML_Verify(texture.loadFromFile("Data/fern.png"));
    }

    sf::IntRect spriteRect = { 0, 0, (int)texture.getSize().x, (int)texture.getSize().y };
    {
      spriteRect.width  *= m_NumTilings.x;
      spriteRect.height *= m_NumTilings.y;
    }

    sf::Vector2u textureDimensions = texture.getSize();

    // Create the result texture by copying the original
    {
      SFML_Verify(TextureOf(m_Result).loadFromImage(texture.copyToImage()));
    }

    // Set up tiling
    {
      SpriteOf(m_Original).setTextureRect(spriteRect);
      SpriteOf(m_Result).setTextureRect(spriteRect); // Tiling
    }

    // Set positions
    {
      TransformOf(m_Original).setPosition(m_fMargin, 2 * m_fMargin);
      TransformOf(m_Result).setPosition(
        m_fMargin + texture.getSize().x * m_NumTilings.x + m_fMargin,
        2 * m_fMargin);
    }

    Initialize(m_Original);
    SFML_Verify(FontOf(m_OriginalCaption).loadFromFile("Data/Fonts/arial.ttf"));
    TextOf(m_OriginalCaption).setCharacterSize((const unsigned int)m_fMargin);
    Append(m_OriginalCaption, "Original");
    AlignWith(m_OriginalCaption, TransformOf(m_Original), { m_fMargin, -1.5f * m_fMargin });

    Initialize(m_Result);
    SFML_Verify(FontOf(m_ResultCaption).loadFromFile("Data/Fonts/arial.ttf"));
    TextOf(m_ResultCaption).setCharacterSize((const unsigned int)m_fMargin);
    Append(m_ResultCaption, "Result");
    AlignWith(m_ResultCaption, TransformOf(m_Result), { m_fMargin, -1.5f * m_fMargin });

    SFML_Verify(FontOf(m_UserInput).loadFromFile("Data/Fonts/arial.ttf"));
    //TextOf(m_UserInput) += "Choose Texture: ";

    SFML_Verify(FontOf(m_Info).loadFromFile("Data/Fonts/arial.ttf"));
    TextOf(m_Info).setCharacterSize((const unsigned int)(m_fMargin * 0.8f));
    Append(m_Info, "Keys: 1 - BlendX | 2 - BlendY | 3 - <Not Implemented> | R - Reset result image");

    // Initialize (create) window
    sf::VideoMode video (
      mpUInt32((m_fMargin + spriteRect.width) * m_NumObjects.x + m_fMargin),
      mpUInt32(m_fMargin + (m_fMargin + spriteRect.height) * m_NumObjects.y + 2 * m_fMargin)
    );

    video.width  = mpMath::Clamp(video.width,  0U, m_MaxVideo.width);
    video.height = mpMath::Clamp(video.height, 0U, m_MaxVideo.height);

    m_Window.create(video, "Texture Blending");

    TransformOf(m_Info).setPosition({
      m_fMargin,
      video.height - m_fMargin * 1.5f });
  }

  void InvokeKernel(mpKernel& Kernel)
  {
    MP_LogBlock("Kernel Invocation (%s)", Kernel.GetName());

    auto vPixelDimensions = TextureOf(m_Result).getSize();
    auto uiPixelCount = vPixelDimensions.x * vPixelDimensions.y;
    size_t GlobalWorkSize[2] = { vPixelDimensions.x, vPixelDimensions.y };

    auto Image = TextureOf(m_Result).copyToImage();
    auto Pixels = (cl_uchar4*)Image.getPixelsPtr();
    auto PixelsPtr = mpMakeArrayPtr(Pixels, uiPixelCount);

    mpBuffer InputBuffer;
    mpBuffer ResultBuffer;

    {
      ProfiledLogBlock("Initialization");

      // Copy pixels to GPU
      {
        Profile("Initializing InputBuffer");
        InputBuffer.Initialize(m_Context, mpBufferFlags::ReadOnly, PixelsPtr);
      }

      // Create buffer on GPU for the result
      {
        Profile("Initializing ResultBuffer");
        ResultBuffer.Initialize(m_Context, mpBufferFlags::WriteOnly, uiPixelCount * sizeof(cl_uchar4));
      }

      // Run Kernel with input and result buffer as arguments
      {
        Profile("Pushing Kernel Args");
        Kernel.PushArg(InputBuffer);
        Kernel.PushArg(ResultBuffer);
      }
    }

    {
      ProfiledLogBlock("Execution");
      {
        Profile("Executing Kernel");
        Kernel.Execute(GlobalWorkSize);
      }

      {
        Profile("Retrieve Results");
        // Copy results from GPU back to CPU
        ResultBuffer.ReadInto(PixelsPtr, m_Queue);
      }
    }

    {
      ProfiledLogBlock("Texture Update");
      // Update the contents of the result texture with the output of the Kernel
      TextureOf(m_Result).update(Image);
    }
  }

  void HandleText(sf::Event::TextEvent& text)
  {
    if(text.unicode < 32)
      return;

    Append(m_UserInput, text.unicode);
  }

  void ResetResultImage()
  {
    TextureOf(m_Result).update(TextureOf(m_Original).copyToImage());
  }

  void HandleInput(sf::Event::KeyEvent& key)
  {
    switch(key.code)
    {
    case sf::Keyboard::Escape:
      m_Window.close();
      break;
    case sf::Keyboard::Return:
      Clear(m_UserInput);
      break;
    case sf::Keyboard::R:
      ResetResultImage();
      break;
    case sf::Keyboard::Num1:
      InvokeKernel(m_Kernel_BlendX);
      break;
    case sf::Keyboard::Num2:
      InvokeKernel(m_Kernel_BlendY);
      break;
    case sf::Keyboard::Num3:
      //InvokeKernel(m_Kernel_Blend);
      break;
    case sf::Keyboard::Add:
    {
      auto view = m_Window.getView();
      view.zoom(1.0f);
      m_Window.setView(view);
    }
    break;
    case sf::Keyboard::Subtract:
    {
      auto view = m_Window.getView();
      view.zoom(-1.0f);
      m_Window.setView(view);
    }
      break;
    }
  }

  void HandleEvent(sf::Event& event)
  {
    switch(event.type)
    {
    case sf::Event::KeyReleased:
      HandleInput(event.key);
      break;
    case sf::Event::TextEntered:
      HandleText(event.text);
      break;
    case sf::Event::Closed:
      m_Window.close();
      break;
    }
  }

  virtual QuitOrContinue Run() final override
  {
    if (!m_Window.isOpen())
      return Quit;

    for(sf::Event event; m_Window.pollEvent(event);)
    {
      HandleEvent(event);
    }

    // General Update section
    //////////////////////////////////////////////////////////////////////////

    // Rendering
    //////////////////////////////////////////////////////////////////////////

    m_Window.clear({ 99, 148, 237, 255 }); // Cornflower Blue

    Draw(m_Window, m_Original);
    Draw(m_Window, m_OriginalCaption);
    Draw(m_Window, m_Result);
    Draw(m_Window, m_ResultCaption);
    Draw(m_Window, m_Info);

    m_Window.display();

    return Continue;
  }

  virtual void PostShutdown() final override
  {
    mpLog::Info("The application ran for %f seconds.",
                (mpTime::Now() - m_Beginning).GetSeconds());
  }
};

MP_EntryClass(Main)
