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
csSkeletalState2 = animation.csSkeletalState
iSkeleton2 = animation.iSkeleton
iSkeletonFactory2 = animation.iSkeletonFactory
iSkeletonManager2 = animation.iSkeletonManager
iSkeletonAnimation2  = animation.iSkeletonAnimation
iSkeletonAnimationNode2 =  iSkeletonAnimationNode
iSkeletonAnimationNodeFactory2 =  iSkeletonAnimationNodeFactory
iSkeletonAnimCallback2 =  iSkeletonAnimCallback
iSkeletonAnimNode2 =  iSkeletonAnimNode
iSkeletonAnimNodeFactory2 =  iSkeletonAnimNodeFactory
iSkeletonAnimPacket2 =  iSkeletonAnimPacket
iSkeletonAnimPacketFactory2 =  iSkeletonAnimPacketFactory
iSkeletonBlendNode2 =  iSkeletonBlendNode
iSkeletonBlendNodeFactory2 =  iSkeletonBlendNodeFactory
iSkeletonFSMNode2 =  iSkeletonFSMNode
iSkeletonFSMNodeFactory2 =  iSkeletonFSMNodeFactory
iSkeletonPriorityNode2 =  iSkeletonPriorityNode
iSkeletonPriorityNodeFactory2 =  iSkeletonPriorityNodeFactory
iSkeletonRandomNode2 =  iSkeletonRandomNode
iSkeletonRandomNodeFactory2 = iSkeletonRandomNodeFactory
