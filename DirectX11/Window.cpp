#include"Window.h"
#include<sstream>
#include"resource.h"
#include"WindowsThrowMacros.h"
#include "imgui/imgui_impl_win32.h"

Window::WindowClass Window::WindowClass::WndClass;

Window::WindowClass::WindowClass() noexcept
	:hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(
		GetInstance(),
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, 32, 32, 0
	));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage(
		GetInstance(),
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, 16, 16, 0
	));;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName()noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return WndClass.hInst;
}

Window::Window(int width, int height, const char* name)
	:width(width), height(height)
{
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	if (!AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE))
	{
		throw CHWND_LAST_EXCEPT();
	};
	hWnd = CreateWindow(
		WindowClass::GetName(), name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this
	);

	if (hWnd == nullptr)
	{
		throw CHWND_LAST_EXCEPT();
	}

	ShowWindow(hWnd, SW_SHOWDEFAULT);

	ImGui_ImplWin32_Init(hWnd);

	pGfx = std::make_unique<Graphics>(hWnd, width, height);
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
	{
		throw CHWND_LAST_EXCEPT();
	}
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return (int)msg.wParam;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}

Graphics& Window::Gfx()
{
	if (!pGfx)
		throw CHWND_NOGFX_EXCEPT();
	return *pGfx;
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (msg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
	const auto& imio = ImGui::GetIO();

	switch (msg)
	{
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}
	case WM_KILLFOCUS:
	{
		kbd.ClearState();
		break;
	}

	//Keybord msg
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
		{
			if (!(lParam & 0x40000000))
			{
				kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
			break;
		}
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
	    	break;
	    }
	case WM_CHAR:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	kbd.OnChar(static_cast<unsigned char>(wParam));
	    	break;
	    }
	//mouse msg
	case WM_MOUSEMOVE:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	mouse.OnMouseMove(pt.x, pt.y);
	    	break;
	    }
	case WM_LBUTTONDOWN:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	mouse.OnLeftPressed(pt.x, pt.y);
	    	break;
	    }
	case WM_LBUTTONUP:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	mouse.OnLeftReleased(pt.x, pt.y);
	    	SetForegroundWindow(hWnd);
	    	break;
	    }
	case WM_RBUTTONDOWN:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	mouse.OnRightPressed(pt.x, pt.y);
	    	break;
	    }
	case WM_RBUTTONUP:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	mouse.OnLeftReleased(pt.x, pt.y);
	    	break;
	    }
	case WM_MOUSEWHEEL:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
	    {
	    	const POINTS pt = MAKEPOINTS(lParam);
	    	if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
	    	{
	    		mouse.OnWheelUp(pt.x, pt.y);
	    	}
	    	else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
	    	{
	    		mouse.OnWheelDown(pt.x, pt.y);
	    	}
	    	break;
	    }
	//Msg handling over for mouse and keyboard
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:Exception(line, file), hr(hr)
{}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code]" << GetErrorCode() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Chili Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	if (nMsgLen == 0)
	{
		return"Unidentified error code";
	}
	std::string errorString = pMsgBuf;
	LocalFree(pMsgBuf);
	return errorString;
	return std::string();
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}

const char* Window::NoGfxException::GetType() const noexcept
{
	return "Chili Window Exception [No Graphics]";
}