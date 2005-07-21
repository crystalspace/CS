# Author: David Goodger
# Contact: goodger@users.sourceforge.net
# Revision: $Revision$
# Date: $Date$
# Copyright: This module has been placed in the public domain.

"""
CSEP HTML Writer.
"""

__docformat__ = 'reStructuredText'


import sys
import docutils
from docutils import frontend, nodes, utils
from docutils.writers import html4css1


class Writer(html4css1.Writer):

    settings_spec = html4css1.Writer.settings_spec + (
        'CSEP/HTML-Specific Options',
        None,
        (('Specify a template file.  Default is "csep-html-template".',
          ['--template'],
          {'default': 'csep-html-template', 'metavar': '<file>'}),
         ('CrystalSpace\'s home URL.  Default is "http://www.crystalspace3d.org".',
          ['--crystal-home'],
          {'default': 'http://www.crystalspace3d.org', 'metavar': '<URL>'}),
         ('Home URL prefix for CSEPs.  Default is "." (current directory).',
          ['--csep-home'],
          {'default': '.', 'metavar': '<URL>'}),
         # For testing.
         (frontend.SUPPRESS_HELP,
          ['--no-random'],
          {'action': 'store_true', 'validator': frontend.validate_boolean}),))

    relative_path_settings = (html4css1.Writer.relative_path_settings
                              + ('template',))

    config_section = 'csep_html writer'
    config_section_dependencies = ('writers', 'html4css1 writer')

    def __init__(self):
        html4css1.Writer.__init__(self)
        self.translator_class = HTMLTranslator

    def translate(self):
        html4css1.Writer.translate(self)
        settings = self.document.settings
        template = open(settings.template).read()
        # Substitutions dict for template:
        subs = {}
        subs['encoding'] = settings.output_encoding
        subs['version'] = docutils.__version__
        subs['stylesheet'] = ''.join(self.stylesheet)
        cshome = settings.crystal_home
        subs['cshome'] = cshome
        subs['csephome'] = settings.csep_home
        if cshome == '..':
            subs['csepindex'] = '.'
        else:
            subs['csepindex'] = cshome + '/cseps'
        index = self.document.first_child_matching_class(nodes.field_list)
        header = self.document[index]
        csepnum = header[0][1].astext()
        subs['csep'] = csepnum
        if settings.no_random:
            subs['banner'] = 0
        else:
            import random
            subs['banner'] = random.randrange(64)
        try:
            subs['csepnum'] = '%04i' % int(csepnum)
        except ValueError:
            subs['csepnum'] = csepnum
        subs['title'] = header[1][1].astext()
        subs['body'] = ''.join(
            self.body_pre_docinfo + self.docinfo + self.body)
        subs['body_suffix'] = ''.join(self.body_suffix)
        self.output = template % subs


class HTMLTranslator(html4css1.HTMLTranslator):

    def depart_field_list(self, node):
        html4css1.HTMLTranslator.depart_field_list(self, node)
        if 'rfc2822' in node['classes']:
             self.body.append('<hr />\n')
