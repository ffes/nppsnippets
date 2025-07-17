// This file is part of Notepad++ project
// Copyright (c) 2021 adzm / Adam D. Walling

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <commctrl.h>
#include <commdlg.h>

#include "NppDarkMode.h"
#include "DarkMode.h"

#include <iostream>
#include <vector>

#ifdef __MINGW64__
#include <cmath>
#endif

#if defined(__GNUC__) && __GNUC__ > 8
#define WINAPI_LAMBDA [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) -> LRESULT WINAPI
#elif defined(__GNUC__)
#define WINAPI_LAMBDA [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) WINAPI -> LRESULT
#else
#define WINAPI_LAMBDA [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) -> LRESULT
#endif

namespace NppDarkMode
{
   struct Brushes
   {
      HBRUSH background = nullptr;
      HBRUSH ctrlBackground = nullptr;
      HBRUSH hotBackground = nullptr;
      HBRUSH dlgBackground = nullptr;
      HBRUSH errorBackground = nullptr;
      HBRUSH edgeBrush = nullptr;
      HBRUSH hotEdgeBrush = nullptr;
      HBRUSH disabledEdgeBrush = nullptr;

      Brushes(const Colors& colors)
      {
         change(colors);
      }

      ~Brushes()
      {
         ::DeleteObject(background);
         ::DeleteObject(ctrlBackground);
         ::DeleteObject(hotBackground);
         ::DeleteObject(dlgBackground);
         ::DeleteObject(errorBackground);
         ::DeleteObject(edgeBrush);
         ::DeleteObject(hotEdgeBrush);
         ::DeleteObject(disabledEdgeBrush);
      }

      void change(const Colors& colors)
      {
         ::DeleteObject(background);
         ::DeleteObject(ctrlBackground);
         ::DeleteObject(hotBackground);
         ::DeleteObject(dlgBackground);
         ::DeleteObject(errorBackground);
         ::DeleteObject(edgeBrush);
         ::DeleteObject(hotEdgeBrush);
         ::DeleteObject(disabledEdgeBrush);

         background = ::CreateSolidBrush(colors.background);
         ctrlBackground = ::CreateSolidBrush(colors.softerBackground);
         hotBackground = ::CreateSolidBrush(colors.hotBackground);
         dlgBackground = ::CreateSolidBrush(colors.pureBackground);
         errorBackground = ::CreateSolidBrush(colors.errorBackground);
         edgeBrush = ::CreateSolidBrush(colors.edge);
         hotEdgeBrush = ::CreateSolidBrush(colors.hotEdge);
         disabledEdgeBrush = ::CreateSolidBrush(colors.disabledEdge);
      }
   };

   struct Pens
   {
      HPEN darkerTextPen = nullptr;
      HPEN edgePen = nullptr;
      HPEN hotEdgePen = nullptr;
      HPEN disabledEdgePen = nullptr;

      Pens(const Colors& colors)
      {
         change(colors);
      }

      ~Pens()
      {
         ::DeleteObject(darkerTextPen);
         ::DeleteObject(edgePen);
         ::DeleteObject(hotEdgePen);
         ::DeleteObject(disabledEdgePen);
      }

      void change(const Colors& colors)
      {
         ::DeleteObject(darkerTextPen);
         ::DeleteObject(edgePen);
         ::DeleteObject(hotEdgePen);
         ::DeleteObject(disabledEdgePen);

         darkerTextPen = ::CreatePen(PS_SOLID, 1, colors.darkerText);
         edgePen = ::CreatePen(PS_SOLID, 1, colors.edge);
         hotEdgePen = ::CreatePen(PS_SOLID, 1, colors.hotEdge);
         disabledEdgePen = ::CreatePen(PS_SOLID, 1, colors.disabledEdge);
      }
   };

