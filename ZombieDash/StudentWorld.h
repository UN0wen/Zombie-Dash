#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <string>
#include <list>

using namespace std;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;

class StudentWorld : public GameWorld
{
public:
	// Needed
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();

	// Add an actor to the world.
	void addActor(Actor* a);

	// Record that one more citizen on the current level is gone (exited,
	// died, or turned into a zombie).
	void recordCitizenGone();

	// Indicate that the player has finished the level if all citizens
	// are gone.
	void recordLevelFinishedIfAllCitizensGone();

	// For each actor overlapping a, activate a if appropriate.
	void activateOnAppropriateActors(Actor* a);

	// Can a vaccine be created at x,y?
	bool isVaccineCreationBlockedAt(double x, double y) const;

	// Is an agent blocked from moving to the indicated location?
	bool isAgentMovementBlockedAt(double x, double y, Actor* curActor) const;

	// Is creation of a flame blocked at the indicated location?
	bool isFlameBlockedAt(double x, double y) const;

	// Is there something at the indicated location that might cause a
	// zombie to vomit (i.e., a human)?
	bool isZombieVomitTriggerAt(double x, double y) const;

	// Return true if there is a living human, otherwise false.  If true,
	// otherX, otherY, and distance will be set to the location and distance
	// of the human nearest to (x,y).
	bool locateNearestVomitTrigger(double x, double y, double& otherX, double& otherY, double& distance);

	// Return true if there is a living zombie or Penelope, otherwise false.
	// If true, otherX, otherY, and distance will be set to the location and
	// distance of the one nearest to (x,y), and isThreat will be set to true
	// if it's a zombie, false if a Penelope.
	bool locateNearestCitizenTrigger(double x, double y, double& otherX, double& otherY, double& distance, bool& isThreat) const;

	// Return true if there is a living zombie, false otherwise.  If true,
	// otherX, otherY and distance will be set to the location and distance
	// of the one nearest to (x,y).
	bool locateNearestCitizenThreat(double x, double y, double& otherX, double& otherY, double& distance) const;
    ~StudentWorld() { cleanUp(); }

private:
	string getStatString(); // returns the stat string
	void addActorToFront(Actor* a);
	Actor* findActorAt(double x, double y) const; // returns actor covering coordinate x, y
	int loadLevel(int curLevel);
	void removeDeadActors();
	list<Actor *> m_actors; // The first element is always penelope
	int m_numCitizens = 0; // number of citizens alive
	bool m_levelFinished; // Is the current level finished?
	int m_level = 1;
};

#endif // STUDENTWORLD_H_
