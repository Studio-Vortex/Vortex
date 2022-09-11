#pragma once

namespace Sparky
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space                            = 32,
		Apostrophe                       = 39, /* ' */
		Comma                            = 44, /* , */
		Minus                            = 45, /* - */
		Period                           = 46, /* . */
		Slash                            = 47, /* / */

		D0                               = 48, /* 0 */
		D1                               = 49, /* 1 */
		D2                               = 50, /* 2 */
		D3                               = 51, /* 3 */
		D4                               = 52, /* 4 */
		D5                               = 53, /* 5 */
		D6                               = 54, /* 6 */
		D7                               = 55, /* 7 */
		D8                               = 56, /* 8 */
		D9                               = 57, /* 9 */

		Semicolon                        = 59, /* ; */
		Equal                            = 61, /* = */
		
		A                                = 65,
		B                                = 66,
		C                                = 67,
		D                                = 68,
		E                                = 69,
		F                                = 70,
		G                                = 71,
		H                                = 72,
		I                                = 73,
		J                                = 74,
		K                                = 75,
		L                                = 76,
		M                                = 77,
		N                                = 78,
		O                                = 79,
		P                                = 80,
		Q                                = 81,
		R                                = 82,
		S                                = 83,
		T                                = 84,
		U                                = 85,
		V                                = 86,
		W                                = 87,
		X                                = 88,
		Y                                = 89,
		Z                                = 90,

		LeftBracket                      = 91,  /* [ */
		Backslash                        = 92,  /* \ */
		RightBracket                     = 93,  /* ] */
		GraveAccent                      = 96,  /* ` */

		World1                           = 161, /* non-US #1 */
		World2                           = 162, /* non-US #2 */

		/* Function keys */
		Escape                           = 256,
		Enter                            = 257,
		Tab                              = 258,
		Backspace                        = 259,
		Insert                           = 260,
		Delete                           = 261,
		Right                            = 262,
		Left                             = 263,
		Down                             = 264,
		Up                               = 265,
		PageUp                           = 266,
		PageDown                         = 267,
		Home                             = 268,
		End                              = 269,
		CapsLock                         = 280,
		ScrollLock                       = 281,
		NumLock                          = 282,
		PrintScreen                      = 283,
		Pause                            = 284,
		F1                               = 290,
		F2                               = 291,
		F3                               = 292,
		F4                               = 293,
		F5                               = 294,
		F6                               = 295,
		F7                               = 296,
		F8                               = 297,
		F9                               = 298,
		F10                              = 299,
		F11                              = 300,
		F12                              = 301,
		F13                              = 302,
		F14                              = 303,
		F15                              = 304,
		F16                              = 305,
		F17                              = 306,
		F18                              = 307,
		F19                              = 308,
		F20                              = 309,
		F21                              = 310,
		F22                              = 311,
		F23                              = 312,
		F24                              = 313,
		F25                              = 314,

		/* Keypad */
		KP0                              = 320,
		KP1                              = 321,
		KP2                              = 322,
		KP3                              = 323,
		KP4                              = 324,
		KP5                              = 325,
		KP6                              = 326,
		KP7                              = 327,
		KP8                              = 328,
		KP9                              = 329,
		KPDecimal                        = 330,
		KPDivide                         = 331,
		KPMultiply                       = 332,
		KPSubtract                       = 333,
		KPAdd                            = 334,
		KPEnter                          = 335,
		KPEqual                          = 336,
		LeftShift                        = 340,
		LeftControl                      = 341,
		LeftAlt                          = 342,
		LeftSuper                        = 343,
		RightShift                       = 344,
		RightControl                     = 345,
		RightAlt                         = 346,
		RightSuper                       = 347,
		Menu                             = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define SP_KEY_SPACE           ::Sparky::Key::Space
