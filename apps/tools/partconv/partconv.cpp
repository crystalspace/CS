/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "crystalspace.h"

#include "partconv.h"

CS_IMPLEMENT_APPLICATION

PartConv::PartConv (iObjectRegistry* objectRegistry)
  : objectRegistry (objectRegistry), unconvertedTags (false)
{

}

void PartConv::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (objectRegistry, CS_REPORTER_SEVERITY_ERROR,
    "crystalspace.apps.partconv", description, arg);
  va_end (arg);
}

void PartConv::Report (int severity, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (objectRegistry, severity,
    "crystalspace.apps.partconv", description, arg);
  va_end (arg);
}


void PartConv::Main ()
{
  // Setup the hashesh
  InitTokenTable (xmltokens);

  particlePlugins.Register ("crystalspace.mesh.loader.particles",
    PARTCONV_PARTSYS_PARTICLES);
  particlePlugins.Register ("crystalspace.mesh.loader.factory.particles", 
    PARTCONV_PARTSYS_PARTICLES_FACT);

  // Get plugins we need
  vfs = csQueryRegistry<iVFS> (objectRegistry);
  commandLine = csQueryRegistry<iCommandLineParser> (objectRegistry);

  syntaxService = csQueryRegistryOrLoad<iSyntaxService> (objectRegistry,
    "crystalspace.syntax.loader.service.text");

  if (!vfs || !commandLine || !syntaxService)
  {
    ReportError ("Could not load required plugins!");
    return;
  }

  if (commandLine->GetOption ("help"))
  {
    csPrintf ("partconv <options> <file>\n");
    csPrintf ("  -help:\n");
    csPrintf ("     Print this help text.\n");
    csPrintf ("  -inds=<plugin>:\n");       
    csPrintf ("     Document system plugin for reading world.\n");
    csPrintf ("  -outds=<plugin>:\n");       
    csPrintf ("     Document system plugin for writing world.\n");
    return;
  }

  csRef<iDocumentSystem> inputDS;
  csRef<iDocumentSystem> outputDS;
  {
    const char *inds = commandLine->GetOption ("inds");
    if (inds)
    {
      csString plg;
      plg.Format ("crystalspace.documentsystem.%s", inds);

      inputDS = csLoadPlugin<iDocumentSystem> (objectRegistry, plg.GetData ());

      if (!inputDS)
      {
        ReportError ("Unable to load input document system %s!", CS::Quote::Single (inds));
        return;
      }
    }

    const char *outds = commandLine->GetOption ("outds");
    if (outds)
    {
      csString plg;
      plg.Format ("crystalspace.documentsystem.%s", outds);

      outputDS = csLoadPlugin<iDocumentSystem> (objectRegistry, plg.GetData ());

      if (!outputDS)
      {
        ReportError ("Unable to load output document system %s!", CS::Quote::Single (outds));
        return;
      }
    }
  }

  // Get file to convert
  const char* val = commandLine->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive! "
      "Use %s to get a list of possible options.",
      CS::Quote::Single ("partconv -help"));
    return;
  }

  csString filename;
  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/partconv_data", val);
    filename = "/tmp/partconv_data/world";
  }
  else
  {
    filename = val;
  }

  // Read the file
  csRef<iDataBuffer> buf = vfs->ReadFile (filename);
  if (!buf || !buf->GetSize ())
  {
    ReportError ("File %s does not exist!", CS::Quote::Single ((const char*)filename));
    return;
  }

  // Make a backup
  vfs->WriteFile (filename+".bak", **buf, buf->GetSize ());
  
  csRef<iDocumentSystem> xml;
  if (inputDS)
  {
    xml = inputDS;
  }
  else
  {
    xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  }

  // Parse input
  csRef<iDocument> doc = xml->CreateDocument ();
  csRef<iDocumentNode> root;
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Parsing...");
  
  csTicks parse_start = csGetTicks();
  const char* error = doc->Parse (buf, true);
  csTicks parse_end = csGetTicks();
  buf = 0;
  if (error != 0)
  {
    ReportError ("Error parsing XML: %s!", error);
    return;
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
      (float)(parse_end - parse_start) / (float)1000);
  }

  // Setup output
  csRef<iDocumentSystem> newsys;
  csRef<iDocument> newdoc;
  csRef<iDocumentNode> newroot;
  if (outputDS)
  {
    newsys = outputDS;
  }
  else
  {
    newsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  }
  newdoc = newsys->CreateDocument ();
  newroot = newdoc->CreateRoot ();

  // Validate input
  root = doc->GetRoot ();

  // Clone the source
  CloneNode (root, newroot);

  // Convert the entire hierarchy
  ConvertDocument (newroot);


  // Write the result
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Writing...");
  csTicks writing_start = csGetTicks();
  error = newdoc->Write (vfs, filename);
  csTicks writing_end = csGetTicks();
  if (error != 0)
  {
    ReportError ("Error writing %s: %s!", CS::Quote::Single ((const char*)filename), error);
    return;
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
      (float)(writing_end - writing_start) / (float)1000);
  }
  newroot = 0;
  newdoc = 0;
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Updating VFS...");
  vfs->Sync();

  if (unconvertedTags)
    Report (CS_REPORTER_SEVERITY_WARNING, "Some tags were not converted."
    " Check the resulting particle system before using it!");
}

