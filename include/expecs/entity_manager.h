#pragma once

#include <cstdint>
#include <stack>
#include <array>

namespace expecs
{
	constexpr uint32_t MAX_ENTITIES = 100000;

	using Entity = uint32_t;
	using Signature = uint32_t;

	class EntityManager
	{
	public:
		EntityManager()
		{
			for (Entity entity = 0; entity < MAX_ENTITIES; entity++)
			{
				_availableEntities.push(entity);
				_signatures[entity] = 0;
			}
		}
		~EntityManager() = default;

		Entity createEntity()
		{
			Entity id = _availableEntities.top();
			_availableEntities.pop();
			_aliveCount++;
			return id;
		}

		void destroyEntity(Entity entity)
		{
			_signatures[entity] = 0;
			_availableEntities.push(entity);
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
			_signatures[entity] = signature;
		}

	private:
		std::stack<Entity> _availableEntities;
		std::array<Signature, MAX_ENTITIES> _signatures = {};
		uint32_t _aliveCount = 0;
	};
}