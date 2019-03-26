#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <list>
#include <cmath>
// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp


///////////
// Actor //
///////////

// Modify nextX and nextY with new coords corresponding to a move of distToMove pixels in direction d
// Returns false if movement is blocked in direction d
bool Agent::calculateNextMove(double& nextX, double& nextY, double distToMove, Direction d)
{
	switch (d)
	{
	case up:
		nextY += distToMove;
		break;
	case down:
		nextY -= distToMove;
		break;
	case left:
		nextX -= distToMove;
		break;
	case right:
		nextX += distToMove;
		break;
	}

	if (getWorld()->isAgentMovementBlockedAt(nextX, nextY, this))
	{
		return false;
	}

	return true;
}
bool Actor::isActorAt(double x, double y) const
{
	return (getX() == x && getY() == y);
}

double Actor::getXCenter() const 
{ 
	return getX() + SPRITE_WIDTH / 2;
}

double Actor::getYCenter() const
{ 
	return getY() + SPRITE_HEIGHT / 2; 
}

double Actor::distanceToActorCenter(const Actor* other) const
{
	double xDiff = getXCenter() - other->getXCenter();
	double yDiff = getYCenter() - other->getYCenter();
	return sqrt(pow(xDiff, 2.0) + pow(yDiff, 2.0));
}

double Actor::distanceToCenter(double x, double y) const
{
	double xDiff = getXCenter() - (x + SPRITE_WIDTH / 2);
	double yDiff = getYCenter() - (y + +SPRITE_HEIGHT / 2);
	return sqrt(pow(xDiff, 2.0) + pow(yDiff, 2.0));
}

bool Actor::spriteOverlapCheck(double x, double y) 
{ 
	return abs(getX() - x) < SPRITE_WIDTH && abs(getY() - y) < SPRITE_HEIGHT; 
}

bool Actor::overlapCheck(Actor* other)
{ 
	return distanceToActorCenter(other) < 10;
}

///////////
// Agent //
///////////

// Attempts to move distance pixels in direction d
bool Agent::move(Direction d, double distance)
{
	double nextX, nextY;
	nextX = getX();
	nextY = getY();
	bool moveSuccess = calculateNextMove(nextX, nextY, distance, d);

	if (!moveSuccess) return false;

	setDirection(d);
	moveTo(nextX, nextY);
	return true;
}

////////////
// Humans //
////////////

void Human::beVomitedOnIfAppropriate()
{
	m_isInfected = true;
}

void Human::clearInfection()
{
	m_isInfected = false;
	m_infectionCount = 0;
}

//////////////
// Penelope //
//////////////

void Penelope::doSomething()
{
	if (!isAlive()) return;

	if (isInfected()) incrementInfectionCount();

	if (infectionCount() > 500)
	{
		kill();
		return;
	}

	int ch;
	if (getWorld()->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_LEFT:
			move(left);
			break;
		case KEY_PRESS_RIGHT:
			move(right);
			break;
		case KEY_PRESS_UP:
			move(up);
			break;
		case KEY_PRESS_DOWN:
			move(down);
			break;
		case KEY_PRESS_SPACE:
			createFlame();
			break;
		case KEY_PRESS_TAB:
			setMine();
			break;
		case KEY_PRESS_ENTER:
			useVaccine();
			break;
		}
	}
}



void Penelope::createFlame()
{
	if (m_flameCharge <= 0) return;
	int dir = getDirection();
	double flameX, flameY;
	m_flameCharge--;
	double x = getX() / SPRITE_WIDTH;
	double y = getY() / SPRITE_HEIGHT;
	for (int i = 1; i < 4; i++)
	{
		if (dir == up)
		{
			flameX = x;
			flameY = y + i;
		}
		else if (dir == down)
		{
			flameX = x;
			flameY = y - i;
		}
		else if (dir == right)
		{
			flameX = x + i;
			flameY = y;
		}
		else if (dir == left)
		{
			flameX = x - i;
			flameY = y;
		}

		if (getWorld()->isFlameBlockedAt(flameX * SPRITE_WIDTH, flameY * SPRITE_HEIGHT)) return;
		Actor* flame = new Flame(getWorld(), flameX, flameY, dir);
		getWorld()->addActor(flame);
	}

	getWorld()->playSound(SOUND_PLAYER_FIRE);
}

void Penelope::useVaccine()
{
	if (m_vaccine <= 0) return;
	clearInfection();
	m_vaccine--;
}

void Penelope::setMine()
{
	if (m_landmine <= 0) return;
	m_landmine--;
	Actor* landmine = new Landmine(getWorld(), getX()/SPRITE_WIDTH, getY()/SPRITE_HEIGHT);
	getWorld()->addActor(landmine);
}

