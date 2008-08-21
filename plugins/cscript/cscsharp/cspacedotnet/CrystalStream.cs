/*
    Copyright (C) 2008 by Ronie Salgado <roniesalg@gmail.com>

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

using System;
using System.IO;

namespace CrystalSpace
{
	
  /// <sumary>
  /// A thin iFile wrapper around System.IO.Stream
  /// </sumary>
	public class CrystalStream: Stream
	{
		private iFile stream_file;
		private int stream_mode;
		
		public CrystalStream(iFile file, int mode)
		{
			stream_file = file;
			stream_mode = mode;
		}
		
		public override int Read (byte[] buffer, int offset, int count)
		{
			if(stream_file != null)
				return (int)stream_file.Read(buffer, offset, count);
			return 0;
		}

		public override void Write (byte[] buffer, int offset, int count)
		{
			if(stream_file != null)
				stream_file.Write(buffer, offset, count);
		}

		public override void Flush ()
		{
			if(stream_file != null)
				stream_file.Flush();
		}
		
		public override long Seek (long offset, SeekOrigin origin)
		{
			if(stream_file != null)
			{
				switch(origin)
				{
				case SeekOrigin.Begin:
					stream_file.SetPos(offset);
					break;
				case SeekOrigin.Current:
					stream_file.SetPos(stream_file.GetPos() + offset);
					break;
				case SeekOrigin.End:
					long newpos = stream_file.GetSize() - offset;
					if(newpos < 0)
						newpos = 0;
					stream_file.SetPos(newpos);
					break;
				}
	
				return stream_file.GetPos();
			}
			return 0;
		}

		public override void SetLength (long value)
		{
			Console.WriteLine("Trying to set the length of a crystal stream");
			throw new NotImplementedException ();
		}

		public override bool CanRead {
			get { return true; }
		}
		
		public override bool CanSeek
		{
			get
			{
				if(stream_file == null)
					return false;
				return true;
			}
		}

		public override bool CanWrite
		{
			get { return (stream_mode & CS.VFS_FILE_WRITE) != 0; }
		}

		public override long Position
		{
			get
			{
				if(stream_file == null)
					return 0;
				return stream_file.GetPos();
			}
			
			set
			{
				if(stream_file != null)
					stream_file.SetPos(value);
			}
		}
		
		public override long Length
		{
			get
			{
				if(stream_file == null)
					return 0;
				return stream_file.GetSize();
			}
		}
		
		public iFile GetCrystalFile()
		{
			return stream_file;
		}
	}
}
