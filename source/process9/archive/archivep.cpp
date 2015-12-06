#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

Archive::Archive(Process9* owner, LowPath *lowpath) :m_owner(owner), m_lowpath(*lowpath)
{
    LOG("path: type = %s, str = %s, size = %d", lowpath->TypeToString(), lowpath->GetPath().c_str(), lowpath->GetSize());
}

Archive::~Archive()
{
}
