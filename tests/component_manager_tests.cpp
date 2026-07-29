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

    TEST_F(expecs_ComponentManagerTest, registerComponentPool_AssignsUniqueSignature)
    {
        ComponentManager componentManager;

        auto positionPool = std::make_unique<ComponentPool<Position>>();
        auto velocityPool = std::make_unique<ComponentPool<Velocity>>();

        Signature positionSignature = componentManager.registerComponentPool(std::type_index(typeid(Position)), std::move(positionPool));
        Signature velocitySignature = componentManager.registerComponentPool(std::type_index(typeid(Velocity)), std::move(velocityPool));

        EXPECT_TRUE(positionSignature.any());
        EXPECT_TRUE(velocitySignature.any());
        EXPECT_NE(positionSignature, velocitySignature);

        // Each signature should be a single bit
        EXPECT_EQ(positionSignature.count(), 1u);
        EXPECT_EQ(velocitySignature.count(), 1u);

        // Pool should be functional
        Entity entity(42);
        Position position(1.0f, 2.0f, 3.0f);
        componentManager.addComponent(entity, position);

        EXPECT_TRUE(componentManager.hasComponent<Position>(entity));
        EXPECT_EQ(componentManager.getComponent<Position>(entity), position);
    }

    TEST_F(expecs_ComponentManagerTest, registerComponentType_ReturnsValidSignature)
    {
        ComponentManager componentManager;

        Signature posSignature = componentManager.registerComponentType<Position>();
        Signature velSignature = componentManager.registerComponentType<Velocity>();

        EXPECT_TRUE(posSignature.any());
        EXPECT_TRUE(velSignature.any());
        EXPECT_NE(posSignature, velSignature);
    }

    TEST_F(expecs_ComponentManagerTest, getSignature_ReturnsCorrectSignature)
    {
        Signature posSignature = _componentManager->getSignature<Position>();
        Signature velSignature = _componentManager->getSignature<Velocity>();

        EXPECT_TRUE(posSignature.any());
        EXPECT_TRUE(velSignature.any());
        EXPECT_NE(posSignature, velSignature);

        // Should be single bit set
        EXPECT_EQ(posSignature.count(), 1u);
        EXPECT_EQ(velSignature.count(), 1u);
    }

    TEST_F(expecs_ComponentManagerTest, getSignature_TypeIndex_ReturnsCorrectSignature)
    {
        Signature templatedSignature = _componentManager->getSignature<Position>();
        Signature typeIndexSignature = _componentManager->getSignature(std::type_index(typeid(Position)));

        EXPECT_EQ(templatedSignature, typeIndexSignature);
    }

    TEST_F(expecs_ComponentManagerTest, addComponent_StoresComponent)
    {
        Entity entity(42);
        Position pos(1.0f, 2.0f, 3.0f);

        auto& addedPos = _componentManager->addComponent(entity, pos);

        EXPECT_EQ(addedPos, pos);
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, addComponent_TypeErased_StoresComponent)
    {
        Entity entity(42);
        Position position(1.0f, 2.0f, 3.0f);

        void* ptr = _componentManager->addComponent(entity, std::type_index(typeid(Position)), &position);

        ASSERT_NE(ptr, nullptr);
        auto* storedPosition = static_cast<Position*>(ptr);
        EXPECT_EQ(*storedPosition, position);
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, getComponent_ReturnsCorrectComponent)
    {
        Entity entity(42);
        Position pos(10.0f, 20.0f, 30.0f);

        _componentManager->addComponent(entity, pos);
        Position& retrievedPos = _componentManager->getComponent<Position>(entity);

        EXPECT_EQ(retrievedPos, pos);

        // Modify and ensure it persists
        retrievedPos.x = 99.0f;
        EXPECT_EQ(_componentManager->getComponent<Position>(entity).x, 99.0f);
    }

    TEST_F(expecs_ComponentManagerTest, getComponent_TypeErased_ReturnsCorrectComponent)
    {
        Entity entity(42);
        Position position(10.0f, 20.0f, 30.0f);

        _componentManager->addComponent(entity, position);

        void* ptr = _componentManager->getComponent(entity, std::type_index(typeid(Position)));

        ASSERT_NE(ptr, nullptr);
        auto* retrievedPosition = static_cast<Position*>(ptr);
        EXPECT_EQ(*retrievedPosition, position);

        retrievedPosition->x = 99.0f;
        EXPECT_EQ(_componentManager->getComponent<Position>(entity).x, 99.0f);
    }

#ifndef NDEBUG
    TEST_F(expecs_ComponentManagerTest, getSignature_Asserts_WhenTypeNotRegistered)
    {
        struct UnregisteredComponent
        {
        };

        EXPECT_DEATH(_componentManager->getSignature<UnregisteredComponent>(), "Component type not registered");
    }

    TEST_F(expecs_ComponentManagerTest, getSignature_Asserts_WhenTypeRegisteredOnAnotherManager)
    {
        struct ForeignComponent
        {
        };

        ComponentManager other;
        other.registerComponentType<ForeignComponent>();

        EXPECT_DEATH(_componentManager->getSignature<ForeignComponent>(), "Component type not registered");
    }
#endif

    TEST_F(expecs_ComponentManagerTest, hasComponent_ReturnsFalse_ForStaleHandle)
    {
        Entity entity(42);
        Entity recycled(42, 1);

        _componentManager->addComponent(entity, Position(1, 2, 3));
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));

        EXPECT_FALSE(_componentManager->hasComponent<Position>(recycled));

        _componentManager->entityDestroyed(entity);
        _componentManager->addComponent(recycled, Position(9, 9, 9));

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));
        EXPECT_FALSE(_componentManager->hasComponent(entity, std::type_index(typeid(Position))));
        EXPECT_TRUE(_componentManager->hasComponent<Position>(recycled));
        EXPECT_EQ(_componentManager->getComponent<Position>(recycled), Position(9, 9, 9));
    }

    TEST_F(expecs_ComponentManagerTest, hasComponent_ReturnsFalse_WhenTypeNotRegistered)
    {
        Entity entity(42);
        struct UnregisteredComponent
        {
        };

        EXPECT_FALSE(_componentManager->hasComponent<UnregisteredComponent>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, hasComponent_ReturnsCorrectStatus)
    {
        Entity entity(42);

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));

        _componentManager->addComponent(entity, Position());
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
        EXPECT_FALSE(_componentManager->hasComponent<Velocity>(entity));

        _componentManager->addComponent(entity, Velocity());
        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));
        EXPECT_TRUE(_componentManager->hasComponent<Velocity>(entity));
    }

    TEST_F(expecs_ComponentManagerTest, hasComponent_TypeIndex_ReturnsCorrectStatus)
    {
        Entity entity(42);

        EXPECT_FALSE(_componentManager->hasComponent(entity, std::type_index(typeid(Position))));

        _componentManager->addComponent(entity, Position());
        EXPECT_TRUE(_componentManager->hasComponent(entity, std::type_index(typeid(Position))));
        EXPECT_FALSE(_componentManager->hasComponent(entity, std::type_index(typeid(Velocity))));
    }

    TEST_F(expecs_ComponentManagerTest, removeComponent_RemovesComponent)
    {
        Entity entity(42);
        _componentManager->addComponent(entity, Position());

        EXPECT_TRUE(_componentManager->hasComponent<Position>(entity));

        _componentManager->removeComponent<Position>(entity);

        EXPECT_FALSE(_componentManager->hasComponent<Position>(entity));
    }



    TEST_F(expecs_ComponentManagerTest, entityDestroyed_RemovesAllComponents)
    {
        Entity entity(42);

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
} // namespace expecs