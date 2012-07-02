#include "D3DApplication.h"

// Handle window messages
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	static D3DApplication* application			= NULL;
	
	if(message == WM_CREATE)
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)(lParam);
		application = (D3DApplication*)cs->lpCreateParams;
	}

	if(application)
		return application->HandleAppMessages(message, wParam, lParam);
	else
		// For all messages not handled above, call the default window procedure and return its return value
		return DefWindowProc(windowHandle, message, wParam, lParam);
}

// Constructor
D3DApplication::D3DApplication(HINSTANCE applicationInstance, LPCTSTR windowTitle, UINT windowWidth, UINT windowHeight)
	: mWindowHandle(NULL), mScreenWidth(windowWidth), mScreenHeight(windowHeight), mSwapChain(NULL), mRenderTarget(NULL),
		mDepthStencilBuffer(NULL), mDepthStencilView(NULL), mClearColor(0, 0, 0, 0), mDeviceD3D(NULL)
{
	// Try to create and initialize the application window, if failed - return with code 0
	if(!InitWindowsApplication(applicationInstance, windowTitle, SW_SHOW))
		exit(0);
	
	// Initialize Direct3D devices, if failed - return with code 0
	if(FAILED(InitializeDirect3D()))
		exit(0);
}

D3DApplication::~D3DApplication()
{
	mWindowHandle = NULL;
	mScreenWidth = 0;
	mScreenHeight = 0;
	SafeRelease(mDeviceD3D);
	SafeRelease(mSwapChain);
	SafeRelease(mRenderTarget);
	SafeRelease(mDepthStencilBuffer);
	SafeRelease(mDepthStencilView);
}

// Create an application window and initialize it returning true if success, else false
bool D3DApplication::InitWindowsApplication(HINSTANCE applicationHandle, LPCTSTR title, int showSetting)
{
	// Create a new window and specify its settings
	WNDCLASSEX window;

	window.cbSize			= sizeof(WNDCLASSEX);				// Size of the structure in bytes
	window.style			= CS_VREDRAW | CS_HREDRAW;			// When to redraw: at both vertical and horizontal resizing
	window.cbClsExtra		= 0;								// Extra memory slot, not needed
	window.cbWndExtra		= 0;								// Extra memory slot, not needed
	window.hInstance		= applicationHandle;				// Application handle
	window.hbrBackground	= (HBRUSH)(COLOR_WINDOWTEXT + 1);	// Window background color
	window.hCursor			= LoadCursor(0, IDC_ARROW);			// Window mouse cursor icon
	window.hIcon			= LoadIcon(0, IDI_APPLICATION);		// Window icon
	window.hIconSm			= 0;								// No small icon version is needed
	window.lpfnWndProc		= WindowProc;						// The window procedure to handle window messages
	window.lpszClassName	= "GameWindow";						// Name of the window, used for identification
	window.lpszMenuName		= 0;								// Since there are no menu for the window, value is 0

	// Try to register the window "mold" with windows: if failed, show an error message and return false
	if(!RegisterClassEx(&window))
	{
		ShowErrorMessage("Class registration failed!");
		return false;
	}

	// Try to create the window, if success the handle will be stored in gWindowHandle, if failed 0 will be stored
	mWindowHandle = CreateWindow("GameWindow",				// The name of the registered window "mold" to use
								title,						// Title of the window
								WS_OVERLAPPEDWINDOW,		// Window style
								CW_USEDEFAULT,				// X-position
								CW_USEDEFAULT,				// Y-position
								mScreenWidth,				// Window width
								mScreenHeight,				// Window height
								0,							// Parent window, 0 because there is no parent
								0,							// Menu handle, 0 because there is no menu
								applicationHandle,			// Application instance handle
								this);						// Extra parameter, not needed

	if(mWindowHandle == 0)		// Creation failed
	{
		ShowErrorMessage("Window creation failed!");
		return false;
	}

	// Show and update the created window and return true indicating successful creation
	ShowWindow(mWindowHandle, showSetting);
	UpdateWindow(mWindowHandle);

	CalculateWindowSize();

	return true;
}

// Initialize Direct3d devices, buffers etc
HRESULT D3DApplication::InitializeDirect3D()
{
	HRESULT result;

	result = CreateSwapChain();
	if(FAILED(result))
	{
		ShowErrorMessage("Swap chain creation failed!");
		return result;
	}

	return SetUpView();
}

