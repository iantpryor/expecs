#pragma once

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#ifndef EXPECS_MAX_COMPONENTS
#define EXPECS_MAX_COMPONENTS 64
#endif

#ifndef EXPECS_ENTITY_INDEX_BITS
#define EXPECS_ENTITY_INDEX_BITS 20
#endif

namespace expecs
{
    constexpr size_t MAX_COMPONENTS = EXPECS_MAX_COMPONENTS;
    using Signature = std::bitset<MAX_COMPONENTS>;

    constexpr uint32_t ENTITY_INDEX_BITS = EXPECS_ENTITY_INDEX_BITS;
    constexpr uint32_t ENTITY_GENERATION_BITS = 32 - ENTITY_INDEX_BITS;
    static_assert(ENTITY_INDEX_BITS > 0 && ENTITY_INDEX_BITS < 32, "EXPECS_ENTITY_INDEX_BITS must be between 1 and 31");

    constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1;
    constexpr uint32_t MAX_GENERATION = (1u << ENTITY_GENERATION_BITS) - 1;
    constexpr uint32_t MAX_ENTITIES = ENTITY_INDEX_MASK;

    struct Entity
    {
        uint32_t id = ~0u;

        constexpr Entity() = default;
        constexpr explicit Entity(uint32_t index, uint32_t generation = 0) : id((generation << ENTITY_INDEX_BITS) | index)
        {
            assert(index <= ENTITY_INDEX_MASK && generation <= MAX_GENERATION && "Entity index or generation out of range");
        }

        constexpr uint32_t index() const { return id & ENTITY_INDEX_MASK; }
        constexpr uint32_t generation() const { return id >> ENTITY_INDEX_BITS; }

        constexpr bool operator==(const Entity& other) const = default;
    };

    constexpr Entity INVALID_ENTITY = Entity(ENTITY_INDEX_MASK, MAX_GENERATION);

    class EntityManager
    {
    public:
        EntityManager() = default;
        ~EntityManager() = default;

        Entity createEntity()
        {
            uint32_t index;
            if (!_recycledIndices.empty())
            {
                index = _recycledIndices.back();
                _recycledIndices.pop_back();
            }
            else
            {
                if (_nextIndex >= MAX_ENTITIES)
                    throw std::length_error("Exceeded maximum entities");

                index = _nextIndex++;
                if (index >= _signatures.size())
                {
                    size_t newSize = std::min(std::max(_signatures.size() * 2, static_cast<size_t>(index) + 1), static_cast<size_t>(MAX_ENTITIES));
                    _signatures.resize(newSize);
                    _generations.resize(newSize, DEAD_SLOT_FLAG);
                }
            }
            _generations[index] &= ~DEAD_SLOT_FLAG;
            _aliveCount++;
            return Entity(index, _generations[index]);
        }

        void destroyEntity(Entity entity)
        {
            assert(isAlive(entity) && "Entity is not alive");

            uint32_t index = entity.index();
            uint32_t generation = _generations[index];
            _signatures[index].reset();
            _aliveCount--;

            if (generation < MAX_GENERATION)
            {
                generation++;
                _recycledIndices.push_back(index);
            }
            _generations[index] = generation | DEAD_SLOT_FLAG;
        }

        bool isAlive(Entity entity) const
        {
            uint32_t index = entity.index();
            return index < _generations.size() && _generations[index] == entity.generation();
        }

        uint32_t getEntityCount() const
        {
            return _aliveCount;
        }

        Signature getSignature(Entity entity) const
        {
            assert(isAlive(entity) && "Entity is not alive");
            return _signatures[entity.index()];
        }

        void setSignature(Entity entity, Signature signature)
        {
            assert(isAlive(entity) && "Entity is not alive");
            _signatures[entity.index()] = signature;
        }

    private:
        static constexpr uint32_t DEAD_SLOT_FLAG = 1u << 31;

        uint32_t _nextIndex = 0;
        std::vector<uint32_t> _recycledIndices = {};
        std::vector<uint32_t> _generations = {};
        std::vector<Signature> _signatures = {};
        uint32_t _aliveCount = 0;
    };
} // namespace expecs

template <>
struct std::hash<expecs::Entity>
{
    size_t operator()(const expecs::Entity& entity) const noexcept
    {
        return std::hash<uint32_t>{}(entity.id);
    }
};