void PartConv::CloneNode (iDocumentNode* from, iDocumentNode* to) const
{
  to->SetValue (from->GetValue ());
  
  csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    to->SetAttribute (attr->GetName (), attr->GetValue ());
  }

  csRef<iDocumentNodeIterator> it = from->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
      child->GetType (), 0);
    CloneNode (child, child_clone);
  }
}

void PartConv::ConvertDocument (iDocumentNode* root)
{
  // Try to determin what kind of file this is
  csRef<iDocumentNode> topNode = root->GetNode ("world");

  if (!topNode)
  {
    topNode = root->GetNode ("library");
  }

  if (topNode)
  {
    // Get all plugin aliases
    AnalyzePluginSection (topNode);

    // Convert all factories
    ConvertMeshFactories (topNode);

    // Convert all objects
    ConvertMeshObjects (topNode);

    return;
  }

  // This is a meshobject file, cope with that
  topNode = root->GetNode ("meshobj");
  if (topNode)
  {
    ConvertMeshObject (topNode);
    return;
  }

  topNode = root->GetNode ("meshfact");
  if (topNode)
  {
    ConvertMeshFactory (topNode);
    return;
  }

  topNode = root->GetNode ("params");
  if (topNode)
  {
    if (topNode->GetNode ("factory"))
      ConvertParticlesObjectParams (topNode, root);
    else
      ConvertParticlesFactoryParams (topNode, root);

    return;
  }
}

void PartConv::AnalyzePluginSection (iDocumentNode* topNode)
{
  csRef<iDocumentNode> pluginsNode = topNode->GetNode ("plugins");
  if (!pluginsNode)
    return;

  csRef<iDocumentNodeIterator> pIt = pluginsNode->GetNodes ("plugin");
  while (pIt->HasNext ())
  {
    csRef<iDocumentNode> pluginNode = pIt->Next ();

    if (pluginNode->GetType () != CS_NODE_ELEMENT)
      continue;

    const char* pluginName = pluginNode->GetContentsValue ();
    const char* alias = pluginNode->GetAttributeValue ("name");

    if (!pluginName || !alias)
      continue;

    pluginNameHash.Put (csString (alias), csString(pluginName));
  }
}

void PartConv::ConvertMeshFactories (iDocumentNode* topNode)
{
  // Top node is either a <world> or <library> node, or another meshfact.. 
  // find all mesh factories and convert them

  csRef<iDocumentNodeIterator> it = topNode->GetNodes ("meshfact");
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT)
      continue;

    ConvertMeshFactory (child);

    // Recurse
    ConvertMeshFactories (child);
  }
}

