#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Goodies;

// Base class
class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int imageID, double startX, double startY, Direction dir = 0, int depth = 0, double size = 1.0) :
		GraphObject(imageID, SPRITE_WIDTH * startX, SPRITE_HEIGHT * startY, dir, depth, size), m_world(world)
	{}

	// Is the actor alive?
	bool isAlive() const { return m_isAlive; }

	// Sets the actor to dead
	void setDead() { m_isAlive = false; }

	// Returns the actor's current world
	StudentWorld* getWorld() const { return m_world; }

	// Modify nextX and nextY with new coords corresponding to a move of distToMove pixels in direction d
	// Returns false if movement is blocked in direction d
	bool isActorAt(double x, double y) const ;

	// Get the center coordinates of the actor
	double getXCenter() const;
	double getYCenter() const;

	// Find the distance between two actor's center
	double distanceToActorCenter(const Actor* other) const;

	// Find distance between this actor's center and the center of an actor with coords x, y
	double distanceToCenter(double x, double y) const;

	// Returns true if x, y is inside of this actor's sprite
	bool spriteOverlapCheck(double x, double y);

	// Returns true if the two actors are overlapping
	bool overlapCheck(Actor* other);

	// If this is an activated object, perform its effect on a (e.g., for an
    // Exit have a use the exit).
	virtual void activateIfAppropriate(Actor* a) {};

	// If this object uses exits, use the exit.
	virtual void useExitIfAppropriate() {};

	// If this object can die by falling into a pit or burning, die.
	virtual void dieByFallOrBurnIfAppropriate() {};

	// If this object can be infected by vomit, get infected.
	virtual void beVomitedOnIfAppropriate() {};

	// If this object can pick up goodies, pick up g
	virtual void pickUpGoodieIfAppropriate(Goodies* g) {};

	// Does this object block agent movement?
	virtual bool blocksMovement() const { return false; }
		
	// Does this object block flames?
	virtual bool blocksFlame() const { return false; }

	// Does this object trigger landmines only when they're active?
	virtual bool triggersOnlyActiveLandmines() const { return false; }

	// Can this object cause a zombie to vomit?
	virtual bool triggersZombieVomit() const { return false; }

	// Is this object a threat to citizens?
	virtual bool threatensCitizens() const { return false; }

	// Does this object trigger citizens to follow it or flee it?
	virtual bool triggersCitizens() const { return false; }

	virtual void doSomething() = 0;

private:
	StudentWorld* m_world;
	bool m_isAlive = true;
};

class Agent : public Actor
{
public:
	Agent(StudentWorld* world, int imageID, double x, double y) :
		Actor(world, imageID, x, y, right, 0)
	{}

	// Agents blocks movement into them
	virtual bool blocksMovement() const { return true; }

	// Agents only triggers active landmines
	virtual bool triggersOnlyActiveLandmines() const { return true; }

	// Agents need to keep track of their time alive because some of them (citizens and zombies)
	// paralyzes every second tick
	int getTimeAlive() { return m_timeAlive; }
	void setTimeAlive(int time) { m_timeAlive = time; }

	// Attempts to move agent in the direction d for distance distance
	// Returns true if movement is successful
	bool move(Direction d, double distance);

	// Modifies d and alt_d to be directions where moving in said direction would leave
	// the agent closer to x, y
	void findDirectionTo(double x, double y, Direction& d, Direction& alt_d);

	// Modify nextX and nextY with new coords corresponding to a move of distToMove pixels in direction d
	// Returns false if movement is blocked in direction d
	bool calculateNextMove(double& nextX, double& nextY, double distToMove, Direction d);
private:
	int m_timeAlive = 0;
};

// Humans base class for the player character and citizens
class Human : public Agent
{
public:
	Human(StudentWorld* world, int imageID, double x, double y) :
		Agent(world, imageID, x, y)
	{}

	// Humans can be infected by vomit
	virtual bool triggersZombieVomit() const { return true; }
	virtual void beVomitedOnIfAppropriate();

	// Clears current infection count and set status to not infected
	void clearInfection();

	void incrementInfectionCount() { m_infectionCount++; }
	bool isInfected() const { return m_isInfected; }
	int infectionCount() const { return m_infectionCount; }
private:
	bool m_isInfected = false;
	int m_infectionCount = 0;
};

// Player character
class Penelope : public Human
{
public:
	// Constructor
	Penelope(StudentWorld* world, int x, int y) :
		Human(world, IID_PLAYER, x, y)
	{}
	virtual void doSomething();
	virtual void useExitIfAppropriate();
	virtual void dieByFallOrBurnIfAppropriate();
	virtual void pickUpGoodieIfAppropriate(Goodies* g);
	
	// Penelope triggers citizens to follow her
	virtual bool triggersCitizens() const { return true; }

	// Increase the number of vaccines the object has.
	void increaseVaccines() { m_vaccine += 1; }

	// Increase the number of flame charges the object has.
	void increaseFlameCharges() { m_flameCharge += 5; }

	// Increase the number of landmines the object has.
	void increaseLandmines() { m_landmine += 2; }

	// How many vaccines does the object have?
	int getNumVaccines() const { return m_vaccine; }

	// How many flame charges does the object have?
	int getNumFlameCharges() const { return m_flameCharge; }

	// How many landmines does the object have?
	int getNumLandmines() const { return m_landmine; }
private:
	// Penelope specfic kill command to play sound when player die
	void kill();

	// Attempts to move in direction d
	void move(Direction d);

	// Creates 3 flames 3 spaces ahead of Penelope if not blocked
	void createFlame();

