#include <btBulletDynamicsCommon.h>
#include <iostream>

struct PlayerContactCallback : public btCollisionWorld::ContactResultCallback
{
  btRigidBody *player;

  PlayerContactCallback(btRigidBody *playerBody) : player(playerBody) {}

  btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0, int partId0, int index0, const btCollisionObjectWrapper *colObj1, int partId1, int index1) override
  {
    if (colObj0->getCollisionObject() == player || colObj1->getCollisionObject() == player)
    {
      btVector3 normal = cp.m_normalWorldOnB;

      if (std::abs(normal.getY()) < 0.9f)
      {
        btVector3 velocity = player->getLinearVelocity();

        if (velocity.dot(normal) > 0.0f)
        {
          normal = -normal;
        }

        btVector3 impulse = -velocity.dot(normal) * normal;

        impulse.setY(impulse.getY());

        player->applyCentralImpulse(impulse);
      }
    }
    return 0;
  }
};