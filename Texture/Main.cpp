#include "PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

#include "Drawing.h"
#include "Object.h"
#include "UserInput.h"

class Main : public mpApplication
{
  sf::RenderWindow  m_Window;
  mpTime            m_Beginning;
  UserInput         m_UserInput;

  Object m_Original;
  Object m_Result;

  /// CL Stuff
  mpPlatform m_Platform = mpPlatform::Get();
  mpDevice m_Device = mpDevice::GetGPU(m_Platform, 0);
  mpContext m_Context;
  mpCommandQueue m_Queue;
  mpProgram m_Program;
  mpKernel m_Kernel_BlendX;

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

    const float fMargin = 20.0f;
    const mpUInt32 uiNumTilingsX = 2;
    const mpUInt32 uiNumTilingsY = 2;

    sf::IntRect spriteRect;
    sf::Vector2u textureDimensions;

    // Init original image.
    {
      Initialize(m_Original, "Data/balls.png");
      TransformOf(m_Original).setPosition(fMargin, fMargin);

      // Tiling
      spriteRect = SpriteOf(m_Original).getTextureRect();
      spriteRect.width *= uiNumTilingsX; spriteRect.height *= uiNumTilingsY;

      textureDimensions = TextureOf(m_Original).getSize();

      SpriteOf(m_Original).setTextureRect(spriteRect);
    }

    // Init result image.
    {
      Initialize(m_Result, textureDimensions);
      TextureOf(m_Result).update(TextureOf(m_Original).copyToImage());
      TransformOf(m_Result).setPosition(fMargin + uiNumTilingsX * (float)textureDimensions.x + fMargin, fMargin);
      SpriteOf(m_Result).setTextureRect(spriteRect); // Tiling
    }

    Initialize(m_UserInput, "Data/Fonts/arial.ttf");
    //TextOf(m_UserInput) += "Choose Texture: ";

    // Initialize (create) window
    auto objectWidth = textureDimensions.x * uiNumTilingsX;
    auto objectHeight = textureDimensions.y * uiNumTilingsY;
    auto videoMode = sf::VideoMode{
      mpUInt32((fMargin + objectWidth) * 2 + fMargin),
      mpUInt32((fMargin + objectHeight)    + fMargin)
    };
    m_Window.create(videoMode, "Texture Blending");
  }

  void InvokeKernel(mpKernel& Kernel)
  {
    auto vPixelDimensions = TextureOf(m_Result).getSize();
    auto uiPixelCount = vPixelDimensions.x * vPixelDimensions.y;
    size_t GlobalWorkSize[2] = { vPixelDimensions.x, vPixelDimensions.y };

    auto Image = TextureOf(m_Result).copyToImage();
    auto Pixels = (cl_uchar4*)Image.getPixelsPtr();
    auto PixelsPtr = mpMakeArrayPtr(Pixels, uiPixelCount);

    // Copy pixels to GPU
    mpBuffer InputBuffer;
    InputBuffer.Initialize(m_Context, mpBufferFlags::ReadOnly, PixelsPtr);

    // Create buffer on GPU for the result
    mpBuffer ResultBuffer;
    ResultBuffer.Initialize(m_Context, mpBufferFlags::WriteOnly, uiPixelCount * sizeof(cl_uchar4));

    // Run Kernel with input and result buffer as arguments
    Kernel.PushArg(InputBuffer);
    Kernel.PushArg(ResultBuffer);
    Kernel.Execute(GlobalWorkSize);

    // Copy results from GPU back to CPU
    ResultBuffer.ReadInto(PixelsPtr, m_Queue);

    // Update the contents of the result texture with the output of the Kernel
    TextureOf(m_Result).update(Image);
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
      //InvokeKernel(m_Kernel_BlendY);
      break;
    case sf::Keyboard::Num3:
      //InvokeKernel(m_Kernel_Blend);
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

    m_Window.clear({ 100, 149, 237, 255 }); // Cornflower Blue (X11)

    Draw(m_Window, m_Original);
    Draw(m_Window, m_Result);
    //Draw(m_Window, m_UserInput);

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