void PartConv::ConvertMeshObjects (iDocumentNode* topNode)
{

}

void PartConv::ConvertMeshFactory (iDocumentNode* factoryNode)
{
  // Get plugins node
  csRef<iDocumentNode> pluginNode = factoryNode->GetNode ("plugin");
  if (!pluginNode)
    return;

  const char* pluginName = pluginNode->GetContentsValue ();
  if (!pluginName)
    return;

  csString fullPluginName;
  fullPluginName = pluginNameHash.Get (pluginName, pluginName);

  // Check against known particle systems
  csStringID plugId = particlePlugins.Request (fullPluginName);

  switch (plugId)
  {
  case PARTCONV_PARTSYS_PARTICLES_FACT:
    {
      pluginNode->SetValue (particlePlugins.Request (
        PARTCONV_PARTSYS_PARTICLES_FACT));

      csRef<iDocumentNode> paramsNode = factoryNode->GetNode ("params");
      if (!paramsNode)
        return;

      ConvertParticlesFactoryParams (paramsNode, factoryNode);
    }
    break;
  default:
    return;
  }
}

void PartConv::ConvertMeshObject (iDocumentNode* objectNode)
{
  // Get plugins node
  csRef<iDocumentNode> pluginNode = objectNode->GetNode ("plugin");
  if (!pluginNode)
    return;

  const char* pluginName = pluginNode->GetContentsValue ();
  if (!pluginName)
    return;

  csString fullPluginName;
  fullPluginName = pluginNameHash.Get (pluginName, pluginName);

  // Check against known particle systems
  csStringID plugId = particlePlugins.Request (fullPluginName);

  switch (plugId)
  {
  case PARTCONV_PARTSYS_PARTICLES:
    {
      pluginNode->SetValue (particlePlugins.Request (
        PARTCONV_PARTSYS_PARTICLES));

      csRef<iDocumentNode> paramsNode = objectNode->GetNode ("params");
      if (!paramsNode)
        return;

      ConvertParticlesObjectParams (paramsNode, objectNode);
    }
    break;
  default:
    return;
  }
}

void PartConv::ConvertParticlesFactoryParams (iDocumentNode* paramsNode,
                                              iDocumentNode* factNode)
{
  // Create new params
  csRef<iDocumentNode> newParams = factNode->CreateNodeBefore (
    CS_NODE_ELEMENT, 0);
  newParams->SetValue ("params");

  // Handle any specific stuff (none atm ;)

  // Handle the common things
  ConvertParticlesCommonParams (paramsNode, newParams);

  // Remove old
  factNode->RemoveNode (paramsNode);
}

void PartConv::ConvertParticlesObjectParams (iDocumentNode* paramsNode,
                                             iDocumentNode* objNode)
{
  // Create new params
  csRef<iDocumentNode> newParams = objNode->CreateNodeBefore (
    CS_NODE_ELEMENT, 0);
  newParams->SetValue ("params");

  

  // Handle specific stuff
  csRef<iDocumentNode> factNode = objNode->GetNode ("factory");
  if (factNode)
  {
    csRef<iDocumentNode> newFact = newParams->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    newFact->SetValue ("factory");
    newFact->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (
      factNode->GetContentsValue ());
  }

  // Handle common things
  ConvertParticlesCommonParams (paramsNode, newParams);

  // Remove old
  objNode->RemoveNode (paramsNode);
}