void Penelope::move(Direction d)
{
	setDirection(d);
	double nextX = getX();
	double nextY = getY();
	bool moveSuccess = calculateNextMove(nextX, nextY, 4, d);

	if (!moveSuccess) return;
	moveTo(nextX, nextY);
}

void Penelope::pickUpGoodieIfAppropriate(Goodies* g)
{
	g->pickUp(this);
	g->setDead();
	getWorld()->increaseScore(50);
}

void Penelope::kill()
{
	setDead();
	getWorld()->decLives();
	getWorld()->playSound(SOUND_PLAYER_DIE);
}

void Penelope::dieByFallOrBurnIfAppropriate()
{
	kill();
}

void Penelope::useExitIfAppropriate()
{
	getWorld()->recordLevelFinishedIfAllCitizensGone();
}



//////////////
// Citizens //
//////////////

void Citizen::doSomething() 
{
	if (!isAlive()) return;

	int timeAlive = getTimeAlive();
	timeAlive++;
	setTimeAlive(timeAlive);

	if (isInfected()) incrementInfectionCount();

	// Citizen was just infected
	if (infectionCount() == 1)
	{
		getWorld()->playSound(SOUND_CITIZEN_INFECTED);
	}

	if (infectionCount() > 500)
	{
		turnIntoZombie();
		return;
	}

	if (timeAlive % 2 == 0) return;

	// Step 4/5: calculate distance to Penelope/Zombie
	double closestTriggerX, closestTriggerY, distClosestTrigger ;
	bool isThreat;
	bool triggerExists = getWorld()->locateNearestCitizenTrigger(getX(), getY(),
		closestTriggerX, closestTriggerY, distClosestTrigger, isThreat);

	// If there is something in the world that triggers citizens
	if (triggerExists)
	{
		double closestZombieX, closestZombieY, distClosestZombie;
		// Nearest is a zombie
		if (isThreat)
		{
			closestZombieX = closestTriggerX;
			closestZombieY = closestTriggerY;
			distClosestZombie = distClosestTrigger;
		}
		// Nearest is Penelope, closest triggerX,Y is penelope's location
		else
		{
			// Citizen only moves when in 80 pixel range
			if (distClosestTrigger < 80)
			{
				// moveTowards return true if the move is successful
				if (moveTowards(closestTriggerX, closestTriggerY)) return;
			}

			// Movement failed or not close enough
			bool zombieExists = getWorld()->locateNearestCitizenThreat(getX(), getY(),
				closestZombieX, closestZombieY, distClosestZombie);

			// No zombies and not close enough to Penelope to move, do nothing
			if (!zombieExists) return;
		}

		// Check if the nearest zombie is near enough for the citizen to run
		if (distClosestZombie < 80)
		{
			moveAwayFrom(closestZombieX, closestZombieY);
			return;
		}
	}
}


// Returns distance to the closest zombie after going 2 pixels in direction d
// Returns 0 if the citizen cant move in this direction
double Citizen::checksZombieDistance(Direction d)
{
	double nextX, nextY, distClosestZombie;
	nextX = getX();
	nextY = getY();

	bool moveSuccess = calculateNextMove(nextX, nextY, 2, d);

	if (!moveSuccess) return 0;

	double zombieX, zombieY;
	getWorld()->locateNearestCitizenThreat(nextX, nextY,
		zombieX, zombieY, distClosestZombie);

	return distClosestZombie;

}

// Moves agent towards actor at location x,y
// Returns true if move is successful
bool Citizen::moveTowards(double x, double y)
{
	Direction d; // Direction (and alternate direction if applicable) to get to to get to Penelope
	Direction alt_d = -1; // Default as -1 to test if a value is assigned to alt_d

	findDirectionTo(x, y, d, alt_d);
	// If first move towards direction d is successful
	if (move(d, 2)) return true;

	// Attemps alternate direction if there is one
	if (alt_d >= 0) return move(alt_d, 2);

	return false;
}

void Agent::findDirectionTo(double x, double y, Direction& d, Direction& alt_d)
{
	double yDiff, xDiff;
	xDiff = getX() - x;
	yDiff = getY() - y;

	if (xDiff == 0)
	{
		d = (yDiff > 0) ? down : up;
	}
	else if (yDiff == 0)
	{
		d = (xDiff > 0) ? left : right;
	}
	else
	{
		Direction moveDirArray[2];
		int mainDir = randInt(0, 1);

		moveDirArray[0] = (yDiff > 0) ? down : up;
		moveDirArray[1] = (xDiff > 0) ? left : right;

		d = moveDirArray[mainDir];
		alt_d = moveDirArray[abs(1 - mainDir)];
	}
}
// Move away from zombie located at x, y
void Citizen::moveAwayFrom(double x, double y)
{
	double distanceToZombie = distanceToCenter(x, y);

	Direction dirs[4] = { up, down, left, right };
	Direction movedir = -1;

	for (int i = 0; i < 4; i++)
	{
		double newDistance = checksZombieDistance(dirs[i]);
		if (newDistance > distanceToZombie)
		{
			distanceToZombie = newDistance;
			movedir = dirs[i];
		}
	}

	// Distance to zombie has not changed -> do nothing
	if (distanceToZombie == distanceToCenter(x, y)) return;

	move(movedir, 2);
}