   // Default dark color scheme
   static const Colors darkColors{
      HEXRGB(0x202020), // background
      HEXRGB(0x404040), // softerBackground
      HEXRGB(0x404040), // hotBackground
      HEXRGB(0x202020), // pureBackground
      HEXRGB(0xB00000), // errorBackground
      HEXRGB(0xE0E0E0), // textColor
      HEXRGB(0xC0C0C0), // darkerTextColor
      HEXRGB(0x808080), // disabledTextColor
      HEXRGB(0xFFFF00), // linkTextColor
      HEXRGB(0x646464), // edgeColor
      HEXRGB(0x9B9B9B), // hotEdgeColor
      HEXRGB(0x484848)  // disabledEdgeColor
   };

   struct Theme
   {
      Colors _colors;
      Brushes _brushes;
      Pens _pens;

      Theme(const Colors& colors)
         : _colors(colors)
         , _brushes(colors)
         , _pens(colors)
      {}

      void change(const Colors& colors)
      {
         _colors = colors;
         _brushes.change(colors);
         _pens.change(colors);
      }
   };

   Theme tCurrent(darkColors);
   static boolean _isDarkModeEnabled = false;

   void initDarkMode()
   {
      ::InitDarkMode();
      _isDarkModeEnabled = false; // Start with false, call setDarkMode to enable
   }

   void setDarkMode(bool useDarkMode)
   {
      ::SetDarkMode(useDarkMode, true);
      // Force dark mode regardless of system settings
      _isDarkModeEnabled = useDarkMode;
   }

   bool isEnabled()
   {
      return _isDarkModeEnabled;
   }

   bool isWindows10()
   {
      return ::IsWindows10();
   }

   bool isWindows11()
   {
      return ::IsWindows11();
   }

   DWORD getWindowsBuildNumber()
   {
      return ::GetWindowsBuildNumber();
   }

   COLORREF getBackgroundColor()         { return tCurrent._colors.background; }
   COLORREF getCtrlBackgroundColor()     { return tCurrent._colors.softerBackground; }
   COLORREF getHotBackgroundColor()      { return tCurrent._colors.hotBackground; }
   COLORREF getDlgBackgroundColor()      { return tCurrent._colors.pureBackground; }
   COLORREF getErrorBackgroundColor()    { return tCurrent._colors.errorBackground; }
   COLORREF getTextColor()               { return tCurrent._colors.text; }
   COLORREF getDarkerTextColor()         { return tCurrent._colors.darkerText; }
   COLORREF getDisabledTextColor()       { return tCurrent._colors.disabledText; }
   COLORREF getLinkTextColor()           { return tCurrent._colors.linkText; }
   COLORREF getEdgeColor()               { return tCurrent._colors.edge; }
   COLORREF getHotEdgeColor()            { return tCurrent._colors.hotEdge; }
   COLORREF getDisabledEdgeColor()       { return tCurrent._colors.disabledEdge; }

   HBRUSH getBackgroundBrush()           { return tCurrent._brushes.background; }
   HBRUSH getCtrlBackgroundBrush()       { return tCurrent._brushes.ctrlBackground; }
   HBRUSH getHotBackgroundBrush()        { return tCurrent._brushes.hotBackground; }
   HBRUSH getDlgBackgroundBrush()        { return tCurrent._brushes.dlgBackground; }
   HBRUSH getErrorBackgroundBrush()      { return tCurrent._brushes.errorBackground; }

   HBRUSH getEdgeBrush()                 { return tCurrent._brushes.edgeBrush; }
   HBRUSH getHotEdgeBrush()              { return tCurrent._brushes.hotEdgeBrush; }
   HBRUSH getDisabledEdgeBrush()         { return tCurrent._brushes.disabledEdgeBrush; }

   HPEN getDarkerTextPen()               { return tCurrent._pens.darkerTextPen; }
   HPEN getEdgePen()                     { return tCurrent._pens.edgePen; }
   HPEN getHotEdgePen()                  { return tCurrent._pens.hotEdgePen; }
   HPEN getDisabledEdgePen()             { return tCurrent._pens.disabledEdgePen; }

   // from DarkMode.h
   void allowDarkModeForApp(bool allow)
   {
      ::AllowDarkModeForApp(allow);
   }

   bool allowDarkModeForWindow(HWND hWnd, bool allow)
   {
      return ::AllowDarkModeForWindow(hWnd, allow);
   }

   void setTitleBarThemeColor(HWND hWnd)
   {
      ::RefreshTitleBarThemeColor(hWnd);
   }

