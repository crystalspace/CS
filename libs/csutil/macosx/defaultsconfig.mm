/*
    Copyright (C) 2003 by Greg Block

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/util.h"
#include "defaultsconfig.h"

#import <Foundation/NSEnumerator.h>

// -[NSUserDefaults objectIsForcedForKey:inDomain:] was introduced with
// MacOS/X 10.2, but we still want the code buildable and useable with 10.1,
// so we must perform a bit of magic by at least making the prototype known on
// 10.1.
@interface NSObject (csDefaultsConfig)
- (BOOL)objectIsForcedForKey:(NSString*)key inDomain:(NSString*)domain;
@end

csPtr<iConfigFile> csGetPlatformConfig (const char* key)
{
  csDefaultsConfig* cfg = new csDefaultsConfig();
  if (!cfg->Open (key))
  {
    delete cfg;
    cfg = 0;
  }
  return csPtr<iConfigFile> (cfg);
}


SCF_IMPLEMENT_IBASE (csDefaultsConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfigFile)
SCF_IMPLEMENT_IBASE_END

csDefaultsConfig::csDefaultsConfig ()
{
  SCF_CONSTRUCT_IBASE (0);

  // Domain information comes from the application's bundle identifier,
  // generally.  Grab a defaults object.
  defaults = [[NSUserDefaults standardUserDefaults] retain];
  domain = nil;
  dict = nil;
}

csDefaultsConfig::~csDefaultsConfig()
{
  if (dict != nil)
    [dict release];
  if (domain != nil)
    [domain release];
  if (defaults != nil)
    [defaults release];
  SCF_DESTRUCT_IBASE();
}

bool csDefaultsConfig::Open (const char* Key)
{
  domain = [[NSString alloc] initWithCString:Key];
  dict = [[defaults persistentDomainForName:domain] mutableCopy];
  if (dict == nil)
    dict = [[NSMutableDictionary alloc] init];
  return true;
}

const char* csDefaultsConfig::GetFileName () const
{
  return "NSUserDefaults";
}

iVFS* csDefaultsConfig::GetVFS () const
{
  return 0;
}

void csDefaultsConfig::SetFileName (const char*, iVFS*)
{
}

bool csDefaultsConfig::Load (const char*, iVFS*, bool, bool)
{
  // @@@ At some point implement loading and merging a serialised NSDictionary.
  return false;
}

bool csDefaultsConfig::Save ()
{
  if (dict != nil)
  {
    [defaults setPersistentDomain:dict forName:domain];
    [defaults synchronize];
  }
  return true;
}

bool csDefaultsConfig::Save (const char*, iVFS*)
{
  Save();
  // @@@ At some point implement saving to a serialised NSDictionary.
  return false;
}

void csDefaultsConfig::Clear ()
{
  [defaults removePersistentDomainForName:domain];
}

csPtr<iConfigIterator> csDefaultsConfig::Enumerate (const char* Subsection)
{
  if (!SubsectionExists (Subsection))
    return 0;
  return new csDefaultsIterator (this, Subsection);
}

bool csDefaultsConfig::KeyExists (NSString* Key) const
{
  return ([dict objectForKey:Key] != nil);
}

bool csDefaultsConfig::KeyExists (const char* Key) const
{
  return KeyExists([NSString stringWithCString:Key]);
}

// Check if we have permission to write to a key.  For MacOS/X 10.1 and earlier
// we always have permission.  For 10.2 and later, we have permission if the
// key does not already exist or if the administrator has not forced a key upon
// us.
bool csDefaultsConfig::Writable (NSString* Key) const
{
  bool writable;
  if (![defaults respondsToSelector:@selector(objectIsForcedForKey:inDomain:)])
    writable = true;
  else
    writable =
      !KeyExists(Key) || ![defaults objectIsForcedForKey:Key inDomain:domain];
  return writable;
}

// Check to see if we add a given subsection to our bundle ID, whether or not
// it exists.
bool csDefaultsConfig::SubsectionExists (const char* subsection) const
{
  NSString* section = [NSString stringWithFormat:@"%@.%s", domain, subsection];
  NSDictionary* subdict = [defaults persistentDomainForName:section];
  return subdict != nil;
}

int csDefaultsConfig::GetInt (const char* Key, int Def) const
{
  NSString* keystring = [NSString stringWithCString:Key];
  if (KeyExists(keystring))
    return [[[dict objectForKey:keystring] description] intValue];
  return Def;
}

float csDefaultsConfig::GetFloat (const char* Key, float Def) const
{
  NSString* keystring = [NSString stringWithCString:Key];
  if (KeyExists(keystring))
    return [[[dict objectForKey:keystring] description] floatValue];
  return Def;
}

const char* csDefaultsConfig::GetStr (const char* Key, const char* Def) const
{
  NSString* keystring = [NSString stringWithCString:Key];
  if (KeyExists(keystring))
    return [[[dict objectForKey:keystring] description] lossyCString];
  return Def;
}

bool csDefaultsConfig::GetBool (const char* Key, bool Def) const
{
  NSString* keystring = [NSString stringWithCString:Key];
  if (KeyExists(keystring))
  {
    char const* s = [[[dict objectForKey:keystring] description] lossyCString];
    char const c = tolower(s[0]);
bool ok = c == 'y' || c == 't' || c == '1';
NSLog(@"GetBool(%@,%d) -> %d", keystring, (int)Def, (int)ok);
    return c == 'y' || c == 't' || c == '1';
  }
  return Def;
}

const char* csDefaultsConfig::GetComment (const char* Key) const
{
  return 0;
}

void csDefaultsConfig::SetStr (const char* Key, const char* Val)
{
  NSString* keystr = [NSString stringWithCString:Key];
  NSString* valstr = [NSString stringWithCString:Val];
  if (Writable(keystr))
    [dict setObject:valstr forKey:keystr];
}

void csDefaultsConfig::SetInt (const char* Key, int Value)
{
  NSString* keystr = [NSString stringWithCString:Key];
  NSNumber* val = [NSNumber numberWithInt:Value];
  if (Writable(keystr))
    [dict setObject:val forKey:keystr];
}

void csDefaultsConfig::SetFloat (const char* Key, float Value)
{
  NSString* keystr = [NSString stringWithCString:Key];
  NSNumber* val = [NSNumber numberWithFloat:Value];
  if (Writable(keystr))
    [dict setObject:val forKey:keystr];
}

void csDefaultsConfig::SetBool (const char* Key, bool Value)
{
  NSString* keystr = [NSString stringWithCString:Key];
  NSString* valstr = (Value ? @"yes" : @"no");
  if (Writable(keystr))
    [dict setObject:valstr forKey:keystr];
}

bool csDefaultsConfig::SetComment (const char* Key, const char* Text)
{
  return false;
}

void csDefaultsConfig::DeleteKey (const char* Key)
{
  NSString* keystr = [NSString stringWithCString:Key];  
  [dict removeObjectForKey:keystr];
}

const char* csDefaultsConfig::GetEOFComment () const
{
  return 0;
}

void csDefaultsConfig::SetEOFComment (const char* Text)
{
}


SCF_IMPLEMENT_IBASE (csDefaultsIterator)
  SCF_IMPLEMENTS_INTERFACE (iConfigIterator)
SCF_IMPLEMENT_IBASE_END

csDefaultsIterator::csDefaultsIterator (
  csDefaultsConfig* Owner, const char* Subsection)
{
  SCF_CONSTRUCT_IBASE (0);

  // Retain our calling parameters.
  owner = Owner;
  name = [[NSString stringWithCString:Subsection] retain];

  // Construct our 'section name'.
  if (Subsection!=0)
    domain = [[NSString stringWithFormat:@"%@.%@",owner->domain,name] retain];
  else
    domain = [owner->domain retain];
  
  // Create our inner csDefaultConfig for the subdomain..
  config = new csDefaultsConfig();
  config->Open([domain lossyCString]);
  
  // Nil out the rest.
  keyenum = nil;
  currentkey = nil;
}

csDefaultsIterator::~csDefaultsIterator()
{
  delete config;
  [domain release];
  [name release];
  if (keyenum != nil)
    [keyenum release];
  owner = 0;  
  SCF_DESTRUCT_IBASE();
}

iConfigFile* csDefaultsIterator::GetConfigFile () const
{
  return owner;
}

const char* csDefaultsIterator::GetSubsection () const
{
  return [name lossyCString];
}

void csDefaultsIterator::Rewind ()
{
  if (keyenum != nil)
    [keyenum release];
  keyenum = nil;
  currentkey = nil;
}

// Navigate though the reg key to the next value entry.
bool csDefaultsIterator::Next()
{
  // Create the iterator if we haven't got one.
  if (keyenum == nil)
    keyenum = [[config->dict keyEnumerator] retain];
  currentkey = [keyenum nextObject];
  return currentkey != nil;
}

const char* csDefaultsIterator::GetKey (bool Local) const
{
  return [currentkey lossyCString];
}

int csDefaultsIterator::GetInt () const
{
  return config->GetInt([currentkey lossyCString], 0);
}

float csDefaultsIterator::GetFloat () const
{
  return config->GetFloat([currentkey lossyCString], 0.0f);
}

const char* csDefaultsIterator::GetStr () const
{
  return config->GetStr([currentkey lossyCString], 0);
}

bool csDefaultsIterator::GetBool () const
{
  return config->GetBool([currentkey lossyCString], false);
}

const char* csDefaultsIterator::GetComment () const
{
  return 0;
}