// Turn citizen into zombie
void Citizen::turnIntoZombie()
{
	kill();
	getWorld()->playSound(SOUND_ZOMBIE_BORN);
	int zombieChance = randInt(1, 10);
	// 30% chance for smart zombie, 70% chance for dumb zombie
	if (zombieChance <= 7)
	{
		getWorld()->addActor(new DumbZombie(getWorld(), getX() / SPRITE_WIDTH, getY() / SPRITE_HEIGHT));
	}
	else 
	{
		getWorld()->addActor(new SmartZombie(getWorld(), getX() / SPRITE_WIDTH, getY() / SPRITE_HEIGHT));
	}
}
void Citizen::kill()
{
	setDead();
	getWorld()->recordCitizenGone();
	getWorld()->increaseScore(-1000);
}

void Citizen::dieByFallOrBurnIfAppropriate()
{
	kill();
	getWorld()->playSound(SOUND_CITIZEN_DIE);
}
void Citizen::useExitIfAppropriate() 
{
	setDead();
	getWorld()->increaseScore(500); // Gain 500 pts for rescuing citizen
	getWorld()->recordCitizenGone(); 
	getWorld()->playSound(SOUND_CITIZEN_SAVED); 
}
/////////////
// Zombies //
/////////////

void Zombie::kill()
{
	setDead();
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
}

// Search for the nearest human to vomit on in currently facing direction
// Return true if zombie vomits, false if not
bool Zombie::vomit()
{
	Direction d = getDirection();
	double vomitX, vomitY;
	vomitX = getX();
	vomitY = getY();
	switch (d)
	{
	case up:
		vomitY += SPRITE_HEIGHT;
		break;
	case down:
		vomitY -= SPRITE_HEIGHT;
		break;
	case left:
		vomitX -= SPRITE_WIDTH;
		break;
	case right:
		vomitX += SPRITE_WIDTH;
		break;
	}

	double humanX, humanY, distToHuman;
	bool humanExists = getWorld()->locateNearestVomitTrigger(vomitX, vomitY, humanX, humanY, distToHuman);

	// There is a human to vomit on
	if (humanExists)
	{
		// Human is within 10 pixels of the vomit
		if (distToHuman < 10)
		{
			// 1 in 3 chance to vomit
			int vomitChance = randInt(1, 3);
			if (vomitChance == 1)
			{
				getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
				getWorld()->addActor(new Vomit(getWorld(), vomitX/SPRITE_WIDTH, vomitY/SPRITE_HEIGHT, d));
				return true;
			}
		}
	}

	// Zombie didnt vomit
	return false;
}

// Attempts to fling a vaccine in direction d
void DumbZombie::flingVaccine(Direction d)
{
	double vaccineX, vaccineY;
	vaccineX = getX();
	vaccineY = getY();
	calculateNextMove(vaccineX, vaccineY, SPRITE_WIDTH, d);

	if (getWorld()->isVaccineCreationBlockedAt(vaccineX, vaccineY)) return;
	else
	{
		getWorld()->addActor(new VaccineGoodie(getWorld(), vaccineX / SPRITE_WIDTH, vaccineY / SPRITE_HEIGHT));
	}
}

void DumbZombie::doSomething()
{
	if (!isAlive()) return;

	int timeAlive = getTimeAlive();
	timeAlive++;
	setTimeAlive(timeAlive);

	if (timeAlive % 2 == 0) return;

	// Attempts to vomit in currently facing direction
	if (vomit()) return;

	// Checks if zombie need a new movement plan
	int mvntPlan = getMvntPlanDist();
	if (mvntPlan == 0)
	{
		mvntPlan = randInt(3, 10);
		Direction alldirs[4] = { up, down, left, right };
		int dirChance = randInt(0, 3);
		setDirection(alldirs[dirChance]);
		setMvntPlanDist(mvntPlan);
	}

	// If movement is successful in the currently facing direction
	if (move(getDirection(), 1))
	{
		// Decrease mvntplandist by 1
		setMvntPlanDist(--mvntPlan);
	}
	else
	{
		setMvntPlanDist(0);
	}
}

