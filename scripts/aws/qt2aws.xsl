<?xml version="1.0"?>
<!--
    Copyright (C) 2002 by Norman Krämer
  
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
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <xsl:template match="/UI">

    <xsl:choose>
      <xsl:when test="./class">
        window "<xsl:value-of select="./class"/>"
      </xsl:when>
      <xsl:otherwise>
        window "<xsl:value-of select="./widget/property[name[text()='name']]/cstring"/>"
      </xsl:otherwise>
    </xsl:choose>
    <xsl:apply-templates />

  </xsl:template>

  <xsl:template match="widget[class[text()='QDialog']]">
    {
    <!-- set the title if any -->
    <xsl:call-template name="style"><xsl:with-param name="prefix" select="'w'"/></xsl:call-template>
    <xsl:call-template name="options"/>
    <xsl:apply-templates select="widget"/>
    }
  </xsl:template>

  <xsl:template match="widget[class[text()='QPushButton']]">
    <xsl:call-template name="component_header"/>
    {
    <xsl:call-template name="style"><xsl:with-param name="prefix" select="'b'"/></xsl:call-template>
    <xsl:apply-templates/>
    }
  </xsl:template>

  <xsl:template match="widget[class[text()='QRadioButton' or text()='QButtonGroup' or text()='QGroupBox' or text()='QCheckBox' or text()='QLineEdit' or text()='QLabel']]">
    <xsl:call-template name="component_header"/>
    {
    <xsl:apply-templates/>
    }
  </xsl:template>

  <xsl:template match="property[name[text()='caption']]">
    Title: "<xsl:value-of select="string"/>"
  </xsl:template>

  <xsl:template match="property[name[text()='text' or text()='title'] and ../class[text() != 'QLineEdit']]">
    Caption: "<xsl:value-of select="string"/>"
  </xsl:template>

  <xsl:template match="property[name[text()='text'] and ../class[text()='QLineEdit']]">
    Text: "<xsl:value-of select="string"/>"
  </xsl:template>

  <xsl:template match="property[name[text()='geometry']]">
    Frame: (<xsl:value-of select="rect/x"/>,<xsl:value-of select="rect/y"/>) - (<xsl:value-of select="number(rect/x)+number(rect/width)"/>,<xsl:value-of select="number(rect/y)+number(rect/height)"/>)
  </xsl:template>

  <xsl:template match="property[name[text()='echoMode']]">
    <xsl:choose>
      <xsl:when test="enum='NoEcho'">
        Masked: Yes
      </xsl:when>
      <xsl:when test="enum='Password'">
        Masked: Yes
        MaskChar: "*"
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="property[name[text()='alignment']]">
    <xsl:if test="../class[text()='QLabel']">
      <xsl:choose>
        <xsl:when test="contains(set,'AlignCenter')">
          Align: lblAlignCenter
        </xsl:when>
        <xsl:when test="contains(set,'AlignLeft')">
          Align: lblAlignLeft
        </xsl:when>
        <xsl:when test="contains(set,'AlignRight')">
          Align: lblAlignRight
        </xsl:when>
      </xsl:choose>
    </xsl:if>
  </xsl:template>
    
  <xsl:template name="style">
    <xsl:param name="prefix"/>
    Style: <xsl:value-of select="$prefix"/>fsNormal
  </xsl:template>

  <xsl:template name="options">
    <xsl:choose>
      <xsl:when test="class[text()='QDialog']">
        Options: wfoBeveledBorder+wfoGrip+wfoTitle+wfoClose+wfoMin+wfoZoom+wfoControl
      </xsl:when>
      <xsl:when test="class[text()='QWidget']">
        Options: wfoTitle+wfoClose
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="component_header">
    component "<xsl:value-of select="property[name[text()='name']]/cstring"/>" is <xsl:call-template name="get_classname"/>
  </xsl:template>

  <xsl:template name="get_classname">
    <xsl:choose>
      <xsl:when test="class='QPushButton'">"Command Button"</xsl:when>
      <xsl:when test="class='QRadioButton'">"Radio Button"</xsl:when>
      <xsl:when test="class='QButtonGroup'">"Group Frame"</xsl:when>
      <xsl:when test="class='QGroupBox'">"Group Frame"</xsl:when>
      <xsl:when test="class='QCheckBox'">"Check Box"</xsl:when>
      <xsl:when test="class='QLineEdit'">"Text Box"</xsl:when>
      <xsl:when test="class='QLabel'">"Label"</xsl:when>
      <xsl:otherwise><xsl:value-of select="class"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="class"/>
  <xsl:template match="property"/>

</xsl:stylesheet>

