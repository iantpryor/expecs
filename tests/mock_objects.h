#pragma once
#include "expecs/registry.h"

namespace expecs
{
    // Test components
    struct Position
    {
        float x, y, z;

        Position() : x(0), y(0), z(0) {}
        Position(float x, float y, float z) : x(x), y(y), z(z) {}

        bool operator==(const Position& other) const 
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct Velocity
    {
        float dx, dy, dz;

        Velocity() : dx(0), dy(0), dz(0) {}
        Velocity(float dx, float dy, float dz) : dx(dx), dy(dy), dz(dz) {}

        bool operator==(const Velocity& other) const 
        {
            return dx == other.dx && dy == other.dy && dz == other.dz;
        }
    };

    struct Health
    {
        int hp;
        int maxHp;

        Health() : hp(100), maxHp(100) {}
        Health(int hp, int maxHp) : hp(hp), maxHp(maxHp) {}

        bool operator==(const Health& other) const 
        {
            return hp == other.hp && maxHp == other.maxHp;
        }
    };

    // Test system
    class MovementSystem : public System 
    {
    public:
        void update() 
        {
            for (Entity entity : getEntities()) 
            {
                auto& pos = getRegistry()->getComponent<Position>(entity);
                auto& vel = getRegistry()->getComponent<Velocity>(entity);

                pos.x += vel.dx;
                pos.y += vel.dy;
                pos.z += vel.dz;
            }
        }

        void entityAdded(Entity entity) override 
        {
            entitiesAdded.insert(entity);
        }

        void entityRemoved(Entity entity) override 
        {
            entitiesRemoved.insert(entity);
        }

        std::unordered_set<Entity> entitiesAdded;
        std::unordered_set<Entity> entitiesRemoved;
    };
}