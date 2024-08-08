
#pragma once


#include <random>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>

#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlctl.h>
#include <commctrl.h>
#include <tchar.h>
#include <cstdlib>

#pragma comment(lib, "comctl32.lib")




//  Gdiplus provokes a few warnings.

#pragma warning(disable: 4458) //  declaration of 'foo' hides class member

//  Gdiplus wants a min and a max but we defined NOMINMAX, so give it the <algorithm> version instead.

namespace Gdiplus { using std::min; using std::max; };

//  Gdiplus also wants to overload operator new, so temporarily suspend what's defined in AtlDbgMem.h.

#pragma push_macro("new")
	#undef new
	#define GDIPVER 0x0110
	#include <gdiplus.h>	
#pragma pop_macro("new")

#pragma comment(lib, "gdiplus.lib")