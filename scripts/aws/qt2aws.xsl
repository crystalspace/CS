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


  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <xsl:template match="/UI">
    <xsl:choose>
      <xsl:when test="./class">
        <xsl:text>window "</xsl:text><xsl:value-of select="./class"/><xsl:text>"</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>window "</xsl:text><xsl:value-of select="./widget/property[name[text()='name']]/cstring"/><xsl:text>"</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:apply-templates />
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>

  </xsl:template>
  
  <xsl:template match="widget[class[text()='QDialog']]">
    <!-- set the title if any -->
    <xsl:call-template name="style"/>
    <xsl:call-template name="options"/>
    <xsl:apply-templates select="widget"/>
  </xsl:template>


  <xsl:template match="widget[class[text()='QPushButton' or text()='QRadioButton' or text()='QButtonGroup' or text()='QGroupBox' or text()='QCheckBox' or text()='QLineEdit' or text()='QLabel']]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>
 
  <xsl:template match="widget[class[text()='QListBox']]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <!-- configure a awslistbox with a single column -->
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Type: lbtList</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Highlight: "/aws/lbhi.png"</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Columns: 1</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>DefaultSortCol: 0</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Column0Width: </xsl:text><xsl:value-of select="property/geometry/rect/width"/>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Column0Caption: "Column 0"</xsl:text>
    <!-- we ignore scrollbar stuff since aws always creates a vertical one (currently) -->
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="widget[class[text()='QListView']]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Highlight: "/aws/lbhi.png"</xsl:text>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>Columns: </xsl:text><xsl:value-of select="count(column)"/>
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="class"/></xsl:call-template><xsl:text>DefaultSortCol: 0</xsl:text>
    <!-- we ignore scrollbar stuff since aws always creates a vertical one (currently) -->
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="column">
    <xsl:call-template name="spacer"/><xsl:value-of select="concat('Column',count(preceding-sibling::node()[self::column]),'Width: ')"/><xsl:value-of select="round(number(../property/rect/width) div count(../column))"/>
    <xsl:call-template name="spacer"/><xsl:value-of select="concat('Column',count(preceding-sibling::node()[self::column]),'Caption: ')"/><xsl:text>"</xsl:text><xsl:value-of select="property[name='text']/string"/><xsl:text>"</xsl:text>
  </xsl:template>
  
  <xsl:template match="property[name[text()='caption']]">
    <xsl:call-template name="spacer"/><xsl:text>Title: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='text' or text()='title'] and ../class[text() != 'QLineEdit']]">
    <xsl:call-template name="spacer"/><xsl:text>Caption: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='text'] and ../class[text()='QLineEdit']]">
    <xsl:call-template name="spacer"/><xsl:text>Text: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='pixmap'] and ../class[text()='QLabel']]">
    <xsl:call-template name="spacer"/><xsl:text>Texture: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='geometry']]">
    <xsl:call-template name="spacer"/><xsl:text>Frame: (</xsl:text><xsl:value-of select="rect/x"/><xsl:text>,</xsl:text><xsl:value-of select="rect/y"/><xsl:text>) - (</xsl:text><xsl:value-of select="number(rect/x)+number(rect/width)"/><xsl:text>,</xsl:text><xsl:value-of select="number(rect/y)+number(rect/height)"/><xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='echoMode']]">
    <xsl:choose>
      <xsl:when test="enum='NoEcho'">
        <xsl:call-template name="spacer"/><xsl:text>Masked: Yes</xsl:text>
      </xsl:when>
      <xsl:when test="enum='Password'">
        <xsl:call-template name="spacer"/><xsl:text>Masked: Yes</xsl:text>
        <xsl:call-template name="spacer"/><xsl:text>MaskChar: "*"</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="property[name[text()='frameShadow']]">
    <xsl:call-template name="spacer"/><xsl:text>Style: </xsl:text>
    <xsl:choose>
      <xsl:when test="enum='Raised'">
        <xsl:call-template name="prefix"/><xsl:text>fsRaised</xsl:text>
      </xsl:when>
      <xsl:when test="enum='Plain'">
        <xsl:call-template name="prefix"/><xsl:text>fsSimple</xsl:text>
      </xsl:when>
      <xsl:when test="enum='Sunken'">
        <xsl:call-template name="prefix"/><xsl:text>fsSunken</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="property[name[text()='alignment']]">
    <xsl:call-template name="spacer"/><xsl:text>Align: </xsl:text>
    <xsl:choose>
      <xsl:when test="contains(set,'AlignCenter')">
        <xsl:call-template name="prefix"/><xsl:text>AlignCenter</xsl:text>
      </xsl:when>
      <xsl:when test="contains(set,'AlignLeft')">
        <xsl:call-template name="prefix"/><xsl:text>AlignLeft</xsl:text>
      </xsl:when>
      <xsl:when test="contains(set,'AlignRight')">
        <xsl:call-template name="prefix"/><xsl:text>AlignRight</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
    
  <xsl:template name="style">
    <xsl:call-template name="spacer"/><xsl:text>Style: </xsl:text><xsl:call-template name="prefix"/><xsl:text>fsNormal</xsl:text>
  </xsl:template>

  <xsl:template name="options">
    <xsl:choose>
      <xsl:when test="class[text()='QDialog']">
        <xsl:call-template name="spacer"/><xsl:text>Options: wfoBeveledBorder+wfoGrip+wfoTitle+wfoClose+wfoMin+wfoZoom+wfoControl</xsl:text>
      </xsl:when>
      <xsl:when test="class[text()='QWidget']">
        <xsl:call-template name="spacer"/><xsl:text>Options: wfoTitle+wfoClose</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="component_header">
    <xsl:text>component "</xsl:text><xsl:value-of select="property[name[text()='name']]/cstring"/><xsl:text>" is </xsl:text><xsl:call-template name="get_classname"/>
  </xsl:template>

  <xsl:template name="get_classname">
    <xsl:choose>
      <xsl:when test="class='QPushButton'">"Command Button"</xsl:when>
      <xsl:when test="class='QRadioButton'">"Radio Button"</xsl:when>
      <xsl:when test="class='QButtonGroup'">"Group Frame"</xsl:when>
      <xsl:when test="class='QGroupBox'">"Group Frame"</xsl:when>
      <xsl:when test="class='QCheckBox'">"Check Box"</xsl:when>
      <xsl:when test="class='QListBox'">"List Box"</xsl:when>
      <xsl:when test="class='QListView'">"List Box"</xsl:when>
      <xsl:when test="class='QLabel' and property/name='text'">"Label"</xsl:when>
      <xsl:when test="class='QLabel' and property/name='pixmap'">"Image View"</xsl:when>
      <xsl:otherwise><xsl:value-of select="class"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="prefix">
    <xsl:choose>
      <xsl:when test="class='QWidget' or class='QDialog'"><xsl:text>w</xsl:text></xsl:when>
      <xsl:when test="../class='QPushButton' or class='QPushButton'"><xsl:text>b</xsl:text></xsl:when>
      <xsl:when test="../class='QButtonGroup'"><xsl:text>g</xsl:text></xsl:when>
      <xsl:when test="../class='QGroupBox'"><xsl:text>g</xsl:text></xsl:when>
      <xsl:when test="../class='QListBox'"><xsl:text>lb</xsl:text></xsl:when>
      <xsl:when test="../class='QListView'"><xsl:text>lb</xsl:text></xsl:when>
      <xsl:when test="../class='QLabel' and ../property/name='pixmap'"><xsl:text>iv</xsl:text></xsl:when>
      <xsl:when test="../class='QLabel' and ../property/name='text'"><xsl:text>lbl</xsl:text></xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="spacer">
    <xsl:param name="here" select="."/>
    <xsl:param name="prefix"><xsl:text>&#10;</xsl:text></xsl:param>
    <xsl:value-of select="$prefix"/>
    <xsl:if test="$here/../..">
      <xsl:text>  </xsl:text>
      <xsl:call-template name="spacer">
        <xsl:with-param name="here" select="$here/.."/>
        <xsl:with-param name="prefix"></xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="widget"/>
  <xsl:template match="class"/>
  <xsl:template match="property"/>
  <xsl:template match="text()"/>

</xsl:stylesheet>