   // enhancements to DarkMode.h
   void enableDarkScrollBarForWindowAndChildren(HWND hwnd)
   {
      ::EnableDarkScrollBarForWindowAndChildren(hwnd);
   }

   inline void paintRoundFrameRect(HDC hdc, const RECT rect, const HPEN hpen, int width, int height)
   {
      auto holdBrush = ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
      auto holdPen = ::SelectObject(hdc, hpen);
      ::RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, width, height);
      ::SelectObject(hdc, holdBrush);
      ::SelectObject(hdc, holdPen);
   }

   // Button subclassing
   constexpr UINT_PTR g_buttonSubclassID = 42;

   struct ButtonData
   {
      bool isHot = false;
      bool isFocused = false;
      bool isPressed = false;
   };

   static LRESULT CALLBACK ButtonSubclass(
      HWND hWnd,
      UINT uMsg,
      WPARAM wParam,
      LPARAM lParam,
      UINT_PTR uIdSubclass,
      DWORD_PTR dwRefData
   )
   {
      auto pButtonData = reinterpret_cast<ButtonData*>(dwRefData);

      switch (uMsg)
      {
      case WM_MOUSEMOVE:
      case WM_MOUSELEAVE:
      {
         pButtonData->isHot = (uMsg == WM_MOUSEMOVE);
         ::InvalidateRect(hWnd, nullptr, FALSE);
         break;
      }

      case WM_PAINT:
      {
         if (!isEnabled())
         {
            break;
         }

         PAINTSTRUCT ps;
         HDC hdc = ::BeginPaint(hWnd, &ps);

         RECT rcClient{};
         ::GetClientRect(hWnd, &rcClient);

         bool isDisabled = !::IsWindowEnabled(hWnd);
         bool isPressed = (::GetKeyState(VK_LBUTTON) & 0x8000) && pButtonData->isHot;

         COLORREF bgColor = isPressed ? getHotBackgroundColor() : 
                           pButtonData->isHot ? getCtrlBackgroundColor() : 
                           getBackgroundColor();
         
         HBRUSH bgBrush = ::CreateSolidBrush(bgColor);
         ::FillRect(hdc, &rcClient, bgBrush);
         ::DeleteObject(bgBrush);

         // Draw border
         HPEN borderPen = ::CreatePen(PS_SOLID, 1, pButtonData->isHot ? getHotEdgeColor() : getEdgeColor());
         paintRoundFrameRect(hdc, rcClient, borderPen, 2, 2);
         ::DeleteObject(borderPen);

         // Draw text
         ::SetBkMode(hdc, TRANSPARENT);
         ::SetTextColor(hdc, isDisabled ? getDisabledTextColor() : getTextColor());

         wchar_t buttonText[256];
         ::GetWindowText(hWnd, buttonText, 256);

         ::DrawText(hdc, buttonText, -1, &rcClient, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

         ::EndPaint(hWnd, &ps);
         return 0;
      }

      case WM_NCDESTROY:
      {
         ::RemoveWindowSubclass(hWnd, ButtonSubclass, uIdSubclass);
         delete pButtonData;
         break;
      }
      }

      return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
   }

   void subclassButtonControl(HWND hwnd)
   {
      DWORD_PTR pButtonData = reinterpret_cast<DWORD_PTR>(new ButtonData());
      SetWindowSubclass(hwnd, ButtonSubclass, g_buttonSubclassID, pButtonData);
   }

   // ComboBox subclassing - minimal implementation
   constexpr UINT_PTR g_comboBoxSubclassID = 43;

   static LRESULT CALLBACK ComboBoxSubclass(
      HWND hWnd,
      UINT uMsg,
      WPARAM wParam,
      LPARAM lParam,
      UINT_PTR uIdSubclass,
      DWORD_PTR dwRefData
   )
   {
      switch (uMsg)
      {
      case WM_NCDESTROY:
      {
         ::RemoveWindowSubclass(hWnd, ComboBoxSubclass, uIdSubclass);
         break;
      }
      }

      return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
   }

   void subclassComboBoxControl(HWND hwnd)
   {
      SetWindowSubclass(hwnd, ComboBoxSubclass, g_comboBoxSubclassID, 0);
   }

   // GroupBox subclassing - minimal implementation
   constexpr UINT_PTR g_groupboxSubclassID = 44;

   static LRESULT CALLBACK GroupboxSubclass(
      HWND hWnd,
      UINT uMsg,
      WPARAM wParam,
      LPARAM lParam,
      UINT_PTR uIdSubclass,
      DWORD_PTR dwRefData
   )
   {
      switch (uMsg)
      {
      case WM_NCDESTROY:
      {
         ::RemoveWindowSubclass(hWnd, GroupboxSubclass, uIdSubclass);
         break;
      }
      }

      return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
   }

   void subclassGroupboxControl(HWND hwnd)
   {
      SetWindowSubclass(hwnd, GroupboxSubclass, g_groupboxSubclassID, 0);
   }

   // Helper function to get window class name
   std::wstring getWndClassName(HWND hwnd)
   {
      const size_t classNameLen = 16;
      TCHAR className[classNameLen];
      GetClassName(hwnd, className, classNameLen);
      return std::wstring(className);
   }

   void autoSubclassAndThemeChildControls(HWND hwndParent, bool subclass, bool theme)
   {
      struct Params
      {
         const wchar_t* themeClassName = nullptr;
         bool subclass = false;
         bool theme = false;
      };

      Params p{
         isEnabled() ? L"DarkMode_Explorer" : nullptr
         , subclass
         , theme
      };

      ::EnableThemeDialogTexture(hwndParent, theme && !isEnabled() ? ETDT_ENABLETAB : ETDT_DISABLE);

      EnumChildWindows(hwndParent, [](HWND hwnd, LPARAM lParam) -> BOOL {
         auto& p = *reinterpret_cast<Params*>(lParam);
         const size_t classNameLen = 16;
         TCHAR className[classNameLen] {};
         GetClassName(hwnd, className, classNameLen);

         if (wcscmp(className, WC_COMBOBOX) == 0)
         {
            auto style = ::GetWindowLongPtr(hwnd, GWL_STYLE);

            if ((style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST || (style & CBS_DROPDOWN) == CBS_DROPDOWN)
            {
               COMBOBOXINFO cbi = {};
               cbi.cbSize = sizeof(COMBOBOXINFO);
               if (GetComboBoxInfo(hwnd, &cbi) == TRUE)
               {
                  if (p.theme && cbi.hwndList)
                  {
                     //dark scrollbar for listbox of combobox
                     SetWindowTheme(cbi.hwndList, p.themeClassName, nullptr);
                  }
               }

               if (p.subclass)
               {
                  HWND hParent = ::GetParent(hwnd);
                  if ((hParent == nullptr || getWndClassName(hParent) != WC_COMBOBOXEX))
                  {
                     subclassComboBoxControl(hwnd);
                  }
               }
            }
            return TRUE;
         }

         if (wcscmp(className, WC_LISTBOX) == 0)
         {
            if (p.theme)
            {
               //dark scrollbar for listbox
               SetWindowTheme(hwnd, p.themeClassName, nullptr);
            }
            return TRUE;
         }

         if (wcscmp(className, WC_EDIT) == 0)
         {
            auto style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
            bool hasScrollBar = ((style & WS_HSCROLL) == WS_HSCROLL) || ((style & WS_VSCROLL) == WS_VSCROLL);
            if (p.theme && hasScrollBar)
            {
               //dark scrollbar for edit control
               SetWindowTheme(hwnd, p.themeClassName, nullptr);
            }
            return TRUE;
         }

         if (wcscmp(className, WC_BUTTON) == 0)
         {
            auto nButtonStyle = ::GetWindowLongPtr(hwnd, GWL_STYLE);
            switch (nButtonStyle & BS_TYPEMASK)
            {
               case BS_CHECKBOX:
               case BS_AUTOCHECKBOX:
               case BS_3STATE:
               case BS_AUTO3STATE:
               case BS_RADIOBUTTON:
               case BS_AUTORADIOBUTTON:
               {
                  if ((nButtonStyle & BS_PUSHLIKE) == BS_PUSHLIKE)
                  {
                     if (p.theme)
                     {
                        SetWindowTheme(hwnd, p.themeClassName, nullptr);
                     }
                  }

                  if (p.subclass)
                  {
                     subclassButtonControl(hwnd);
                  }
                  break;
               }

               case BS_GROUPBOX:
               {
                  if (p.subclass)
                  {
                     subclassGroupboxControl(hwnd);
                  }
                  break;
               }

               case BS_PUSHBUTTON:
               case BS_DEFPUSHBUTTON:
               case BS_SPLITBUTTON:
               case BS_DEFSPLITBUTTON:
               {
                  if (p.theme)
                  {
                     SetWindowTheme(hwnd, p.themeClassName, nullptr);
                  }
                  break;
               }

               default:
               {
                  break;
               }
            }
            return TRUE;
         }

         return TRUE;
      }, reinterpret_cast<LPARAM>(&p));
   }

   void autoThemeChildControls(HWND hwndParent)
   {
      autoSubclassAndThemeChildControls(hwndParent, false, true);
   }

   void setDarkTitleBar(HWND hwnd)
   {
      allowDarkModeForWindow(hwnd, isEnabled());
      setTitleBarThemeColor(hwnd);
   }

   void setDarkScrollBar(HWND hwnd)
   {
      if (isEnabled())
      {
         enableDarkScrollBarForWindowAndChildren(hwnd);
      }
   }

   void disableVisualStyle(HWND hwnd, bool doDisable)
   {
      if (doDisable)
      {
         SetWindowTheme(hwnd, L"", L"");
      }
      else
      {
         SetWindowTheme(hwnd, nullptr, nullptr);
      }
   }

   void setBorder(HWND hwnd, bool border)
   {
      auto currentStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
      auto newStyle = border ? 
         (currentStyle | WS_EX_CLIENTEDGE) : 
         (currentStyle & ~WS_EX_CLIENTEDGE);

      if (newStyle != currentStyle)
      {
         ::SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyle);
         ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
      }
   }

   LRESULT onCtlColor(HDC hdc)
   {
      ::SetTextColor(hdc, getTextColor());
      ::SetBkColor(hdc, getBackgroundColor());
      return reinterpret_cast<LRESULT>(getBackgroundBrush());
   }

   LRESULT onCtlColorSofter(HDC hdc)
   {
      ::SetTextColor(hdc, getTextColor());
      ::SetBkColor(hdc, getCtrlBackgroundColor());
      return reinterpret_cast<LRESULT>(getCtrlBackgroundBrush());
   }

   LRESULT onCtlColorDarker(HDC hdc)
   {
      ::SetTextColor(hdc, getTextColor());
      ::SetBkColor(hdc, getDlgBackgroundColor());
      return reinterpret_cast<LRESULT>(getDlgBackgroundBrush());
   }

   LRESULT onCtlColorError(HDC hdc)
   {
      ::SetTextColor(hdc, getTextColor());
      ::SetBkColor(hdc, getErrorBackgroundColor());
      return reinterpret_cast<LRESULT>(getErrorBackgroundBrush());
   }

   LRESULT onCtlColorIfEnabled(HDC hdc, bool bEnabled)
   {
      LRESULT result{ FALSE };

      if (isEnabled()) {
         result = onCtlColorDarker(hdc);
         SetTextColor(hdc, bEnabled ? getTextColor() : getDisabledTextColor());
      }
      else {
         SetTextColor(hdc, GetSysColor(bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
      }

      return result;
   }

   INT_PTR onCtlColorListbox(WPARAM wParam, LPARAM lParam)
   {
      auto hdc = reinterpret_cast<HDC>(wParam);
      auto hwnd = reinterpret_cast<HWND>(lParam);

      auto style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
      bool isComboBox = (style & LBS_COMBOBOX) == LBS_COMBOBOX;
      if (!isComboBox && ::IsWindowEnabled(hwnd))
      {
         return static_cast<INT_PTR>(onCtlColorSofter(hdc));
      }
      return static_cast<INT_PTR>(onCtlColor(hdc));
   }
}