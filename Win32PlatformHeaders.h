#pragma once

#if defined(_WIN64)

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
//#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES
//#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
//#define NOKEYSTATES
#define NOSYSCOMMANDS
//#define NORASTEROPS
//#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
//#define NOGDI
#define NOKERNEL
//#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
//#define NOMETAFILE
//#define NOMINMAX
//#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
// WinUser.h defines.
#define NOAPISET
#define NOSYSPARAMSINFO
#define NOWINABLE
// Objidl.h defines.
#define COM_NO_WINDOWS_H
// rpc.h defines.
#define RPC_NO_WINDOWS_H

#define _AMD64_
#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>

#endif
