# Author: David Goodger
# Contact: goodger@users.sourceforge.net
# Revision: $Revision$
# Date: $Date$
# Copyright: This module has been placed in the public domain.

"""
CrystalSpace  Enhancement Proposal (CSEP) Reader.
"""

__docformat__ = 'reStructuredText'


from docutils.readers import standalone
from docutils.transforms import references, misc
from docutils.parsers import rst
import cseps

class Reader(standalone.Reader):

    supported = ('csep',)
    """Contexts this reader supports."""

    settings_spec = (
        'CSEP Reader Option Defaults',
        'The --csep-references and --rfc-references options (for the '
        'reStructuredText parser) are on by default.',
        ())

    config_section = 'csep reader'
    config_section_dependencies = ('readers', 'standalone reader')

    default_transforms = (references.Substitutions,
                          references.PropagateTargets,
                          cseps.Headers,
                          cseps.Contents,
                          references.AnonymousHyperlinks,
                          references.IndirectHyperlinks,
                          cseps.TargetNotes,
                          references.Footnotes,
                          references.ExternalTargets,
                          references.InternalTargets,
                          references.DanglingReferences,
                          misc.Transitions,
                          )

    settings_default_overrides = {'csep_references': 1, 'rfc_references': 1}

    inliner_class = rst.states.Inliner

    def __init__(self, parser=None, parser_name=None):
        """`parser` should be ``None``."""
        if parser is None:
            parser = rst.Parser(rfc2822=1, inliner=self.inliner_class())
        standalone.Reader.__init__(self, parser, '')
