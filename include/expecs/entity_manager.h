#pragma once

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <vector>

#ifndef EXPECS_MAX_COMPONENTS
#define EXPECS_MAX_COMPONENTS 64
#endif

namespace expecs
{
    using Entity = uint32_t;
    constexpr size_t MAX_COMPONENTS = EXPECS_MAX_COMPONENTS;
    using Signature = std::bitset<MAX_COMPONENTS>;

    class EntityManager
    {
    public:
        EntityManager() = default;
        ~EntityManager() = default;

        Entity createEntity()
        {
            Entity id;
            if (!_recycledEntities.empty())
            {
                id = _recycledEntities.back();
                _recycledEntities.pop_back();
            }
            else
            {
                id = _nextEntity++;
                if (id >= _signatures.size())
                {
                    _signatures.resize(std::max(_signatures.size() * 2, static_cast<size_t>(id) + 1));
                }
            }
            _aliveCount++;
            return id;
        }

        void destroyEntity(Entity entity)
        {
            _signatures[entity].reset();
            _recycledEntities.push_back(entity);
            _aliveCount--;
        }

        uint32_t getEntityCount() const
        {
            return _aliveCount;
        }

        Signature getSignature(Entity entity) const
        {
            return _signatures[entity];
        }

        void setSignature(Entity entity, Signature signature)
        {
            if (entity >= _signatures.size())
            {
                _signatures.resize(std::max(_signatures.size() * 2, static_cast<size_t>(entity) + 1));
            }
            _signatures[entity] = signature;
        }

    private:
        Entity _nextEntity = 0;
        std::vector<Entity> _recycledEntities = {};
        std::vector<Signature> _signatures = {};
        uint32_t _aliveCount = 0;
    };
} // namespace expecs