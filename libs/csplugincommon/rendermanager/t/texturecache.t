/*
    Copyright (C) 2010 by Frank Richter

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

#include "csplugincommon/rendermanager/texturecache.h"
#include "cstool/initapp.h"

/**
 * Test CS::RenderManager::TextureCache operations.
 */
class TextureCacheTest : public CppUnit::TestFixture
{
private:
  class TestTextureCache :
    public CS::RenderManager::TextureCacheT<>
  {
    typedef CS::RenderManager::TextureCacheT<> S;
  public:
    TestTextureCache (uint options) :
      S (csimg2D, "rgb8", CS_TEXTURE_3D, 0, options, 
	CS::Utility::ResourceCache::ReuseConditionAfterTime<uint> (),
	CS::Utility::ResourceCache::PurgeConditionAfterTime<uint> (10000)) {}
  };
  
  iObjectRegistry* object_reg;
public:
  void setUp();
  void tearDown();

  void testQueryExact();
  void testQueryExact2();
  void testQueryLarger();
  void testReuse();
  void testReuse2();

  CPPUNIT_TEST_SUITE(TextureCacheTest);
    CPPUNIT_TEST(testQueryExact);
    CPPUNIT_TEST(testQueryExact2);
    CPPUNIT_TEST(testQueryLarger);
    CPPUNIT_TEST(testReuse);
    CPPUNIT_TEST(testReuse2);
  CPPUNIT_TEST_SUITE_END();
};

void TextureCacheTest::setUp()
{
  const char* const fake_argv[] = { "", 0 };
  object_reg = csInitializer::CreateEnvironment (0, fake_argv);
  CS_ASSERT (object_reg);
  bool status = csInitializer::SetupConfigManager (object_reg, 0);
  CS_ASSERT (status);
  status = csInitializer::RequestPlugins (object_reg,
    CS_REQUEST_NULL3D,
    // The plugins below aren't really required but present to quell warnings
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_ENGINE,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_END);
  CS_ASSERT (status);
  status = csInitializer::OpenApplication (object_reg);
  CS_ASSERT (status);
}

void TextureCacheTest::tearDown()
{
  csInitializer::DestroyApplication (object_reg);
  object_reg = 0;
}

/* Test exact queries work as they should */
void TextureCacheTest::testQueryExact()
{
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  CS_ASSERT (g3d);
  
  TestTextureCache cache (TestTextureCache::tcacheExactSizeMatch);
  cache.SetG3D (g3d);
  int real_w, real_h;
  iTextureHandle* t1 = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(16, real_w);
  CPPUNIT_ASSERT_EQUAL(32, real_h);
  int render_w, render_h;
  t1->GetRendererDimensions (render_w, render_h);
  CPPUNIT_ASSERT_EQUAL(real_w, render_w);
  CPPUNIT_ASSERT_EQUAL(real_h, render_h);
}

void TextureCacheTest::testQueryExact2()
{
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  CS_ASSERT (g3d);
  
  TestTextureCache cache (TestTextureCache::tcacheExactSizeMatch);
  cache.SetG3D (g3d);
  int real_w, real_h;
  iTextureHandle* t1 = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  iTextureHandle* t2 = cache.QueryUnusedTexture (16, 16, real_w, real_h);
  iTextureHandle* t3 = cache.QueryUnusedTexture (32, 16, real_w, real_h);
  
  cache.AdvanceFrame (1);
  iTextureHandle* tt;
  tt = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(t1, tt);
  tt = cache.QueryUnusedTexture (16, 16, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(t2, tt);
  tt = cache.QueryUnusedTexture (32, 16, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(t3, tt);
}

void TextureCacheTest::testQueryLarger()
{
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  CS_ASSERT (g3d);
  
  TestTextureCache cache (0);
  cache.SetG3D (g3d);
  int real_w, real_h;
  iTextureHandle* t1 = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  cache.AdvanceFrame (1);
  iTextureHandle* t2 = cache.QueryUnusedTexture (16, 16, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(t1, t2);
  
  iTextureHandle* t3 = cache.QueryUnusedTexture (16, 64, real_w, real_h);
  iTextureHandle* t4 = cache.QueryUnusedTexture (64, 16, real_w, real_h);
  cache.AdvanceFrame (2);
  iTextureHandle* tt;
  cache.QueryUnusedTexture (16, 16, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(16, real_w);
  CPPUNIT_ASSERT_EQUAL(32, real_h);
  tt = cache.QueryUnusedTexture (16, 64, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(16, real_w);
  CPPUNIT_ASSERT_EQUAL(64, real_h);
  CPPUNIT_ASSERT_EQUAL(t3, tt);
  tt = cache.QueryUnusedTexture (64, 16, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(64, real_w);
  CPPUNIT_ASSERT_EQUAL(16, real_h);
  CPPUNIT_ASSERT_EQUAL(t4, tt);
}

/* Test reusing works even if putting a lot of different sizes into the cache */
void TextureCacheTest::testReuse()
{
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  CS_ASSERT (g3d);
  
  TestTextureCache cache (TestTextureCache::tcacheExactSizeMatch);
  cache.SetG3D (g3d);
  int real_w, real_h;
  iTextureHandle* t1 = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  
  cache.AdvanceFrame (1);
  iTextureHandle* t2 = cache.QueryUnusedTexture (16, 32, real_w, real_h);
  CPPUNIT_ASSERT_EQUAL(t1, t2);
  
  static const int dimensions[][2] = {{16, 64}, {32, 16}, {16, 64}, {16, 64},
    {64, 64}, {64, 32}};
  const size_t numDimensions = sizeof (dimensions)/sizeof(int[2]);
  for (size_t i = 0; i < numDimensions; i++)
  {
    cache.QueryUnusedTexture (dimensions[i][0], dimensions[i][1]);
    cache.AdvanceFrame (2+i);
    
    iTextureHandle* t2 = cache.QueryUnusedTexture (16, 32);
    CPPUNIT_ASSERT_EQUAL(t1, t2);
  }
}
  
void TextureCacheTest::testReuse2()
{
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  CS_ASSERT (g3d);
  
  TestTextureCache cache (TestTextureCache::tcacheExactSizeMatch);
  cache.SetG3D (g3d);
  int real_w, real_h;
  
  static const int dimensions[][2] = {{128, 128}, {128, 128}, {256, 64}, {64, 128},
    {128, 256}, {64, 64}};
  const size_t numDimensions = sizeof (dimensions)/sizeof(int[2]);
  for (size_t i = 0; i < numDimensions; i++)
  {
    cache.QueryUnusedTexture (dimensions[i][0], dimensions[i][1], real_w, real_h);
  }
    
  cache.AdvanceFrame (1);
  iTextureHandle* t1 = cache.QueryUnusedTexture (128, 512, real_w, real_h);
  
  for (size_t i = 0; i < 16; i++)
  {
    cache.AdvanceFrame (2+i);
    iTextureHandle* t2 = cache.QueryUnusedTexture (128, 512, real_w, real_h);
    CPPUNIT_ASSERT_EQUAL(t1, t2);
  }
}
