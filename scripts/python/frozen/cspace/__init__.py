# cspace package glue code
# The modules must be imported in inheritance order, that is,
# if module foo depends on bar, then bar must be imported before.

from core import *
from isndsys import *
from ivaria import *
from csgfx import *
from ivideo import *
from csgeom import *
from imesh import *
import animation
from iengine import *
from cstool import *
from imap import *
try:
    from pycscegui import *
except:
    pass

# Legacy identifiers
csSkeletalState2 = animation.AnimatedMeshState
iSkeleton2 = animation.iSkeleton
iSkeletonFactory2 = animation.iSkeletonFactory
iSkeletonManager2 = animation.iSkeletonManager
iSkeletonAnimation2  = animation.iSkeletonAnimation
iSkeletonAnimationNode2 = animation.iSkeletonAnimationNode
iSkeletonAnimationNodeFactory2 = animation.iSkeletonAnimationNodeFactory
iSkeletonAnimCallback2 = animation.iSkeletonAnimCallback
iSkeletonAnimNode2 = animation.iSkeletonAnimNode
iSkeletonAnimNodeFactory2 = animation.iSkeletonAnimNodeFactory
iSkeletonAnimPacket2 = animation.iSkeletonAnimPacket
iSkeletonAnimPacketFactory2 = animation.iSkeletonAnimPacketFactory
iSkeletonBlendNode2 = animation.iSkeletonBlendNode
iSkeletonBlendNodeFactory2 = animation.iSkeletonBlendNodeFactory
iSkeletonFSMNode2 = animation.iSkeletonFSMNode
iSkeletonFSMNodeFactory2 = animation.iSkeletonFSMNodeFactory
iSkeletonPriorityNode2 = animation.iSkeletonPriorityNode
iSkeletonPriorityNodeFactory2 = animation.iSkeletonPriorityNodeFactory
iSkeletonRandomNode2 = animation.iSkeletonRandomNode
iSkeletonRandomNodeFactory2 = animation.iSkeletonRandomNodeFactory