// Update the window size variables based on the actual client area of the window
void D3DApplication::CalculateWindowSize()
{
	// Get and save the window size
	RECT clientRect;
	GetClientRect(mWindowHandle, &clientRect);
	mScreenWidth = clientRect.right - clientRect.left;
	mScreenHeight = clientRect.bottom - clientRect.top;
}

// Set up render target, depth and stencil buffers and viewport
HRESULT D3DApplication::SetUpView()
{
	HRESULT result = CreateRenderTargetView();
	if(FAILED(result))
	{
		ShowErrorMessage("Render target view creation failed!");
		return result;
	}

	result = CreateDepthStencil();
	if(FAILED(result))
	{
		ShowErrorMessage("Depth Stencil creation failed!");
		return result;
	}

	mDeviceD3D->OMSetRenderTargets(1,					// Only one render target needs to be bound
								&mRenderTarget,			// First element in an array of render targets to bind
								mDepthStencilView);		// The depth stencil view to bind

	SetUpViewport();

	return result;
}

// Run the application
int D3DApplication::Run()
{
	MSG messages			= {0};			// Initialize the message array to hold zero messages

	while(messages.message != WM_QUIT)		// Run program loop until the window
	{
		if(PeekMessage(&messages, 0, 0, 0, PM_REMOVE))	// Look at, and remove, next message,
		{												// returns whether there was a message
			TranslateMessage(&messages);	// Translate messages
			DispatchMessage(&messages);		// Dispatch message (sends it to the windowProc)
		}
		else
		{
			ProgramLoop();
		}
	}

	return (int)messages.wParam;
}

// Handle messages
LRESULT D3DApplication::HandleAppMessages(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:			// The window is destroyed
			PostQuitMessage(0);		// Post quit message (= WM_QUIT), will terminate the program loop
			return 0;
		case WM_SIZE:
			CalculateWindowSize();
			return 0;
		case WM_EXITSIZEMOVE:
			OnResize();
			return 0;
	}

	return DefWindowProc(mWindowHandle, message, wParam, lParam);
}

// Initialize and create the swap chain
HRESULT D3DApplication::CreateSwapChain()
{
	// Describe the swap chain
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferDesc.Width					= mScreenWidth;					// Width of the buffer is equal to the screen width
	swapDesc.BufferDesc.Height					= mScreenHeight;				// Height of the buffer is equal to the screen height
	swapDesc.BufferDesc.RefreshRate.Numerator	= 60;							// The refresh ratio is 60/1
	swapDesc.BufferDesc.RefreshRate.Denominator	= 1;							// The refresh ratio is 60/1
	swapDesc.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;	// Display format of the swap chain
	swapDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	swapDesc.SampleDesc.Count			= 1;									// 1 sample, i.e. no multisampling (no antialiasing)
	swapDesc.SampleDesc.Quality			= 0;									// Lowest image quality level (no antialiasing)

	swapDesc.BufferCount				= 1;									// Only one backbuffer - double buffering (one front buffer)
	swapDesc.BufferUsage				= DXGI_USAGE_RENDER_TARGET_OUTPUT;		// Use the buffer as a render target (render to it)
	swapDesc.Flags						= 0;									// No extra settings
	swapDesc.OutputWindow				= mWindowHandle;						// The window to render to - the application's window
	swapDesc.Windowed					= true;									// Run in windowed mode instead of full screen
	swapDesc.SwapEffect					= DXGI_SWAP_EFFECT_DISCARD;				// Discard buffer contents after presenting, driver chooses
																				// most efficient presentation technique
	// Create the swap chain
	UINT deviceCreationFlags			= 0;									// No flags needed for release builds
#ifdef _DEBUG
	deviceCreationFlags					|= D3D10_CREATE_DEVICE_DEBUG;			// If debug is set, enable debug messages	
#endif
	
	HRESULT result = D3D10CreateDeviceAndSwapChain(NULL,						// Adapter not specified - use primary display adapter
								D3D10_DRIVER_TYPE_HARDWARE,						// Use hardware acceleration for rendering
								NULL,											// No software rasterizer
								deviceCreationFlags,							// Flags for creation (0 or debug)
								D3D10_SDK_VERSION,								// Direct 3D version
								&swapDesc,										// Swap Chain description
								&mSwapChain,									// Swap Chain output
								&mDeviceD3D);									// Direct3D device output

	return result;
}