#define SP_KEY_APOSTROPHE      ::Sparky::Key::Apostrophe    /* ' */
#define SP_KEY_COMMA           ::Sparky::Key::Comma         /* , */
#define SP_KEY_MINUS           ::Sparky::Key::Minus         /* - */
#define SP_KEY_PERIOD          ::Sparky::Key::Period        /* . */
#define SP_KEY_SLASH           ::Sparky::Key::Slash         /* / */
#define SP_KEY_0               ::Sparky::Key::D0
#define SP_KEY_1               ::Sparky::Key::D1
#define SP_KEY_2               ::Sparky::Key::D2
#define SP_KEY_3               ::Sparky::Key::D3
#define SP_KEY_4               ::Sparky::Key::D4
#define SP_KEY_5               ::Sparky::Key::D5
#define SP_KEY_6               ::Sparky::Key::D6
#define SP_KEY_7               ::Sparky::Key::D7
#define SP_KEY_8               ::Sparky::Key::D8
#define SP_KEY_9               ::Sparky::Key::D9
#define SP_KEY_SEMICOLON       ::Sparky::Key::Semicolon     /* ; */
#define SP_KEY_EQUAL           ::Sparky::Key::Equal         /* = */
#define SP_KEY_A               ::Sparky::Key::A
#define SP_KEY_B               ::Sparky::Key::B
#define SP_KEY_C               ::Sparky::Key::C
#define SP_KEY_D               ::Sparky::Key::D
#define SP_KEY_E               ::Sparky::Key::E
#define SP_KEY_F               ::Sparky::Key::F
#define SP_KEY_G               ::Sparky::Key::G
#define SP_KEY_H               ::Sparky::Key::H
#define SP_KEY_I               ::Sparky::Key::I
#define SP_KEY_J               ::Sparky::Key::J
#define SP_KEY_K               ::Sparky::Key::K
#define SP_KEY_L               ::Sparky::Key::L
#define SP_KEY_M               ::Sparky::Key::M
#define SP_KEY_N               ::Sparky::Key::N
#define SP_KEY_O               ::Sparky::Key::O
#define SP_KEY_P               ::Sparky::Key::P
#define SP_KEY_Q               ::Sparky::Key::Q
#define SP_KEY_R               ::Sparky::Key::R
#define SP_KEY_S               ::Sparky::Key::S
#define SP_KEY_T               ::Sparky::Key::T
#define SP_KEY_U               ::Sparky::Key::U
#define SP_KEY_V               ::Sparky::Key::V
#define SP_KEY_W               ::Sparky::Key::W
#define SP_KEY_X               ::Sparky::Key::X
#define SP_KEY_Y               ::Sparky::Key::Y
#define SP_KEY_Z               ::Sparky::Key::Z
#define SP_KEY_LEFT_BRACKET    ::Sparky::Key::LeftBracket   /* [ */
#define SP_KEY_BACKSLASH       ::Sparky::Key::Backslash     /* \ */
#define SP_KEY_RIGHT_BRACKET   ::Sparky::Key::RightBracket  /* ] */
#define SP_KEY_GRAVE_ACCENT    ::Sparky::Key::GraveAccent   /* ` */
#define SP_KEY_WORLD_1         ::Sparky::Key::World1        /* non-US #1 */
#define SP_KEY_WORLD_2         ::Sparky::Key::World2        /* non-US #2 */

