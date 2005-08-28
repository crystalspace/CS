#ifndef _LIGHTMAPCACHE_H_
#define _LIGHTMAPCACHE_H_

#include <vos/metaobjects/property/property.hh>
#include <iutil/cache.h>
#include <imesh/lighting.h>
#include <csutil/databuf.h>

class LightmapCache : public iCacheManager
{
private:
    VUtil::vRef<VOS::Property> property;

public:
    SCF_DECLARE_IBASE;

    LightmapCache(VOS::Property* target);
    virtual ~LightmapCache() { };
    virtual void SetCurrentType (const char* type);

    virtual const char* GetCurrentType () const;

    virtual void SetCurrentScope (const char* scope);

    virtual const char* GetCurrentScope () const;

    virtual bool CacheData (const void* data, size_t size,
                            const char* type, const char* scope, uint32 id);

    virtual csPtr<iDataBuffer> ReadCache (
        const char* type, const char* scope, uint32 id);

    virtual bool ClearCache (const char* type = 0, const char* scope = 0,
                             const uint32* id = 0);

    virtual void Flush();
    virtual void SetReadOnly(bool) { }
    virtual bool IsReadOnly() const { return false; }
};

#endif
