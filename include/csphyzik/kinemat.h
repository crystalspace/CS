#ifndef CT_KINEMATIC_ENITITY
#define CT_KINEMATIC_ENITITY

/**
 * kinematic entities are not controlled by newtons laws.
 * used primarily here as attachment points for entities in the physics module 
 * here and a users entities.
 */
class ctKinematicEntity
{
public:
  virtual ctVector3 get_x () = 0;
  virtual ctVector3 get_v () = 0;
  virtual ctVector3 get_angular_v () = 0;
  virtual ctVector3 get_a () = 0;
  virtual ctVector3 get_angular_a () = 0;
};


#endif
