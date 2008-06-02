<?xml version="1.0"?>
<!--
    Copyright (C) 2002 by Norman Kraemer
  
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
  <xsl:variable name="tabwidth" select="'  '"/>

  <!-- the top level widget types allowed -->
  <xsl:template match="widget[class[text()='QDialog' or text()='QWidget'] and not(../class='QTabWidget')]">
    <xsl:call-template name="spacer"/><xsl:text>window "</xsl:text>
    <xsl:choose>
      <xsl:when test="/UI/class"><xsl:value-of select="/UI/class"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="property[name[text()='name']]/cstring"/></xsl:otherwise>
    </xsl:choose>
    <xsl:text>"</xsl:text>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <!-- set the title if any -->
    <xsl:call-template name="style"/>
    <xsl:call-template name="options"/>
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <!-- list of widgets we convert -->
  <xsl:template match="widget[class[text()='QPushButton' or text()='QRadioButton' or text()='QSlider' or text()='QButtonGroup' or text()='QGroupBox' or text()='QFrame' or text()='QCheckBox' or text()='QLineEdit' or text()='QLabel' or text()='QTextView' or text()='QTabWidget'] or (class='QWidget' and ../class='QTabWidget')]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:call-template name="layout"/>
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>
 
  <!-- special treatment of QListBox that maps to List Box with one column -->
  <xsl:template match="widget[class[text()='QListBox']]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <!-- configure a awslistbox with a single column -->
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Type: lbtList</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Highlight: "/aws/lbhi.png"</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Columns: 1</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>DefaultSortCol: 0</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Column0Width: </xsl:text><xsl:value-of select="property/rect/width"/>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Column0Caption: "Column 0"</xsl:text>
    <!-- we ignore scrollbar stuff since aws always creates a vertical one (currently) -->
    <xsl:call-template name="layout"/>
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="widget[class[text()='QListView']]">
    <xsl:call-template name="spacer"/><xsl:call-template name="component_header"/>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Highlight: "/aws/lbhi.png"</xsl:text>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>Columns: </xsl:text><xsl:value-of select="count(column)"/>
    <xsl:call-template name="spacer">
      <xsl:with-param name="here" select="class"/>
    </xsl:call-template>
    <xsl:text>DefaultSortCol: 0</xsl:text>
    <!-- we ignore scrollbar stuff since aws always creates a vertical one (currently) -->
    <xsl:call-template name="layout"/>
    <xsl:apply-templates/>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="column">
    <xsl:call-template name="spacer"/>
    <xsl:value-of select="concat('Column',count(preceding-sibling::node()[self::column]),'Width: ')"/>
    <xsl:value-of select="round(number(../property/rect/width) div count(../column))"/>
    <xsl:call-template name="spacer"/>
    <xsl:value-of select="concat('Column',count(preceding-sibling::node()[self::column]),'Caption: ')"/>
    <xsl:text>"</xsl:text><xsl:value-of select="property[name='text']/string"/><xsl:text>"</xsl:text>
  </xsl:template>
  
  <xsl:template match="property[name[text()='caption']]">
    <xsl:call-template name="spacer"/><xsl:text>Title: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='text' or text()='title'] and ../class[text() != 'QLineEdit']]">
    <xsl:if test="string-length(string) > 0">
      <xsl:call-template name="spacer"/><xsl:text>Caption: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="property[name[text()='text'] and ../class[text()='QLineEdit']]">
    <xsl:call-template name="spacer"/><xsl:text>Text: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='pixmap'] and ../class[text()='QLabel']]">
    <!--
    <xsl:call-template name="spacer"/><xsl:text>Texture: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
    -->
  </xsl:template>

  <xsl:template match="property[name[text()='geometry']]">
    <xsl:call-template name="spacer"/><xsl:text>Frame: (</xsl:text>
    <xsl:value-of select="rect/x"/><xsl:text>,</xsl:text><xsl:value-of select="rect/y"/>
    <xsl:text>) - (</xsl:text>
    <xsl:value-of select="number(rect/x)+number(rect/width)"/><xsl:text>,</xsl:text>
    <xsl:value-of select="number(rect/y)+number(rect/height)"/><xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template match="property[name[text()='minValue']]">
    <xsl:call-template name="spacer"/><xsl:text>Min: </xsl:text><xsl:value-of select="number"/>
  </xsl:template>

  <xsl:template match="property[name[text()='maxValue']]">
    <xsl:call-template name="spacer"/><xsl:text>Max: </xsl:text><xsl:value-of select="number"/>
  </xsl:template>

  <xsl:template match="property[name[text()='pageStep']]">
    <xsl:call-template name="spacer"/><xsl:text>PageSize: </xsl:text><xsl:value-of select="number"/>
  </xsl:template>

  <xsl:template match="property[name[text()='value']]">
    <xsl:call-template name="spacer"/><xsl:text>Value: </xsl:text><xsl:value-of select="number"/>
  </xsl:template>

  <xsl:template match="property[name[text()='orientation']]">
    <xsl:call-template name="spacer"/><xsl:text>Orientation: </xsl:text>
    <xsl:choose>
      <xsl:when test="enum='Horizontal'">
        <xsl:call-template name="prefix"/><xsl:text>oHorizontal</xsl:text>
      </xsl:when>
      <xsl:when test="enum='Vertical'">
        <xsl:call-template name="prefix"/><xsl:text>oVertical</xsl:text>
      </xsl:when>
    </xsl:choose>
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

  <xsl:template match="property[name[text()='frameShadow'] and not(../class='QLabel')]">
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

  <xsl:template match="grid">
    <xsl:call-template name="spacer"/><xsl:text>Layout: "GridBag"</xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="property[name[text()='sizePolicy']]">
    <xsl:choose>
      <xsl:when test="sizepolicy/hsizetype=0 and sizepolicy/vsizetype=0">
        <xsl:call-template name="spacer"/><xsl:text>Fill: gbcNone</xsl:text>
      </xsl:when>
      <xsl:when test="not(sizepolicy/hsizetype=0) and sizepolicy/vsizetype=0">
        <xsl:call-template name="spacer"/><xsl:text>Fill: gbcHorizontal</xsl:text>
      </xsl:when>
      <xsl:when test="sizepolicy/hsizetype=0 and not(sizepolicy/vsizetype=0)">
        <xsl:call-template name="spacer"/><xsl:text>Fill: gbcVertical</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="spacer"/><xsl:text>Fill: gbcBoth</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="property[name='whatsThis']">
    <xsl:call-template name="awsinfo">
      <xsl:with-param name="info" select="concat(string,'|')"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="attribute[name='title']">
    <xsl:if test="string-length(string) > 0">
      <xsl:call-template name="spacer"/>
      <xsl:text>Caption: "</xsl:text><xsl:value-of select="string"/><xsl:text>"</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="layout">
    <xsl:if test="name(..)='grid'">
      <xsl:call-template name="spacer"><xsl:with-param name="here" select="./property"/></xsl:call-template>
      <xsl:text>Anchor: gbcCenter</xsl:text>
      <xsl:if test="@column">
        <xsl:call-template name="spacer"><xsl:with-param name="here" select="./property"/></xsl:call-template>
        <xsl:text>GridX: </xsl:text><xsl:value-of select="number(@column)"/>
      </xsl:if>
      <xsl:if test="@row">
        <xsl:call-template name="spacer"><xsl:with-param name="here" select="./property"/></xsl:call-template>
        <xsl:text>GridY: </xsl:text><xsl:value-of select="number(@row)"/>
      </xsl:if>
      <xsl:if test="@colspan">
        <xsl:call-template name="spacer"><xsl:with-param name="here" select="./property"/></xsl:call-template>
        <xsl:text>GridWidth: </xsl:text><xsl:value-of select="number(@colspan)"/>
      </xsl:if>
      <xsl:if test="@rowspan">
        <xsl:call-template name="spacer"><xsl:with-param name="here" select="./property"/></xsl:call-template>
        <xsl:text>GridHeight: </xsl:text><xsl:value-of select="number(@rowspan)"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="style">
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="./class"/></xsl:call-template>
    <xsl:text>Style: </xsl:text><xsl:call-template name="prefix"/><xsl:text>fsNormal</xsl:text>
  </xsl:template>

  <xsl:template name="options">
    <xsl:call-template name="spacer"><xsl:with-param name="here" select="./class"/></xsl:call-template>
    <xsl:choose>
      <xsl:when test="class[text()='QDialog']">
        <xsl:text>Options: wfoGrip+wfoTitle+wfoClose+wfoMin+wfoZoom+wfoControl</xsl:text>
      </xsl:when>
      <xsl:when test="class[text()='QWidget']">
        <xsl:text>Options: wfoTitle+wfoClose</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="component_header">
    <xsl:text>component "</xsl:text>
    <xsl:value-of select="property[name[text()='name']]/cstring"/>
    <xsl:text>" is </xsl:text><xsl:call-template name="get_classname"/>
  </xsl:template>

  <xsl:template name="get_classname">
    <xsl:choose>
      <xsl:when test="contains(concat('|',property[name='whatsThis']/string),'|t:')">
        <xsl:call-template name="awsinfo">
          <xsl:with-param name="info" select="concat(property[name='whatsThis']/string,'|')"/>
          <xsl:with-param name="process" select="'t'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="class='QTabWidget'">"Notebook"</xsl:when>
      <xsl:when test="class='QWidget' and ../class='QTabWidget'">"Notebook Page"</xsl:when>
      <xsl:when test="class='QPushButton'">"Command Button"</xsl:when>
      <xsl:when test="class='QRadioButton'">"Radio Button"</xsl:when>
      <xsl:when test="class='QButtonGroup'">"Group Frame"</xsl:when>
      <xsl:when test="class='QGroupBox'">"Group Frame"</xsl:when>
      <xsl:when test="class='QFrame'">"Group Frame"</xsl:when>
      <xsl:when test="class='QLineEdit'">"Text Box"</xsl:when>
      <xsl:when test="class='QTextView'">"Multiline Edit"</xsl:when>
      <xsl:when test="class='QCheckBox'">"Check Box"</xsl:when>
      <xsl:when test="class='QListBox'">"List Box"</xsl:when>
      <xsl:when test="class='QListView'">"List Box"</xsl:when>
      <xsl:when test="class='QSlider'">"Scroll Bar"</xsl:when>
      <xsl:when test="class='QLabel' and property/name='text'">"Label"</xsl:when>
      <xsl:when test="class='QLabel' and property/name='pixmap'">"Image View"</xsl:when>
      <xsl:otherwise><xsl:value-of select="class"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="prefix">
    <xsl:choose>
      <xsl:when test="../class='QWidget' and ../../class='QTabWidget'"><xsl:text>nb</xsl:text></xsl:when>
      <xsl:when test="class='QWidget' or class='QDialog'"><xsl:text>w</xsl:text></xsl:when>
      <xsl:when test="../class='QPushButton' or class='QPushButton'"><xsl:text>b</xsl:text></xsl:when>
      <xsl:when test="../class='QButtonGroup'"><xsl:text>g</xsl:text></xsl:when>
      <xsl:when test="../class='QGroupBox'"><xsl:text>g</xsl:text></xsl:when>
      <xsl:when test="../class='QFrame'"><xsl:text>g</xsl:text></xsl:when>
      <xsl:when test="../class='QListBox'"><xsl:text>lb</xsl:text></xsl:when>
      <xsl:when test="../class='QListView'"><xsl:text>lb</xsl:text></xsl:when>
      <xsl:when test="../class='QSlider'"><xsl:text>sb</xsl:text></xsl:when>
      <xsl:when test="../class='QTextView'"><xsl:text>me</xsl:text></xsl:when>
      <xsl:when test="../class='QLabel' and ../property/name='pixmap'"><xsl:text>iv</xsl:text></xsl:when>
      <xsl:when test="../class='QLabel' and ../property/name='text'"><xsl:text>lbl</xsl:text></xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="spacer">
    <xsl:param name="here" select="."/>
    <xsl:param name="prefix"><xsl:text>&#10;</xsl:text></xsl:param>
    <xsl:value-of select="$prefix"/>
    <xsl:if test="$here/../../..">
      <xsl:if test="not(name($here/..) = 'grid')">
        <xsl:value-of select="$tabwidth"/>
      </xsl:if>
      <xsl:call-template name="spacer">
        <xsl:with-param name="here" select="$here/.."/>
        <xsl:with-param name="prefix"></xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="awsinfo">
    <xsl:param name="info"/>
    <xsl:param name="process"><xsl:text>cl</xsl:text></xsl:param>
    <xsl:param name="sepa"><xsl:text>|</xsl:text></xsl:param>
    <xsl:param name="idsepa"><xsl:text>:</xsl:text></xsl:param>
    <xsl:variable name="token" select="substring-before($info,$sepa)"/>
    <xsl:if test="string-length($token)>0">
      <xsl:variable name="tokenid" select="substring-before($token,$idsepa)"/>
      <xsl:if test="contains($process,$tokenid)">
        <xsl:choose>
          <xsl:when test="$tokenid='c'">
            <xsl:call-template name="do_connect">
              <xsl:with-param name="inp" select="substring-after($token,$idsepa)"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="$tokenid='l'">
            <xsl:call-template name="do_literal">
              <xsl:with-param name="inp" select="substring-after($token,$idsepa)"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="$tokenid='t'">
            <xsl:call-template name="do_classtype">
              <xsl:with-param name="inp" select="substring-after($token,$idsepa)"/>
            </xsl:call-template>
          </xsl:when>
        </xsl:choose>
      </xsl:if>
      <xsl:call-template name="awsinfo">
        <xsl:with-param name="info" select="substring-after($info,$sepa)"/>
        <xsl:with-param name="process" select="$process"/>
        <xsl:with-param name="sepa" select="$sepa"/>
        <xsl:with-param name="idsepa" select="$idsepa"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="do_literal">
    <xsl:param name="inp"/>
    <xsl:call-template name="spacer"/><xsl:value-of select="$inp"/>
  </xsl:template>

  <xsl:template name="do_classtype">
    <xsl:param name="inp"/>
    <xsl:text>"</xsl:text><xsl:value-of select="$inp"/><xsl:text>"</xsl:text>
  </xsl:template>

  <xsl:template name="do_connect">
    <xsl:param name="sepa"><xsl:text>,</xsl:text></xsl:param>
    <xsl:param name="inp"/>
    <xsl:call-template name="spacer"/><xsl:text>connect</xsl:text>
    <xsl:call-template name="spacer"/><xsl:text>{</xsl:text>
    <xsl:call-template name="connect">
      <xsl:with-param name="inp" select="concat($inp,$sepa)"/>
      <xsl:with-param name="sepa" select="$sepa"/>
    </xsl:call-template>
    <xsl:call-template name="spacer"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template name="connect">
    <!-- tokenizes a string and for token of the form "from,to,from1,to1" 
         it places connection entries ala "from -> to" and "from1 -> to1" in the output
    -->
    <xsl:param name="sepa"><xsl:text>,</xsl:text></xsl:param>
    <xsl:param name="inp"/>
    <xsl:variable name="from" select="substring-before($inp,$sepa)"/>
    <xsl:variable name="inp" select="substring-after($inp,$sepa)"/>
    <xsl:variable name="to" select="substring-before($inp,$sepa)"/>
    <xsl:if test="string-length($from)>0 and string-length($to)>0">
      <xsl:call-template name="spacer"/><xsl:value-of select="concat($tabwidth,$from,' -> ',$to)"/>
    </xsl:if>
    <xsl:variable name="inp" select="substring-after($inp,$sepa)"/>
    <xsl:if test="string-length($inp)>0">
      <xsl:call-template name="connect">
        <xsl:with-param name="sepa" select="$sepa"/>   
        <xsl:with-param name="inp" select="$inp"/>   
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="widget"/>
  <xsl:template match="class"/>
  <xsl:template match="property"/>
  <xsl:template match="text()"/>

</xsl:stylesheet>

