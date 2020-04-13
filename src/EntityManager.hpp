#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

#include "BaseEntity.hpp"
#include "Bob.hpp"
#include "CommentSection.hpp"

#include <vector>

class EntityManager
{
public:
	EntityManager();
	~EntityManager();

	std::vector<BaseEntity*> entities;
	std::vector<BaseEntity*> selectedEnts;

	Bob* newBob();
	CommentSection* newCommentSection();

//	GooglePlus* newGooglePlus();

	template <typename T>
	inline T* create()
	{
		T* newEntity = new T(entities.size() + 1);

		entities.push_back(newEntity);
				
//		logger::DEBUG("created " + newEntity->type + " entity (" + std::to_string(newEntity->entityID) + ")");
		return newEntity;
	}

	void addEnt(BaseEntity *ent);
	void deleteEnt(BaseEntity *ent);

	bool selectEnt(BaseEntity *ent);
	void deselectEnt(BaseEntity *ent);
	void deselectAllEnts();
	// add selected bool to entity, check if it's selected without looping.

	std::vector<BaseEntity*> getNearbyEntities(BaseEntity* entity, float x, float y, BaseEntity* filter = nullptr);

	unsigned long long int physicalMaxEnts = std::numeric_limits<unsigned long long int>::max();
	unsigned long long int maxEnts = 100000;
	unsigned long long int maxEntsPerTeam;
};

#endif // ENTITY_MANAGER_HPP