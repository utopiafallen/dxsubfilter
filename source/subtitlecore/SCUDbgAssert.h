// Defines a custom assertion macro
#ifndef SCUDBGASSERT_H
#define SCUDBGASSERT_H
#pragma once

#ifdef _DEBUG
extern bool DbgAssertFunction(bool expr, wchar_t* expr_string, wchar_t* desc, int line_num, wchar_t* file_name);

// These macros convert __FILE__ from char* to wchar_t*
#define DBG_WIDEN2(x) L##x
#define DBG_WIDEN(x) DBG_WIDEN2(x)
#define __WFILE__ DBG_WIDEN(__FILE__)

#define AssertMsg(expr, description) {if (DbgAssertFunction((expr), L#expr, L##description, __LINE__, __WFILE__)) {_asm{int 3}}}
#define Assert(expr) AssertMsg(!!(expr), #expr "was invalid.")
#define WhenAsserting(...) __VA_ARGS__
#else
#define WhenAsserting(...)
#define AssertMsg(expr, description)
#define Assert(expr)
#endif // _DEBUG

#endif // SCUDBGASSERT_H_
