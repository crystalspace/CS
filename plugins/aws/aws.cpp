/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "aws.h"
#include "awsprefs.h"
#include "awsslot.h"
#include "awscscr.h"
#include "awskcfct.h"
#include "awslayot.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (awsKeyFactory)
  SCF_IMPLEMENTS_INTERFACE (iAwsKeyFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsConnectionNodeFactory)
  SCF_IMPLEMENTS_INTERFACE (iAwsConnectionNodeFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsKey)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE_EXT (awsIntKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsIntKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsFloatKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsFloatKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsStringKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsStringKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsRectKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsRectKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsPointKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsPointKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsRGBKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsRGBKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsConnectionKey)
  SCF_IMPLEMENTS_INTERFACE (iAwsConnectionKey)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsComponentNode)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponentNode)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE_EXT (awsKeyContainer)
  SCF_IMPLEMENTS_INTERFACE (iAwsKeyContainer)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE (awsScreenCanvas)
  SCF_IMPLEMENTS_INTERFACE (iAwsCanvas)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsComponent)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE_EXT (awsWindow)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE (awsSink)
  SCF_IMPLEMENTS_INTERFACE (iAwsSink)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsSlot)
  SCF_IMPLEMENTS_INTERFACE (iAwsSlot)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsSource)
  SCF_IMPLEMENTS_INTERFACE (iAwsSource)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsSinkManager)
  SCF_IMPLEMENTS_INTERFACE (iAwsSinkManager)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsPrefManager)
  SCF_IMPLEMENTS_INTERFACE (iAwsPrefManager)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsManager)
  SCF_IMPLEMENTS_INTERFACE (iAws)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (awsManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (awsManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (awsManager)
SCF_IMPLEMENT_FACTORY (awsPrefManager)
SCF_IMPLEMENT_FACTORY (awsSinkManager)

SCF_IMPLEMENT_IBASE (awsLayoutManager)
  SCF_IMPLEMENTS_INTERFACE (iAwsLayoutManager)
SCF_IMPLEMENT_IBASE_END