void SmartZombie::doSomething()
{
	if (!isAlive()) return;

	int timeAlive = getTimeAlive();
	timeAlive++;
	setTimeAlive(timeAlive);

	if (timeAlive % 2 == 0) return;

	// Attempts to vomit in currently facing direction
	if (vomit()) return;

	// Checks if zombie need a new movement plan
	int mvntPlan = getMvntPlanDist();
	if (mvntPlan == 0)
	{
		double humanX, humanY, distToHuman;
		Direction d = getDirection();
		Direction alt_d;
		mvntPlan = randInt(3, 10);
		bool humanExists = getWorld()->locateNearestVomitTrigger(getX(), getY(), humanX, humanY, distToHuman);
		if (humanExists)
		{
			if (distToHuman < 80)
			{
				findDirectionTo(humanX, humanY, d, alt_d);
				setDirection(d);
			}
			else
			{
				Direction alldirs[4] = { up, down, left, right };
				int dirChance = randInt(0, 3);
				d = alldirs[dirChance];
			}
		}
		
		setDirection(d);
		setMvntPlanDist(mvntPlan);
	}

	// If movement is successful in the currently facing direction
	if (move(getDirection(), 1))
	{
		// Decrease mvntplandist by 1
		setMvntPlanDist(--mvntPlan);
	}
	else
	{
		setMvntPlanDist(0);
	}
}

void DumbZombie::dieByFallOrBurnIfAppropriate()
{
	kill();
	getWorld()->increaseScore(1000);
	int vaccineChance = randInt(1, 10);
	if (vaccineChance == 10)
	{
		Direction alldirs[4] = { up, down, left, right };
		int dirChance = randInt(0, 3);
		Direction d = alldirs[dirChance];

		flingVaccine(d);
	}
}

void SmartZombie::dieByFallOrBurnIfAppropriate()
{
	kill();
	getWorld()->increaseScore(2000);
}

/////////////
// Goodies //
/////////////

void Goodies::activateIfAppropriate(Actor* a)
{
	a->pickUpGoodieIfAppropriate(this);
	if (!isAlive())
	{
		getWorld()->playSound(SOUND_GOT_GOODIE);
	}
}

void Goodies::dieByFallOrBurnIfAppropriate()
{
	setDead();
}

void Goodies::doSomething()
{
	if (isAlive()) getWorld()->activateOnAppropriateActors(this);
}


void VaccineGoodie::pickUp(Penelope* p)
{
	p->increaseVaccines();
}

void GasCanGoodie::pickUp(Penelope* p)
{
	p->increaseFlameCharges();
}

void LandmineGoodie::pickUp(Penelope* p)
{
	p->increaseLandmines();
}

/////////////////
// Projectiles //
/////////////////

void Projectiles::doSomething()
{
	if (isAlive())
	{

		if (m_timeAlive == 3)
		{
			setDead();
		}

		getWorld()->activateOnAppropriateActors(this);

		m_timeAlive++;
	}
}
void Flame::activateIfAppropriate(Actor* a)
{
	if (isAlive()) a->dieByFallOrBurnIfAppropriate();
}

void Vomit::activateIfAppropriate(Actor* a)
{
	if (isAlive()) a->beVomitedOnIfAppropriate();
}

///////////
// Other //
///////////

void Landmine::activateIfAppropriate(Actor* a)
{
	if (m_active && a->triggersOnlyActiveLandmines()) explode();
}

void Landmine::dieByFallOrBurnIfAppropriate()
{
	if (isAlive()) explode();
}

void Landmine::explode()
{
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
	setDead();
	double curX = getX() / SPRITE_WIDTH;
	double curY = getY() / SPRITE_HEIGHT;
	double x_values[3] = { curX - 1, curX, curX + 1 };
	double y_values[3] = { curY - 1, curY, curY + 1 };
	Actor* actor;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (x_values[i] == curX && y_values[j] == curY)
			{
				actor = new Pit(getWorld(), curX, curY);
			}
			else
			{
				actor = new Flame(getWorld(), x_values[i], y_values[j], up);
			}

			getWorld()->addActor(actor);
		}
	}
}
void Landmine::doSomething()
{
	if (!isAlive()) return;
	if (!m_active)
	{
		if (m_safety > 0) m_safety--;
		else m_active = true;
	}
	else if (m_active)
	{
		getWorld()->activateOnAppropriateActors(this);
	}
}

void Pit::activateIfAppropriate(Actor* a)
{
	a->dieByFallOrBurnIfAppropriate();
}

void Pit::doSomething()
{
	if (isAlive()) getWorld()->activateOnAppropriateActors(this);
}

void Exit::activateIfAppropriate(Actor* a)
{
	a->useExitIfAppropriate();
}

void Exit::doSomething()
{
	if (isAlive()) getWorld()->activateOnAppropriateActors(this);
}