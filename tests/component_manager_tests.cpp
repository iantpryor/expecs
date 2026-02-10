#include "mock_objects.h"

#include <gtest/gtest.h>

namespace expecs
{
    class expecs_ComponentManagerTest : public ::testing::Test
    {
    protected:
        void SetUp() override 
        {
            _componentManager = std::make_unique<ComponentManager>();
            _componentManager->registerComponentType<Position>();
            _componentManager->registerComponentType<Velocity>();
            _componentManager->registerComponentType<Health>();
        }

        std::unique_ptr<ComponentManager> _componentManager;
    };

    TEST_F(expecs_ComponentManagerTest, registerComponentType_ReturnsValidSignature) 
    {
        ComponentManager cm;

        Signature posSignature = cm.registerComponentType<Position>();
        Signature velSignature = cm.registerComponentType<Velocity>();

        EXPECT_NE(posSignature, 0u);
        EXPECT_NE(velSignature, 0u);
        EXPECT_NE(posSignature, velSignature);
    }

    TEST_F(expecs_ComponentManagerTest, getSignature_ReturnsCorrectSignature)
    {
        Signature posSignature = _componentManager->getSignature<Position>();
        Signature velSignature = _componentManager->getSignature<Velocity>();

        EXPECT_NE(posSignature, 0u);
        EXPECT_NE(velSignature, 0u);
        EXPECT_NE(posSignature, velSignature);

        // Should be powers of 2 (single bit set)
        EXPECT_EQ(posSignature & (posSignature - 1), 0u);
        EXPECT_EQ(velSignature & (velSignature - 1), 0u);
    }

    TEST_F(expecs_ComponentManagerTest, addComponent_StoresComponent) 
    {
        Entity entity = 42;
        Position pos(1.0f, 2.0f, 3.0f);

        auto& addedPos = _componentManager->addComponent(entity, pos);

        EXPECT_EQ(addedPos, pos);
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, getComponent_ReturnsCorrectComponent)
    {
        Entity entity = 42;
        Position pos(10.0f, 20.0f, 30.0f);

        _componentManager->addComponent(entity, pos);
        Position& retrievedPos = _componentManager->getComponent<Position>(entity);

        EXPECT_EQ(retrievedPos, pos);

        // Modify and ensure it persists
        retrievedPos.x = 99.0f;
        EXPECT_EQ(_componentManager->getComponent<Position>(entity).x, 99.0f);
    }

    TEST_F(expecs_ComponentManagerTest, hasComponent_ReturnsCorrectStatus)
    {
        Entity entity = 42;

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));

        _componentManager->addComponent(entity, Position());
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
        EXPECT_FALSE(_componentManager->hasComponent<Velocity>(entity));

        _componentManager->addComponent(entity, Velocity());
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
        EXPECT_TRUE(_componentManager->hasComponent<Velocity>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, removeComponent_RemovesComponent)
    {
        Entity entity = 42;
        _componentManager->addComponent(entity, Position());

        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));

        _componentManager->removeComponent<Position>(entity);

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, getEntitiesWithComponent_ReturnsCorrectEntities)
    {
        Entity entity1 = 1;
        Entity entity2 = 2;
        Entity entity3 = 3;

        _componentManager->addComponent(entity1, Position());
        _componentManager->addComponent(entity2, Position());
        _componentManager->addComponent(entity3, Velocity()); // Different component

        auto entities = _componentManager->getEntitiesWithComponent<Position>();

        EXPECT_EQ(entities.size(), 2);
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), entity1) != entities.end());
        EXPECT_TRUE(std::find(entities.begin(), entities.end(), entity2) != entities.end());
    }

    TEST_F(expecs_ComponentManagerTest, entityDestroyed_RemovesAllComponents)
    {
        Entity entity = 42;

        _componentManager->addComponent(entity, Position());
        _componentManager->addComponent(entity, Velocity());
        _componentManager->addComponent(entity, Health());

        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
        EXPECT_TRUE(_componentManager->hasComponent<Velocity>(entity));
        EXPECT_TRUE(_componentManager->hasComponent<Health>(entity));

        _componentManager->entityDestroyed(entity);

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));
        EXPECT_FALSE(_componentManager->hasComponent<Velocity>(entity));
        EXPECT_FALSE(_componentManager->hasComponent<Health>(entity));
    }
}