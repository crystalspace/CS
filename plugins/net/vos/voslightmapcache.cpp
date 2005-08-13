#include "cssysdef.h"
#include "voslightmapcache.h"

SCF_IMPLEMENT_IBASE(LightmapCache)
  SCF_IMPLEMENTS_INTERFACE(iCacheManager)
SCF_IMPLEMENT_IBASE_END

using namespace VUtil;
using namespace VOS;

LightmapCache::LightmapCache(Property* target)
    : property(target, true)
{
}

void LightmapCache::SetCurrentType (const char* type)
{
    LOG("LightmapCache", 3, "SetCurrentType " << type);
}


const char* LightmapCache::GetCurrentType () const
{
    return "";
}

void LightmapCache::SetCurrentScope (const char* scope)
{
    if(scope) { LOG("LightmapCache", 3, "SetCurrentScope " << scope); }
    else { LOG("LightmapCache", 3, "SetCurrentScope NULL"); }
}

const char* LightmapCache::GetCurrentScope () const
{
    return "";
}

bool LightmapCache::CacheData (const void* data, size_t size, const char* type,
                               const char* scope, uint32 id)
{
    LOG("lightmapcache", 3, "caching " << size << " bytes");
    property->replace(std::string((char*)data, size));
    return true;
}

csPtr<iDataBuffer> LightmapCache::ReadCache (const char* type,
                                             const char* scope,
                                             uint32 id)
{
    std::string s = property->read();
    char* m = new char[s.size()];
    memcpy(m, s.c_str(), s.size());

    LOG("lightmapcache", 3, "reading cache " << s.size() << " bytes");

    return csPtr<iDataBuffer>(new csDataBuffer(m, s.size()));
}

bool LightmapCache::ClearCache (const char* type, const char* scope,
                                const uint32* id)
{
    return true;
}

void LightmapCache::Flush()
{
}

