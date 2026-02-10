#include "expecs/entity_manager.h"

#include "mock_objects.h"

#include <gtest/gtest.h>

namespace expecs
{
    class expecs_EntityManagerTest : public ::testing::Test 
    {
    protected:
        void SetUp() override 
        {
            _entityManager = std::make_unique<EntityManager>();
        }

        std::unique_ptr<EntityManager> _entityManager;
    };

    TEST_F(expecs_EntityManagerTest, createEntity)
    {
        Entity entity = _entityManager->createEntity();
        EXPECT_LT(entity, MAX_ENTITIES);
        EXPECT_EQ(_entityManager->getEntityCount(), 1u);
    }

    TEST_F(expecs_EntityManagerTest, createMultipleEntities) 
    {
        std::unordered_set<Entity> entities;

        for (int i = 0; i < 100; ++i) 
        {
            Entity entity = _entityManager->createEntity();
            EXPECT_TRUE(entities.insert(entity).second) << "Entity " << entity << " was not unique";
        }

        EXPECT_EQ(_entityManager->getEntityCount(), 100u);
    }

    TEST_F(expecs_EntityManagerTest, destroyEntity_Count) 
    {
        Entity entity1 = _entityManager->createEntity();
        Entity entity2 = _entityManager->createEntity();

        EXPECT_EQ(_entityManager->getEntityCount(), 2u);

        _entityManager->destroyEntity(entity1);
        EXPECT_EQ(_entityManager->getEntityCount(), 1u);

        _entityManager->destroyEntity(entity2);
        EXPECT_EQ(_entityManager->getEntityCount(), 0u);
    }

    TEST_F(expecs_EntityManagerTest, destroyEntity_Signature) 
    {
        Entity entity = _entityManager->createEntity();
        _entityManager->setSignature(entity, 0xFF);

        _entityManager->destroyEntity(entity);

        // Create new entity - it should reuse the ID and have signature 0
        Entity newEntity = _entityManager->createEntity();
        EXPECT_EQ(entity, newEntity);
        EXPECT_EQ(_entityManager->getSignature(newEntity), 0u);
    }

    TEST_F(expecs_EntityManagerTest, setgetSignature)
    {
        Entity entity = _entityManager->createEntity();

        _entityManager->setSignature(entity, 0b10101010);
        EXPECT_EQ(_entityManager->getSignature(entity), 0b10101010u);

        _entityManager->setSignature(entity, 0b01010101);
        EXPECT_EQ(_entityManager->getSignature(entity), 0b01010101u);
    }
}