void PartConv::ConvertParticlesCommonParams (iDocumentNode* oldParams, 
                                             iDocumentNode* newParams)
{
  //Emitter properties
  enum { EMIT_NONE, EMIT_BOX, EMIT_SPHERE, EMIT_CYL } emitType (EMIT_NONE);
  bool emitEnabled = true;
  csVector3 emitSize (0.0f);
  float minTTL (1.0f), spanTTL (0.0f), emissionRate (0.0f), minMass (1.0f),
    spanMass (0.0f), duration (FLT_MAX);

  //Effector properties
  bool haveGravity = false, haveGradient = false;
  csVector3 gravity (0.0f);
  csArray<csColor> colorGradient;

  csRef<iDocumentNodeIterator> it = oldParams->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT)
      continue;

    csStringID token = xmltokens.Request (child->GetValue ());
    switch (token)
    {
    case XMLTOKEN_MATERIAL:
      {
        csRef<iDocumentNode> materialNode = newParams->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        materialNode->SetValue ("material");
        materialNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (
          child->GetContentsValue ());
      }
      break;
    case XMLTOKEN_EMITTER:
      {
        // Parse emitter
        const char* type = child->GetAttributeValue ("type");
        if (!type)
        {
          ReportError ("No emitter type given!");
          continue;
        }
        
        csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
        while (it2->HasNext ())
        {
          csRef<iDocumentNode> child2 = it2->Next ();
          if (child2->GetType () != CS_NODE_ELEMENT)
            continue;

          csStringID token2 = xmltokens.Request (child2->GetValue ());
          switch (token2)
          {
          case XMLTOKEN_OUTERRADIUS:
            emitSize.x = child2->GetContentsValueAsFloat ();
            break;
          case XMLTOKEN_INNERRADIUS:
            emitSize.y = child2->GetContentsValueAsFloat ();
            break;
          case XMLTOKEN_SIZE:
            syntaxService->ParseVector (child2, emitSize);
            if (emitSize.SquaredNorm () == 0)
              emitSize.x = child2->GetContentsValueAsFloat ();
            break;
          case XMLTOKEN_TIME:
            duration = child2->GetContentsValueAsFloat ();
            break;
          case XMLTOKEN_FORCE:
            //TODO!
            unconvertedTags = true;
            break;
          default:
            Report (CS_REPORTER_SEVERITY_WARNING, 
              "Unhandled in tag:%s <emitter>",child->GetValue ());
          }
        }

        if (!strcasecmp (type, "box"))
          emitType = EMIT_BOX;
        else if (!strcasecmp (type, "plane"))
        {
          emitType = EMIT_BOX;
          emitSize.y = 0;
        }
        else if (!strcasecmp (type, "point"))
        {
          emitSize.x = 0;
          emitType = EMIT_SPHERE;
        }
        else if (!strcasecmp (type, "sphere"))
          emitType = EMIT_SPHERE;
        else if (!strcasecmp (type, "cylinder"))
        {
          emitType = EMIT_CYL;
        }

      }
      break;
    case XMLTOKEN_DIFFUSION:
      //@@TODO
      unconvertedTags = true;
      break;
    case XMLTOKEN_GRAVITY:
      {
        haveGravity = true;
        syntaxService->ParseVector (child, gravity);
      }
      break;
    case XMLTOKEN_TTL:
      minTTL = child->GetContentsValueAsFloat ();
      break;
    case XMLTOKEN_TIMEVARIATION:
      spanTTL = child->GetContentsValueAsFloat ();
      break;
    case XMLTOKEN_INITIAL:
      //@@ TODO!
      unconvertedTags = true;
      break;
    case XMLTOKEN_PPS:
      emissionRate = child->GetContentsValueAsFloat ();
      break;
    case XMLTOKEN_RADIUS:
      {
        float radius = child->GetContentsValueAsFloat ();

        csRef<iDocumentNode> sizeNode = newParams->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        sizeNode->SetValue ("particlesize");
        sizeNode->SetAttributeAsFloat ("x", radius);
        sizeNode->SetAttributeAsFloat ("y", radius);
      }
      break;
    case XMLTOKEN_MASS:
      minMass = child->GetContentsValueAsFloat ();
      break;
    case XMLTOKEN_MASSVARIATION:
      spanMass = child->GetContentsValueAsFloat ();
      break;
    case XMLTOKEN_AUTOSTART:
      {
        const char *autostart = child->GetContentsValue ();
        if(!strcasecmp(autostart, "no"))
          emitEnabled = false;
        else if(!strcasecmp(autostart, "yes")) 
          emitEnabled = true;
        else 
        {
          ReportError ("Unknown autostart value: %s", autostart);
        }
        break;
      }
    case XMLTOKEN_TRANSFORMMODE:
      {
        const char *mode = child->GetContentsValue ();
        if(!strcasecmp(mode, "no")) 
        {
          //nothing
        }
        else if(!strcasecmp(mode, "yes")) 
        {
          csRef<iDocumentNode> localNode = newParams->CreateNodeBefore (
            CS_NODE_ELEMENT, 0);
          localNode->SetValue ("localmode");
        }
        else 
        {
          ReportError ("Unknown transform mode: %s", mode);
        }
      }
      break;
    case XMLTOKEN_PHYSICSPLUGIN:
      //Nothing, not valid anymore
      break;
    case XMLTOKEN_DAMPENER:
      //@@TODO
      unconvertedTags = true;
      break;
    case XMLTOKEN_COLORMETHOD:
      {
        const char *str = child->GetAttributeValue ("type");
        if (!str)
        {
          ReportError ("No color method given.");
        }
        if (!strcasecmp (str, "constant"))
        {
          csColor c (1,1,1);
          colorGradient.Empty ();

          csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
          while (it2->HasNext ())
          {
            csRef<iDocumentNode> child2 = it2->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT)
              continue;

            csStringID token2 = xmltokens.Request (child2->GetValue ());
            switch (token2)
            {
            case XMLTOKEN_COLOR:
              syntaxService->ParseColor (child2, c);
              break;
            default:
              Report (CS_REPORTER_SEVERITY_WARNING, 
                "Unhandled in tag: %s",child2->GetValue ());
            }
          }

          colorGradient.Push (c);
          haveGradient = true;
        }
        else if (!strcasecmp (str, "linear"))
        {
          csColor c;
          colorGradient.Empty ();

          csRef<iDocumentNode> gradient = 
            child->GetNode (xmltokens.Request (XMLTOKEN_GRADIENT));

          if (!gradient)
          {
            Report (CS_REPORTER_SEVERITY_WARNING, "No <gradient> node");
            continue;
          }

          csRef<iDocumentNodeIterator> it2 = gradient->GetNodes ();
          while (it2->HasNext ())
          {
            csRef<iDocumentNode> child2 = it2->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT)
              continue;

            csStringID token2 = xmltokens.Request (child2->GetValue ());
            switch (token2)
            {
            case XMLTOKEN_COLOR:
              syntaxService->ParseColor (child2, c);
              colorGradient.Push (c);
              break;
            default:
              Report (CS_REPORTER_SEVERITY_WARNING, 
                "Unhandled in tag: %s",child2->GetValue ());
            }
          }

          haveGradient = true;
        }
        else
        {
          ReportError ("Unknown color method: %s", str);
        }
      }
      break;
    case XMLTOKEN_MIXMODE:
      {
        csRef<iDocumentNode> mixNode = newParams->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        CloneNode (child, mixNode);
      }
      break;
    case XMLTOKEN_ZSORT:
      {
        csRef<iDocumentNode> sortNode = newParams->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        sortNode->SetValue ("sortmode");
        sortNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue ("dot");
      }
      break;
    default:
      Report (CS_REPORTER_SEVERITY_WARNING, "Unhandled tag:%s",
        child->GetValue ());
      break;
    }
  }

  // Write emitter
  if (emitType != EMIT_NONE)
  {
    csRef<iDocumentNode> emitNode = newParams->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    emitNode->SetValue ("emitter");

    // General
    csRef<iDocumentNode> ttlNode = emitNode->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    ttlNode->SetValue ("initialttl");
    ttlNode->SetAttributeAsFloat ("min", minTTL);
    ttlNode->SetAttributeAsFloat ("max", minTTL+spanTTL);

    csRef<iDocumentNode> massNode = emitNode->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    massNode->SetValue ("mass");
    massNode->SetAttributeAsFloat ("min", minMass);
    massNode->SetAttributeAsFloat ("max", minMass+spanMass);

    csRef<iDocumentNode> valueNode;

    if (duration < FLT_MAX)
    {
      csRef<iDocumentNode> durationNode = emitNode->CreateNodeBefore (
        CS_NODE_ELEMENT, 0);
      durationNode->SetValue ("duration");

      valueNode = durationNode->CreateNodeBefore (CS_NODE_TEXT, 0);
      valueNode->SetValueAsFloat (duration);
    }

    csRef<iDocumentNode> rateNode = emitNode->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    rateNode->SetValue ("emissionrate");

    valueNode = rateNode->CreateNodeBefore (CS_NODE_TEXT, 0);
    valueNode->SetValueAsFloat (emissionRate);

    // Type specific
    switch (emitType)
    {
    case EMIT_BOX:
      {
        emitNode->SetAttribute ("type", "box");
        csRef<iDocumentNode> boxNode = emitNode->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        boxNode->SetValue ("box");

        csOBB tmpObb;
        tmpObb.SetSize (emitSize);

        syntaxService->WriteBox (boxNode, tmpObb);
      }
      break;
    case EMIT_SPHERE:
      {
        emitNode->SetAttribute ("type", "sphere");
        csRef<iDocumentNode> radiusNode = emitNode->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        radiusNode->SetValue ("radius");
        radiusNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (
          emitSize.x);

        if (emitSize.x == 0.0f)
        {
          csRef<iDocumentNode> placeNode = emitNode->CreateNodeBefore (
            CS_NODE_ELEMENT, 0);
          placeNode->SetValue ("placement");
          placeNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue ("center");
        }
      }
      break;
    case EMIT_CYL:
      {
        emitNode->SetAttribute ("type", "cylinder");
        emitNode->SetAttribute ("type", "sphere");
        csRef<iDocumentNode> radiusNode = emitNode->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
        radiusNode->SetValue ("radius");
        radiusNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (
          emitSize.x);
        
      }
      break;
    default:
      break;
    }
  }

  // Write effector(s)
  if (haveGravity)
  {
    csRef<iDocumentNode> forceEffectorNode = newParams->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    forceEffectorNode->SetValue ("effector");
    forceEffectorNode->SetAttribute ("type", "force");
    csRef<iDocumentNode> anode = forceEffectorNode->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    anode->SetValue ("acceleration");

    syntaxService->WriteVector (anode, gravity);
  }

  if (haveGradient && colorGradient.GetSize () > 0)
  {
    float maxTTL = minTTL + spanTTL;
    float ttlStep = (maxTTL) / colorGradient.GetSize ();

    csRef<iDocumentNode> lineff = newParams->CreateNodeBefore (
      CS_NODE_ELEMENT, 0);
    lineff->SetValue ("effector");
    lineff->SetAttribute ("type", "lincolor");

    for (size_t i = 0; i < colorGradient.GetSize (); ++i)
    {
      csRef<iDocumentNode> colorNode = lineff->CreateNodeBefore (
        CS_NODE_ELEMENT, 0);
      colorNode->SetValue ("color");
      syntaxService->WriteColor (colorNode, colorGradient[i]);
      colorNode->SetAttributeAsFloat ("time", maxTTL-i*ttlStep);
    }
  }
}


int main (int argc, char* argv[])
{
  iObjectRegistry *objReg = csInitializer::CreateEnvironment (argc, argv);

  if (!objReg)
    return 1;
  
  if (!csInitializer::RequestPlugins (objReg,
    CS_REQUEST_VFS,
    CS_REQUEST_END))
  {
    return 1;
  }

  PartConv *pc = new PartConv (objReg);

  pc->Main ();
  delete pc;

  csInitializer::DestroyApplication (objReg);

  return 0;
}

