#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include "Level.h"
#include <sstream> 
#include <string>
#include <iomanip>
#include <list>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
	: GameWorld(assetPath)
{
}

bool StudentWorld::isAgentMovementBlockedAt(double x, double y, Actor* curActor) const
{
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr) == curActor) continue;
		if ((*actorPtr)->blocksMovement())
		{
			if ((*actorPtr)->spriteOverlapCheck(x, y)) return true;
		}
	}

	return false;
}

// Loads the specified level number from the file
int StudentWorld::loadLevel(int curLevel)
{
	Level lev(assetPath());
	ostringstream levelFileStream;
	levelFileStream.fill('0');
	levelFileStream << "level" << setw(2) << curLevel << ".txt";
	string levelFile = levelFileStream.str();
	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found)
		cerr << "Cannot find " << levelFile << " data file" << endl;
	else if (result == Level::load_fail_bad_format)
		cerr << "Your level was improperly formatted" << endl;
	else if (result == Level::load_success)
	{
		cerr << "Successfully loaded level" << endl;
		for (int i = 0; i < LEVEL_WIDTH; i++)
		{
			for (int j = 0; j < LEVEL_HEIGHT; j++)
			{
				Actor* actor = nullptr;
				Level::MazeEntry me = lev.getContentsOf(i, j);
				switch (me) 
				{
				case Level::empty:
					break;
				case Level::smart_zombie:
					actor = new SmartZombie(this, i, j);
					break;
				case Level::dumb_zombie:
					actor = new DumbZombie(this, i, j);
					break;
				case Level::player:
				{
					Penelope* playerCharacter = new Penelope(this, i, j);
					addActorToFront(playerCharacter);
					break;
				}
				case Level::exit:
					actor = new Exit(this, i, j);
					break;
				case Level::wall:
					actor = new Wall(this, i, j);
					break;
				case Level::pit:
					actor = new Pit(this, i, j);
					break;
				case Level::citizen:
				{
					actor = new Citizen(this, i, j);
					m_numCitizens++;
					break;
				}
				case Level::landmine_goodie:
					actor = new LandmineGoodie(this, i, j);
					break;
				case Level::gas_can_goodie:
					actor = new GasCanGoodie(this, i, j);
					break;
				case Level::vaccine_goodie:
					actor = new VaccineGoodie(this, i, j);
					break;
				}
				
				if (actor != nullptr) addActor(actor);
			}
		}
	}

	return result;
}

int StudentWorld::init()
{
	if (m_level == 100) return GWSTATUS_PLAYER_WON;
	// Loads level, level loading will allocate all actors for the level
	m_numCitizens = 0;
	int result = loadLevel(m_level);
	if (result == Level::load_fail_bad_format)
	{
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level::load_fail_file_not_found)
	{
		return GWSTATUS_PLAYER_WON;
	}
	else
	{
		m_levelFinished = false;
		return GWSTATUS_CONTINUE_GAME;
	}
}

// Calls actor's doSomething(), disposing actors that disappeared during a tick
int StudentWorld::move()
{
	Penelope* player = static_cast<Penelope*>(m_actors.front());

	// Call every actor's doSomething() method
	
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr)->isAlive())
		{
			(*actorPtr)->doSomething();

			if (!player->isAlive()) return GWSTATUS_PLAYER_DIED;

			if (m_levelFinished)
			{
				m_level++;
				playSound(SOUND_LEVEL_FINISHED);
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}

	removeDeadActors();

	string stat = getStatString();
	setGameStatText(stat);

	return GWSTATUS_CONTINUE_GAME;
}

// Frees all actors
void StudentWorld::cleanUp()
{
	list<Actor *>::const_iterator actorPtr = m_actors.begin();
	for (; actorPtr != m_actors.end(); actorPtr++)
	{
		delete (*actorPtr);
	}

	m_actors.clear();
}

void StudentWorld::addActor(Actor* a)
{
	m_actors.push_back(a);
}

void StudentWorld::addActorToFront(Actor* a)
{
	m_actors.push_front(a);
}

void StudentWorld::removeDeadActors()
{
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if (!(*actorPtr)->isAlive())
		{
			delete (*actorPtr);
			actorPtr = m_actors.erase(actorPtr);
		}

		if (actorPtr == m_actors.end()) return;
	}
}

