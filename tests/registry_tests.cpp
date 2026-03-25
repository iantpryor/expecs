#include "expecs/registry.h"

#include "mock_objects.h"

#include <chrono>
#include <gtest/gtest.h>

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
        EXPECT_EQ(_registry->getEntityCount(), 1u);
    }

    TEST_F(expecs_RegistryTest, destroyEntity_CleansUpEverything)
    {
        Entity entity = _registry->createEntity();

        // Add components
        _registry->addComponent(entity, Position(1, 2, 3));
        _registry->addComponent(entity, Velocity(4, 5, 6));

        // Verify entity is in system
        EXPECT_TRUE(_movementSystem->contains(entity));

        // Destroy entity
        _registry->destroyEntity(entity);

        // Verify cleanup
        EXPECT_EQ(_registry->getEntityCount(), 0u);
        EXPECT_FALSE(_registry->hasComponent<Position>(entity));
        EXPECT_FALSE(_registry->hasComponent<Velocity>(entity));
        EXPECT_FALSE(_movementSystem->contains(entity));
    }

    TEST_F(expecs_RegistryTest, addComponent_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        // Initially not in movement system
        EXPECT_FALSE(_movementSystem->contains(entity));

        // Add Position only - still not in system
        _registry->addComponent(entity, Position());
        EXPECT_FALSE(_movementSystem->contains(entity));

        // Add Velocity - now in system
        _registry->addComponent(entity, Velocity());
        EXPECT_TRUE(_movementSystem->contains(entity));
    }

    TEST_F(expecs_RegistryTest, addComponent_TypeErased_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        Position position(1.0f, 2.0f, 3.0f);
        Velocity velocity(4.0f, 5.0f, 6.0f);

        _registry->addComponent(entity, std::type_index(typeid(Position)), &position);
        EXPECT_FALSE(_movementSystem->contains(entity));

        _registry->addComponent(entity, std::type_index(typeid(Velocity)), &velocity);
        EXPECT_TRUE(_movementSystem->contains(entity));
    }

    TEST_F(expecs_RegistryTest, removeComponent_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        // Add both components
        _registry->addComponent(entity, Position());
        _registry->addComponent(entity, Velocity());
        EXPECT_TRUE(_movementSystem->contains(entity));

        // Remove one component
        _registry->removeComponent<Velocity>(entity);
        EXPECT_FALSE(_movementSystem->contains(entity));
        EXPECT_TRUE(_registry->hasComponent<Position>(entity));
        EXPECT_FALSE(_registry->hasComponent<Velocity>(entity));
    }

    TEST_F(expecs_RegistryTest, removeComponent_TypeErased_UpdatesSystemMembership)
    {
        Entity entity = _registry->createEntity();

        _registry->addComponent(entity, Position());
        _registry->addComponent(entity, Velocity());
        EXPECT_TRUE(_movementSystem->contains(entity));

        _registry->removeComponent(entity, std::type_index(typeid(Velocity)));
        EXPECT_FALSE(_movementSystem->contains(entity));
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

    TEST_F(expecs_RegistryTest, getComponent_TypeErased_ReturnsCorrectPointer)
    {
        Entity entity = _registry->createEntity();
        Position position(10.0f, 20.0f, 30.0f);

        _registry->addComponent(entity, position);

        void* ptr = _registry->getComponent(entity, std::type_index(typeid(Position)));
        ASSERT_NE(ptr, nullptr);

        auto* retrievedPosition = static_cast<Position*>(ptr);
        EXPECT_EQ(*retrievedPosition, position);
    }

    TEST_F(expecs_RegistryTest, hasComponent_TypeErased_ReturnsCorrectStatus)
    {
        Entity entity = _registry->createEntity();

        EXPECT_FALSE(_registry->hasComponent(entity, std::type_index(typeid(Position))));

        _registry->addComponent(entity, Position());
        EXPECT_TRUE(_registry->hasComponent(entity, std::type_index(typeid(Position))));
        EXPECT_FALSE(_registry->hasComponent(entity, std::type_index(typeid(Velocity))));
    }

    TEST_F(expecs_RegistryTest, registerComponentPool_WorksWithTypeErasedApi)
    {
        Registry registry;

        auto positionPool = std::make_unique<ComponentPool<Position>>();
        Signature positionSignature = registry.registerComponentPool(std::type_index(typeid(Position)), std::move(positionPool));

        EXPECT_NE(positionSignature, 0u);

        Entity entity = registry.createEntity();
        Position position(1.0f, 2.0f, 3.0f);

        registry.addComponent(entity, std::type_index(typeid(Position)), &position);

        EXPECT_TRUE(registry.hasComponent(entity, std::type_index(typeid(Position))));
        EXPECT_EQ(registry.getEntitySignature(entity), positionSignature);

        auto* retrievedPosition = static_cast<Position*>(registry.getComponent(entity, std::type_index(typeid(Position))));
        EXPECT_EQ(*retrievedPosition, position);
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
        EXPECT_TRUE(_movementSystem->contains(player));
        EXPECT_TRUE(_movementSystem->contains(npc));
        EXPECT_FALSE(_movementSystem->contains(staticObject));

        // Run movement system
        _movementSystem->update();

        // Check positions changed for moving entities
        EXPECT_EQ(_registry->getComponent<Position>(player).x, 1.0f);
        EXPECT_EQ(_registry->getComponent<Position>(npc).x, 9.0f);
        EXPECT_EQ(_registry->getComponent<Position>(staticObject).x, 5.0f); // Unchanged

        // Get entities with specific components
        auto entitiesWithHealth = _registry->getEntitiesWithComponents<Health>();
        EXPECT_EQ(entitiesWithHealth.size(), 2u);
        EXPECT_TRUE(std::find(entitiesWithHealth.begin(), entitiesWithHealth.end(), player) != entitiesWithHealth.end());
        EXPECT_TRUE(std::find(entitiesWithHealth.begin(), entitiesWithHealth.end(), staticObject) != entitiesWithHealth.end());
    }

    TEST_F(expecs_RegistryTest, getEntitiesWithComponents_MultipleComponents)
    {
        Entity e1 = _registry->createEntity();
        Entity e2 = _registry->createEntity();
        Entity e3 = _registry->createEntity();

        _registry->addComponent(e1, Position());
        _registry->addComponent(e1, Velocity());
        _registry->addComponent(e1, Health());

        _registry->addComponent(e2, Position());
        _registry->addComponent(e2, Health());

        _registry->addComponent(e3, Position());
        _registry->addComponent(e3, Velocity());

        // Query for Position + Health
        auto entities = _registry->getEntitiesWithComponents<Position, Health>();
        EXPECT_EQ(entities.size(), 2u);
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), e1) != entities.end());
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), e2) != entities.end());

        // Query for all three
        auto allThree = _registry->getEntitiesWithComponents<Position, Velocity, Health>();
        EXPECT_EQ(allThree.size(), 1u);
        EXPECT_EQ(allThree[0], e1);
    }

    TEST_F(expecs_RegistryTest, getEntitiesWithComponents_SignatureBased)
    {
        Entity e1 = _registry->createEntity();
        Entity e2 = _registry->createEntity();
        Entity e3 = _registry->createEntity();

        _registry->addComponent(e1, Position());
        _registry->addComponent(e1, Velocity());
        _registry->addComponent(e1, Health());

        _registry->addComponent(e2, Position());
        _registry->addComponent(e2, Health());

        _registry->addComponent(e3, Position());
        _registry->addComponent(e3, Velocity());

        // Query for Position + Health
        Signature query = _registry->getComponentSignature(std::type_index(typeid(Position))) | _registry->getComponentSignature(std::type_index(typeid(Health)));
        auto entities = _registry->getEntitiesWithComponents(query);
        EXPECT_EQ(entities.size(), 2u);
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), e1) != entities.end());
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), e2) != entities.end());

        // Query for all three
        query = _registry->getComponentSignature(std::type_index(typeid(Position))) | _registry->getComponentSignature(std::type_index(typeid(Velocity))) | _registry->getComponentSignature(std::type_index(typeid(Health)));
        auto allThree = _registry->getEntitiesWithComponents(query);
        EXPECT_EQ(allThree.size(), 1u);
        EXPECT_EQ(allThree[0], e1);
    }
} // namespace expecs