// Create a render target view and bind a back buffer to it
HRESULT D3DApplication::CreateRenderTargetView()
{
	HRESULT result					= S_OK;
	ID3D10Texture2D* backBuffer		= NULL;

	result = mSwapChain->GetBuffer(0,							// The index of the back buffer wanted
						__uuidof(ID3D10Texture2D),				// Interface type of the buffer
						reinterpret_cast<void**>(&backBuffer));	// A pointer to the back buffer, out value

	if (FAILED(result))
		return result;

	result = mDeviceD3D->CreateRenderTargetView(backBuffer,		// The resource to use as render target
									NULL,						// No other resource description needed
									&mRenderTarget);			// A pointer to the render target view, out value
	SafeRelease(backBuffer);
	return result;
}

// Create the depth stencil buffer and view, and bind to the output merger stage
HRESULT D3DApplication::CreateDepthStencil()
{
	HRESULT result						 = S_OK;

	// Describe the depth stencil
	D3D10_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width				= mScreenWidth;				// The width of the depth stencil texture
	depthStencilDesc.Height				= mScreenHeight;			// The height of the depth stencil texture
	depthStencilDesc.ArraySize			= 1;						// Only one texture in the texture array
	depthStencilDesc.BindFlags			= D3D10_BIND_DEPTH_STENCIL;	// The part of the pipeline to bind the resource to
	depthStencilDesc.CPUAccessFlags		= 0;						// Only the GPU will access the resource
	depthStencilDesc.Format				= DXGI_FORMAT_D32_FLOAT;	// Texel format
	depthStencilDesc.MipLevels			= 1;						// Number of mipmap-levels, depth stencil only need one
	depthStencilDesc.MiscFlags			= 0;						// No other flags needed
	depthStencilDesc.SampleDesc.Count	= 1;						// No multisampling (sample count is 1)
	depthStencilDesc.SampleDesc.Quality = 0;						// Lowest quality
	depthStencilDesc.Usage				= D3D10_USAGE_DEFAULT;		// How to use the texture - GPU read/write, CPU none

	result = mDeviceD3D->CreateTexture2D(&depthStencilDesc,			// The texture description
										0,							// Initial data in texture
										&mDepthStencilBuffer);		// Pointer to the created texture, out value
	if(FAILED(result))
		return result;

	result = mDeviceD3D->CreateDepthStencilView(mDepthStencilBuffer,	// Depth stencil surface (bind to depth stencil)
												0,				// Use the format the texture was created with
												&mDepthStencilView);	// Pointer to the depth stencil view, out value

	return result;
}

// Set up viewport and bind to the Direct3D device
void D3DApplication::SetUpViewport()
{
	D3D10_VIEWPORT viewportSettings;
	viewportSettings.TopLeftX		= 0;				// Relative to the client area, 0 to start to the left
	viewportSettings.TopLeftY		= 0;				// Relative to the client area, 0 to start at the top
	viewportSettings.Width			= mScreenWidth;		// Relative to the client area, same width means cover
	viewportSettings.Height			= mScreenHeight;	// Relative to the client area, same height means cover
	viewportSettings.MinDepth		= 0;				// Minimum depth buffer value (0 is standard)
	viewportSettings.MaxDepth		= 1;				// Maximum depth buffer value (1 is standard)

	mDeviceD3D->RSSetViewports(1,						// Number of viewports
							&viewportSettings);			// First element of array with viewports to bind
}

// Show a messagebox with the error message
void D3DApplication::ShowErrorMessage(LPCSTR message)
{
	MessageBox(0, message, "ERROR", 0);
}

// The program loop that is executed whenever there are no messages for the application
void D3DApplication::ProgramLoop()
{
	// Render application
	ClearScene();
	RenderScene();
}

// Clear the render target view
void D3DApplication::ClearScene()
{
	mDeviceD3D->ClearRenderTargetView(mRenderTarget, mClearColor);
	mDeviceD3D->ClearDepthStencilView(mDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);
}

// Show the drawn image on the screen (switch the front buffer and back buffer)
void D3DApplication::RenderScene()
{
	mSwapChain->Present(0, 0);
}

// Resizes the buffers and views to correspond to screen size in order to avoid stretching
void D3DApplication::OnResize()
{
	SafeRelease(mRenderTarget);
	SafeRelease(mDepthStencilBuffer);
	SafeRelease(mDepthStencilView);

	mSwapChain->ResizeBuffers(1, mScreenWidth, mScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	HRESULT result = SetUpView();
}

// Exit application
void D3DApplication::Quit()
{
	PostQuitMessage(0);
}