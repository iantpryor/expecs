#include "expecs/registry.h"

#include "mock_objects.h"

#include <gtest/gtest.h>
#include <chrono>

namespace expecs
{
    class expecs_PerformanceTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            _registry = std::make_unique<Registry>();
            _registry->registerComponent<Position>();
            _registry->registerComponent<Velocity>();
        }

        std::unique_ptr<Registry> _registry;
    };

    TEST_F(expecs_PerformanceTest, createManyEntities)
    {
        constexpr int NUM_ENTITIES = 100000;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Should complete in reasonable time (adjust threshold as needed)
        EXPECT_LT(duration.count(), 100000) << "Creating " << NUM_ENTITIES << " entities took " << duration.count() << " microseconds";
        EXPECT_EQ(_registry->getEntityCount(), NUM_ENTITIES);
    }
}