string StudentWorld::getStatString()
{
	ostringstream statString;
	Penelope* player = static_cast<Penelope*>(m_actors.front());

	statString.fill('0');
	int score = getScore();
	if (score < 0)
	{
		statString << "Score: " << "-" << setw(5) << abs(score) << "  ";
	}
	else
	{
		statString << "Score: " << setw(6) << getScore() << "  ";
	}
	statString << "Level: " << m_level << "  ";
	statString << "Lives: " << getLives() << "  ";
	statString << "Vacc: " << player->getNumVaccines() << "  ";
	statString << "Flames: " << player->getNumFlameCharges() << "  ";
	statString << "Mines: " << player->getNumLandmines() << "  ";
	statString << "Infected: " << player->infectionCount() << " ";
	string str = statString.str();
	return str;
}

// Is there an actor at x,y?
Actor* StudentWorld::findActorAt(double x, double y) const
{
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr)->spriteOverlapCheck(x, y))
		{
			return (*actorPtr);
		}
	}

	return nullptr;
}

void StudentWorld::activateOnAppropriateActors(Actor* a)
{
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr) == a) continue;

		if ((*actorPtr)->overlapCheck(a))
		{
			a->activateIfAppropriate((*actorPtr));
		}
	}
}

// Record that one more citizen on the current level is gone (exited,
// died, or turned into a zombie).
void StudentWorld::recordCitizenGone()
{
	m_numCitizens--;
}

// Indicate that the player has finished the level if all citizens
// are gone.
void StudentWorld::recordLevelFinishedIfAllCitizensGone()
{
	if (m_numCitizens == 0)
	{
		m_levelFinished = true;
	}
}

// Is creation of a vaccine blocked at the indicated location?
bool StudentWorld::isVaccineCreationBlockedAt(double x, double y) const
{
	Actor* a = findActorAt(x, y);
	if (a != nullptr) return true;
	return false;
}

// Is creation of a flame blocked at the indicated location?
bool StudentWorld::isFlameBlockedAt(double x, double y) const
{
	Actor* a = findActorAt(x, y);
	if (a != nullptr) return a->blocksFlame();
	return false;
}

// Is there something at the indicated location that might cause a
// zombie to vomit (i.e., a human)?
bool StudentWorld::isZombieVomitTriggerAt(double x, double y) const
{
	Actor* a = findActorAt(x, y);

	return a->triggersZombieVomit();
}

// Return true if there is a living human, otherwise false.  If true,
// otherX, otherY, and distance will be set to the location and distance
// of the human nearest to (x,y).
bool StudentWorld::locateNearestVomitTrigger(double x, double y, double& otherX, double& otherY, double& distance)
{
	bool humanExists = false;
	double minDistance = 1000; // Arbitrary large number
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr)->triggersZombieVomit())
		{
			double distanceToHuman = (*actorPtr)->distanceToCenter(x, y);
			if (minDistance > distanceToHuman)
			{
				otherX = (*actorPtr)->getX();
				otherY = (*actorPtr)->getY();
				minDistance = distanceToHuman;
			}
			humanExists = true;
		}
	}
	distance = minDistance;
	return humanExists;
}

// Return true if there is a living zombie or Penelope, otherwise false.
// If true, otherX, otherY, and distance will be set to the location and
// distance of the one nearest to (x,y), and isThreat will be set to true
// if it's a zombie, false if a Penelope.
bool StudentWorld::locateNearestCitizenTrigger(double x, double y, double& otherX, double& otherY, double& distance, bool& isThreat) const
{
	bool triggerExists = false;
	double minDistance = 1000; // Arbitrary large number
	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr)->triggersCitizens())
		{
			double distanceToActor = (*actorPtr)->distanceToCenter(x, y);
			if (minDistance > distanceToActor)
			{	
				otherX = (*actorPtr)->getX();
				otherY = (*actorPtr)->getY();
				minDistance = distanceToActor;
			}
			triggerExists = true;
		}
	}
	distance = minDistance;
	isThreat = !(m_actors.front()->isActorAt(otherX, otherY)); // First element of list is Penelope, checks if Penelope is at x,y
	
	return triggerExists;
}

// Return true if there is a living zombie, false otherwise.  If true,
// otherX, otherY and distance will be set to the location and distance
// of the one nearest to (x,y).
bool StudentWorld::locateNearestCitizenThreat(double x, double y, double& otherX, double& otherY, double& distance) const
{
	bool zombieExists = false;
	double minDistance = 1000; // Arbitrary large number

	for (list<Actor *>::const_iterator actorPtr = m_actors.begin(); actorPtr != m_actors.end(); actorPtr++)
	{
		if ((*actorPtr)->threatensCitizens())
		{
			double distanceToZombie = (*actorPtr)->distanceToCenter(x, y);
			if (minDistance > distanceToZombie)
			{
				otherX = (*actorPtr)->getX();
				otherY = (*actorPtr)->getY();
				minDistance = distanceToZombie;
			}
			zombieExists = true;
		}
	}

	distance = minDistance;
	return zombieExists;
}

