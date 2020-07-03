#pragma once
#include"ChiliWin.h"
#include"ChiliException.h"
#include"Keyboard.h"
#include"Mouse.h"
#include"Graphics.h"
#include<optional>
#include<memory>

class Window
{
public:
	class Exception : public ChiliException
	{
		using ChiliException::ChiliException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Window::Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public Window::Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
private:
	class WindowClass
	{
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static constexpr const char* wndClassName = "Chili Direct3D Engine Window";
		static WindowClass WndClass;
		HINSTANCE hInst;
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	};
public:
	Window(int width, int height, const char* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessages() noexcept;
	Graphics& Gfx();
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	Keyboard kbd;
	Mouse mouse;

private:
	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
};

#define CHWND_EXCEPT(hr) Window::HrException(__LINE__, __FILE__, hr)
#define CHWND_LAST_EXCEPT() Window::HrException(__LINE__, __FILE__, GetLastError())
#define CHWND_NOGFX_EXCEPT() Window::NoGfxException(__LINE__,__FILE__)