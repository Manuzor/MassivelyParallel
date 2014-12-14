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

    // Initialize rendering stuff
    m_Window.create({ 800, 600 }, "Texture Blending");

    const float fTop = 20.0f;
    const float fLeft = fTop;
    const float fMargin = 20.0f;

    Initialize(m_Original, "Data/balls.png");
    TransformOf(m_Original).setPosition(fLeft, fTop);
    auto sizeOfOriginal = TextureOf(m_Original).getSize();

    Initialize(m_Result, sizeOfOriginal);
    TextureOf(m_Result).update(TextureOf(m_Original).copyToImage());
    TransformOf(m_Result).setPosition(fLeft + (float)sizeOfOriginal.x + fMargin, fTop);

    Initialize(m_UserInput, "Data/Fonts/arial.ttf");
    //TextOf(m_UserInput) += "Choose Texture: ";
  }

  void InvokeKernel()
  {
    auto size = TextureOf(m_Result).getSize();
    auto totalSize = size.x * size.y;
    size_t GlobalWorkSize[2] = { size.x, size.y };

    auto Image = TextureOf(m_Result).copyToImage();
    auto Pixels = (cl_uchar4*)Image.getPixelsPtr();
    auto PixelsPtr = mpMakeArrayPtr(Pixels, totalSize);

    // Set width and height parameters
    m_Kernel_BlendX.PushArg((cl_int)size.x);
    m_Kernel_BlendX.PushArg((cl_int)size.y);

    // Copy pixels to GPU
    mpBuffer Buffer;
    Buffer.Initialize(m_Context, mpBufferFlags::ReadWrite, PixelsPtr);
    m_Kernel_BlendX.PushArg(Buffer);

    // Run Kernel
    m_Kernel_BlendX.Execute(GlobalWorkSize);

    // Copy results from GPU back to CPU
    Buffer.ReadInto(PixelsPtr, m_Queue);

    // Update the contents of the result texture with the output of the Kernel
    TextureOf(m_Result).update(Image);
  }

  void HandleText(sf::Event::TextEvent& text)
  {
    if(text.unicode < 32)
      return;

    Append(m_UserInput, text.unicode);
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
      InvokeKernel();
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

    m_Window.clear({ 100, 149, 237, 255 });

    Draw(m_Window, m_Original);
    Draw(m_Window, m_Result);
    Draw(m_Window, m_UserInput);

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