	// Sets a mine at Penelope's current location
	void setMine();

	// Use a vaccine to clear infection
	void useVaccine();
	int m_vaccine = 0;
	int m_landmine = 0;
	int m_flameCharge = 0;
};

// Citizens
class Citizen : public Human
{
public:
	Citizen(StudentWorld* world, double x, double y) :
		Human(world, IID_CITIZEN, x, y)
	{}
	virtual void doSomething();
	virtual void useExitIfAppropriate();
	virtual void dieByFallOrBurnIfAppropriate();
private:
	// Kill this citizen and turn them into zombie
	void turnIntoZombie();

	// Attempts to move towards x,y
	// Returns true if move is successful
	bool moveTowards(double x, double y);

	// Attempts to move away from x,y
	void moveAwayFrom(double x, double y);

	// Returns the closest zombie distance if a move was to be made in
	// direction d
	double checksZombieDistance(Direction d);

	// Kill function specific to citizens to play sound
	void kill();

};

// Zombies base class
class Zombie : public Agent
{
public:
	Zombie(StudentWorld* world, double x, double y) :
		Agent(world, IID_ZOMBIE, x, y)
	{}

	// Kill function specific to zombies to play sound
	void kill();

	// Zombies triggers and threatens citizens
	virtual bool triggersCitizens() const { return true; }
	virtual bool threatensCitizens() const { return true; }

	// All zombies have a set movement plan
	int getMvntPlanDist() { return m_mvntPlanDist; }
	void setMvntPlanDist(int plan) { m_mvntPlanDist = plan; }

	// Attempts to vomit in the currently facing direction
	// returns true if vomit is successful
	bool vomit();

private:
	int m_mvntPlanDist = 0;
};

class DumbZombie : public Zombie
{
public:
	DumbZombie(StudentWorld* world, double x, double y) :
		Zombie(world, x, y)
	{}
	virtual void doSomething();
	virtual void dieByFallOrBurnIfAppropriate();
private:
	// Attempts to fling vaccine in direction d
	void flingVaccine(Direction d);
};

class SmartZombie : public Zombie
{
public:
	SmartZombie(StudentWorld* world, double x, double y) :
		Zombie(world, x, y)
	{}
	virtual void doSomething();
	virtual void dieByFallOrBurnIfAppropriate();

};


// Objects base class for anything other than agents
class Object : public Actor
{
public:
	Object(StudentWorld* world, int imageID, double x, double y, int depth, int dir) :
		Actor(world, imageID, x, y, dir, depth)
	{}
};

// Goodies base class
class Goodies : public Object
{
public:
	Goodies(StudentWorld* world, int imageID, double x, double y) :
		Object(world, imageID, x, y, 1, right)
	{}
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();
	virtual void doSomething();
	// Have p pick up this goodie.
	virtual void pickUp(Penelope* p) = 0;
};

class VaccineGoodie : public Goodies
{
public:
	VaccineGoodie(StudentWorld* world, double x, double y) :
		Goodies(world, IID_VACCINE_GOODIE, x, y)
	{}
	virtual void pickUp(Penelope* p);
};

class GasCanGoodie : public Goodies
{
public:
	GasCanGoodie(StudentWorld* world, double x, double y) :
		Goodies(world, IID_GAS_CAN_GOODIE, x, y)
	{}
	virtual void pickUp(Penelope* p);
};

class LandmineGoodie : public Goodies
{
public:
	LandmineGoodie(StudentWorld* world, double x, double y) :
		Goodies(world, IID_LANDMINE_GOODIE, x, y)
	{}
	virtual void pickUp(Penelope* p);
};


// Projectiles base class (for flames and vomits) 
// mainly for the 2 ticks after creation destruction common property

class Projectiles : public Object
{
public:
	Projectiles(StudentWorld* world, int imageID, double x, double y, int dir) :
		Object(world, imageID, x, y, 0, dir)
	{}
	virtual void doSomething();
private:
	int m_timeAlive = 0; // Counts up to 2 ticks
};

// Projectile classes go here
class Flame : public Projectiles
{
public:
	Flame(StudentWorld* world, double x, double y, int dir) :
		Projectiles(world, IID_FLAME, x, y, dir)
	{}
	virtual void activateIfAppropriate(Actor* a);
};

class Vomit : public Projectiles
{
public:
	Vomit(StudentWorld* world, double x, double y, int dir) :
		Projectiles(world, IID_VOMIT, x, y, dir)
	{}
	virtual void activateIfAppropriate(Actor* a);
};

class Landmine : public Object
{
public:
	Landmine(StudentWorld* world, double x, double y) :
		Object(world, IID_LANDMINE, x, y, 1, right)
	{}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();
private:
	// Make the landmine explode, introducing a pit in its current location and flames in 8 squares surrounding 
	// the pit
	void explode();
	int m_safety = 30;
	bool m_active = false;
};

// Other classes: walls, pits, exits

class Pit : public Object
{
public:
	Pit(StudentWorld* world, double x, double y) :
		Object(world, IID_PIT, x, y, 0, right)
	{}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
};

class Exit : public Object
{
public:
	Exit(StudentWorld* world, double x, double y) :
		Object(world, IID_EXIT, x, y, 1, right)
	{}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual bool blocksFlame() const { return true; }
};

class Wall : public Actor
{
public:
	Wall(StudentWorld* world, double x, double y) :
		Actor(world, IID_WALL, x, y)
	{}
	virtual void doSomething() {};

	// Wall blocks movement and flame
	virtual bool blocksMovement() const {return true;}
	virtual bool blocksFlame() const { return true; }
};


#endif // ACTOR_H_
