#include "expecs/registry.h"

#include "mock_objects.h"

#include <gtest/gtest.h>
#include <chrono>

namespace expecs
{
    class expecs_RegistryTest : public ::testing::Test
    {
    protected:
        void SetUp() override 
        {
            _registry = std::make_unique<Registry>();

            // Register components
            _registry->registerComponent<Position>();
            _registry->registerComponent<Velocity>();
            _registry->registerComponent<Health>();

            // Register movement system (Position + Velocity)
            Signature movementSignature = 0;
            movementSignature |= _registry->getComponentSignature<Position>();
            movementSignature |= _registry->getComponentSignature<Velocity>();

            _movementSystem = _registry->registerSystem<MovementSystem>(movementSignature);
        }

        std::unique_ptr<Registry> _registry;
        MovementSystem* _movementSystem;
    };

    TEST_F(expecs_RegistryTest, createEntity_ReturnsValidEntity)
    {
        Entity entity = _registry->createEntity();
        EXPECT_LT(entity, MAX_ENTITIES);
        EXPECT_EQ(_registry->getEntityCount(), 1u);
    }

    TEST_F(expecs_RegistryTest, destroyEntity_CleansUpEverything)
    {
        Entity entity = _registry->createEntity();

        // Add components
        _registry->addComponent(entity, Position(1, 2, 3));
        _registry->addComponent(entity, Velocity(4, 5, 6));

        // Verify entity is in system
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));

        // Destroy entity
        _registry->destroyEntity(entity);

        // Verify cleanup
        EXPECT_EQ(_registry->getEntityCount(), 0u);
        EXPECT_FALSE(_registry->hasComponent<Position>(entity));
        EXPECT_FALSE(_registry->hasComponent<Velocity>(entity));
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));
    }

    TEST_F(expecs_RegistryTest, addComponent_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        // Initially not in movement system
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));

        // Add Position only - still not in system
        _registry->addComponent(entity, Position());
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));

        // Add Velocity - now in system
        _registry->addComponent(entity, Velocity());
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));
    }

    TEST_F(expecs_RegistryTest, removeComponent_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        // Add both components
        _registry->addComponent(entity, Position());
        _registry->addComponent(entity, Velocity());
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));

        // Remove one component
        _registry->removeComponent<Velocity>(entity);
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));
        EXPECT_TRUE(_registry->hasComponent<Position>(entity));
        EXPECT_FALSE(_registry->hasComponent<Velocity>(entity));
    }

    TEST_F(expecs_RegistryTest, getComponent_ReturnsCorrectReference)
    {
        Entity entity = _registry->createEntity();
        Position originalPos(10, 20, 30);

        _registry->addComponent(entity, originalPos);
        Position& pos = _registry->getComponent<Position>(entity);

        EXPECT_EQ(pos, originalPos);

        // Modify through reference
        pos.x = 99;
        EXPECT_EQ(_registry->getComponent<Position>(entity).x, 99.0f);
    }

    TEST_F(expecs_RegistryTest, complexScenario_MultipleEntitiesAndSystems)
    {
        // Create entities with different component combinations
        Entity player = _registry->createEntity();
        Entity npc = _registry->createEntity();
        Entity staticObject = _registry->createEntity();

        // Player: Position + Velocity + Health (moves and has health)
        _registry->addComponent(player, Position(0, 0, 0));
        _registry->addComponent(player, Velocity(1, 0, 0));
        _registry->addComponent(player, Health(100, 100));

        // NPC: Position + Velocity (moves but no health tracking)
        _registry->addComponent(npc, Position(10, 0, 0));
        _registry->addComponent(npc, Velocity(-1, 0, 0));

        // Static object: Position + Health (has health but doesn't move)
        _registry->addComponent(staticObject, Position(5, 5, 0));
        _registry->addComponent(staticObject, Health(50, 50));

        // Check system membership
        EXPECT_TRUE(_movementSystem->getEntities().contains(player));
        EXPECT_TRUE(_movementSystem->getEntities().contains(npc));
        EXPECT_FALSE(_movementSystem->getEntities().contains(staticObject));

        // Run movement system
        _movementSystem->update();

        // Check positions changed for moving entities
        EXPECT_EQ(_registry->getComponent<Position>(player).x, 1.0f);
        EXPECT_EQ(_registry->getComponent<Position>(npc).x, 9.0f);
        EXPECT_EQ(_registry->getComponent<Position>(staticObject).x, 5.0f); // Unchanged

        // Get entities with specific components
        auto entitiesWithHealth = _registry->getEntitiesWithComponent<Health>();
        EXPECT_EQ(entitiesWithHealth.size(), 2u);
        EXPECT_TRUE(std::find(entitiesWithHealth.begin(), entitiesWithHealth.end(), player) != entitiesWithHealth.end());
        EXPECT_TRUE(std::find(entitiesWithHealth.begin(), entitiesWithHealth.end(), staticObject) != entitiesWithHealth.end());
    }
}