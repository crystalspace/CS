/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_JPEGCXXWRAPPER_H__
#define __CS_JPEGCXXWRAPPER_H__

namespace CS
{
  namespace Jpeg
  {
    /// Jpeg common compress/decompress data
    class Common : public jpeg_common_struct
    {
    };

    class DestinationManager;

    /// Jpeg compress structure
    class Compress : public jpeg_compress_struct
    {
    public:
      Compress (jpeg_error_mgr* errorManager)
      {
        err = errorManager;
        jpeg_create_compress (this);
      }
      ~Compress()
      {
        jpeg_destroy_compress (this);
      }

      void SetDestination (DestinationManager* dst);

      void SetDefaults ()
      { jpeg_set_defaults (this); }
      void SetQuality (int quality, bool force_baseline)
      { jpeg_set_quality (this, quality, force_baseline); }
      void SimpleProgression ()
      { jpeg_simple_progression (this); }

      void StartCompress (bool write_all_tables)
      { jpeg_start_compress (this, write_all_tables); }
      void FinishCompress ()
      { jpeg_finish_compress (this); }

      void WriteScanlines (JSAMPARRAY scanlines, JDIMENSION num_lines)
      { jpeg_write_scanlines (this, scanlines, num_lines); }

      operator Common& ()
      { return *((Common*)(jpeg_common_struct*)(jpeg_compress_struct*)this); }
      operator const Common& () const
      { return *((Common*)(jpeg_common_struct*)(jpeg_compress_struct*)this); }
    };

    class SourceManager;

    /// Jpeg decompress structure
    class Decompress : public jpeg_decompress_struct
    {
    public:
      Decompress (jpeg_error_mgr* errorManager)
      {
        err = errorManager;
        jpeg_create_decompress (this);
      }
      ~Decompress()
      {
        jpeg_destroy_decompress (this);
      }

      void SetDataSource (SourceManager* source);

      int ReadHeader (bool require_image)
      { return jpeg_read_header(this, require_image); }
      void CalcOutputDimensions ()
      { jpeg_calc_output_dimensions (this); }

      void StartDecompress ()
      { jpeg_start_decompress (this); }
      void FinishDecompress ()
      { jpeg_finish_decompress (this); }

      JDIMENSION ReadScanlines (JSAMPARRAY scanlines, JDIMENSION max_lines)
      { return jpeg_read_scanlines (this, scanlines, max_lines); }

      operator Common& ()
      { return *((Common*)(jpeg_common_struct*)(jpeg_decompress_struct*)this); }
      operator const Common& () const
      { return *((Common*)(jpeg_common_struct*)(jpeg_decompress_struct*)this); }
    };

    /// Jpeg compress destination manager
    class DestinationManager : public jpeg_destination_mgr
    {
      static void _init_destination (j_compress_ptr cinfo)
      {
        DestinationManager* this_ = 
          static_cast<DestinationManager*> (cinfo->dest);
        this_->Init (*(static_cast<Compress*> (cinfo)));
      }
      static boolean _empty_output_buffer (j_compress_ptr cinfo)
      {
        DestinationManager* this_ = 
          static_cast<DestinationManager*> (cinfo->dest);
        return this_->EmptyOutputBuffer (*(static_cast<Compress*> (cinfo)));
      }
      static void _term_destination (j_compress_ptr cinfo)
      {
        DestinationManager* this_ = 
          static_cast<DestinationManager*> (cinfo->dest);
        this_->Term (*(static_cast<Compress*> (cinfo)));
      }
    public:
      DestinationManager()
      {
        init_destination = _init_destination;
        empty_output_buffer = _empty_output_buffer;
        term_destination = _term_destination;
      }
      virtual ~DestinationManager() {}

      virtual void Init (Compress& cinfo) = 0;
      virtual bool EmptyOutputBuffer (Compress& cinfo) = 0;
      virtual void Term (Compress& cinfo) = 0;
    };

    inline void Compress::SetDestination (DestinationManager* dst)
    { dest = dst; }

    /// Jpeg decompress source manager
    class SourceManager : public jpeg_source_mgr
    {
      static void _init_source (j_decompress_ptr cinfo)
      {
        SourceManager* this_ = static_cast<SourceManager*> (cinfo->src);
        this_->Init (*(static_cast<Decompress*> (cinfo)));
      }
      static boolean _fill_input_buffer (j_decompress_ptr cinfo)
      {
        SourceManager* this_ = static_cast<SourceManager*> (cinfo->src);
        return this_->FillInputBuffer (*(static_cast<Decompress*> (cinfo)));
      }
      static void _skip_input_data (j_decompress_ptr cinfo, long num_bytes)
      {
        SourceManager* this_ = static_cast<SourceManager*> (cinfo->src);
        this_->SkipInputData (*(static_cast<Decompress*> (cinfo)), num_bytes);
      }
      static boolean _resync_to_restart (j_decompress_ptr cinfo, int desired)
      {
        SourceManager* this_ = static_cast<SourceManager*> (cinfo->src);
        return this_->ResyncToRestart (*(static_cast<Decompress*> (cinfo)), desired);
      }
      static void _term_source (j_decompress_ptr cinfo)
      {
        SourceManager* this_ = static_cast<SourceManager*> (cinfo->src);
        this_->Term (*(static_cast<Decompress*> (cinfo)));
      }
    public:
      SourceManager()
      {
        init_source = _init_source;
        fill_input_buffer = _fill_input_buffer;
        skip_input_data = _skip_input_data;
        resync_to_restart = _resync_to_restart;
        term_source = _term_source;
      }
      virtual ~SourceManager() {}

      virtual void Init (Decompress& cinfo) = 0;
      virtual bool FillInputBuffer (Decompress& cinfo) = 0;
      virtual void SkipInputData (Decompress& cinfo, long numBytes) = 0;
      virtual bool ResyncToRestart (Decompress& cinfo, int desired)
      {
        return (jpeg_resync_to_restart (&cinfo, desired) != 0) ? true : false;
      }
      virtual void Term (Decompress& cinfo) = 0;
    };

    inline void Decompress::SetDataSource (SourceManager* source)
    { src = source; }

  }
}

#endif // __CS_JPEGCXXWRAPPER_H__