/* Function keys */
#define SP_KEY_ESCAPE          ::Sparky::Key::Escape
#define SP_KEY_ENTER           ::Sparky::Key::Enter
#define SP_KEY_TAB             ::Sparky::Key::Tab
#define SP_KEY_BACKSPACE       ::Sparky::Key::Backspace
#define SP_KEY_INSERT          ::Sparky::Key::Insert
#define SP_KEY_DELETE          ::Sparky::Key::Delete
#define SP_KEY_RIGHT           ::Sparky::Key::Right
#define SP_KEY_LEFT            ::Sparky::Key::Left
#define SP_KEY_DOWN            ::Sparky::Key::Down
#define SP_KEY_UP              ::Sparky::Key::Up
#define SP_KEY_PAGE_UP         ::Sparky::Key::PageUp
#define SP_KEY_PAGE_DOWN       ::Sparky::Key::PageDown
#define SP_KEY_HOME            ::Sparky::Key::Home
#define SP_KEY_END             ::Sparky::Key::End
#define SP_KEY_CAPS_LOCK       ::Sparky::Key::CapsLock
#define SP_KEY_SCROLL_LOCK     ::Sparky::Key::ScrollLock
#define SP_KEY_NUM_LOCK        ::Sparky::Key::NumLock
#define SP_KEY_PRINT_SCREEN    ::Sparky::Key::PrintScreen
#define SP_KEY_PAUSE           ::Sparky::Key::Pause
#define SP_KEY_F1              ::Sparky::Key::F1
#define SP_KEY_F2              ::Sparky::Key::F2
#define SP_KEY_F3              ::Sparky::Key::F3
#define SP_KEY_F4              ::Sparky::Key::F4
#define SP_KEY_F5              ::Sparky::Key::F5
#define SP_KEY_F6              ::Sparky::Key::F6
#define SP_KEY_F7              ::Sparky::Key::F7
#define SP_KEY_F8              ::Sparky::Key::F8
#define SP_KEY_F9              ::Sparky::Key::F9
#define SP_KEY_F10             ::Sparky::Key::F10
#define SP_KEY_F11             ::Sparky::Key::F11
#define SP_KEY_F12             ::Sparky::Key::F12
#define SP_KEY_F13             ::Sparky::Key::F13
#define SP_KEY_F14             ::Sparky::Key::F14
#define SP_KEY_F15             ::Sparky::Key::F15
#define SP_KEY_F16             ::Sparky::Key::F16
#define SP_KEY_F17             ::Sparky::Key::F17
#define SP_KEY_F18             ::Sparky::Key::F18
#define SP_KEY_F19             ::Sparky::Key::F19
#define SP_KEY_F20             ::Sparky::Key::F20
#define SP_KEY_F21             ::Sparky::Key::F21
#define SP_KEY_F22             ::Sparky::Key::F22
#define SP_KEY_F23             ::Sparky::Key::F23
#define SP_KEY_F24             ::Sparky::Key::F24
#define SP_KEY_F25             ::Sparky::Key::F25

/* Keypad */
#define SP_KEY_KP_0            ::Sparky::Key::KP0
#define SP_KEY_KP_1            ::Sparky::Key::KP1
#define SP_KEY_KP_2            ::Sparky::Key::KP2
#define SP_KEY_KP_3            ::Sparky::Key::KP3
#define SP_KEY_KP_4            ::Sparky::Key::KP4
#define SP_KEY_KP_5            ::Sparky::Key::KP5
#define SP_KEY_KP_6            ::Sparky::Key::KP6
#define SP_KEY_KP_7            ::Sparky::Key::KP7
#define SP_KEY_KP_8            ::Sparky::Key::KP8
#define SP_KEY_KP_9            ::Sparky::Key::KP9
#define SP_KEY_KP_DECIMAL      ::Sparky::Key::KPDecimal
#define SP_KEY_KP_DIVIDE       ::Sparky::Key::KPDivide
#define SP_KEY_KP_MULTIPLY     ::Sparky::Key::KPMultiply
#define SP_KEY_KP_SUBTRACT     ::Sparky::Key::KPSubtract
#define SP_KEY_KP_ADD          ::Sparky::Key::KPAdd
#define SP_KEY_KP_ENTER        ::Sparky::Key::KPEnter
#define SP_KEY_KP_EQUAL        ::Sparky::Key::KPEqual

#define SP_KEY_LEFT_SHIFT      ::Sparky::Key::LeftShift
#define SP_KEY_LEFT_CONTROL    ::Sparky::Key::LeftControl
#define SP_KEY_LEFT_ALT        ::Sparky::Key::LeftAlt
#define SP_KEY_LEFT_SUPER      ::Sparky::Key::LeftSuper
#define SP_KEY_RIGHT_SHIFT     ::Sparky::Key::RightShift
#define SP_KEY_RIGHT_CONTROL   ::Sparky::Key::RightControl
#define SP_KEY_RIGHT_ALT       ::Sparky::Key::RightAlt
#define SP_KEY_RIGHT_SUPER     ::Sparky::Key::RightSuper
#define SP_KEY_MENU            ::Sparky::Key::Menu