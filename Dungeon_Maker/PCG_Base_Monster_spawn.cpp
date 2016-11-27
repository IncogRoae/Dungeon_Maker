#include<iostream>
#include<vector>
#include<string>
#include<cstdlib>
#include<ctime>

#define NUMOFSTAGE 15
#define MAXGENERATION 10

using namespace std;

class GameObject {
	string name;
	int level;
	int exp;
	int hp;

public:
	GameObject(string name, int level, int exp, int hp) : name(name), level(level), exp(exp), hp(hp) {};

	string getName() { return name; }

	int getLevel() { return level; }
	void setLevel() { level++; }

	int getExp() { return exp; }
	void setExp(int _exp) { exp += _exp; }

	int getHp() { return hp; }

	virtual void printInfo() = 0;
};

class Player :public GameObject {
	int stage;
	int atk;
	int GoalExp[10] = { 5, 10, 20, 50, 100, 150, 200, 400, 600, 800 };

public:
	Player(string name, int level, int exp, int hp, int atk, int stage) : GameObject(name, level, exp, hp), atk(atk), stage(stage) {}

	void LevelUP() {
		setLevel();
		atk += 5;
	}

	void isLevelUP() {
		if (getExp() >= GoalExp[getLevel()-1]) {
			setExp(GoalExp[getLevel()-1] - getExp());
			LevelUP();
		}
	}

	int getAtk() { return atk; }

	void setStage() { stage++; }
	int getStage() { return stage; }

	void printInfo() {
		cout << "PLAYER INFO : " 
			<< getName() << " : LEVEL["
			<< getLevel() << "] : ATK["
			<< getAtk() << "] : EXP[" 
			<< getExp() << "] " <<endl;
	}
};

class Monster :public GameObject {
public:
	Monster(string name, int level, int exp, int hp) : GameObject(name, level, exp, hp) {}

	void printInfo() {
		cout << getName() << " : EXP["
			<< getExp() << "] : HP["
			<< getHp() << "]" << endl;
	}

};

class saveData {
	Player player;
	vector<Monster> monsters;
	int score;

public:
	saveData(Player player, vector<Monster> monsters, int score) : player(player), monsters(monsters), score(score) {}

	int getScore() { return score; }
	Player getPlayer() { return player; }
	vector<Monster> getMonster() { return monsters; }

};

void createDungeon(Player *player, const vector<Monster> *monsters, vector<Monster> *dungeon) {
	int monstercount = (rand() % 5) + 1;
	int monsterIndex;

	for (int i = 0; i<monstercount; i++) {
		/*if(player->getLevel() >= 3) monsterIndex = rand() % (player->getLevel() - (player->getLevel() - 1) + 2) + (player->getLevel() - 1);
		else monsterIndex = rand() % (player->getLevel() + 1);*/
		//생성할 몬스터의 최소 레벨값을 조정하려 헀으나 낮은 레벨의 몬스터가 출현하는 던전은 자연스레 스코어가 낮아 도태되어질것임으로 배제함.
		monsterIndex = rand() % (player->getLevel() + 1);
		dungeon->push_back(monsters->at(monsterIndex));
	}
}

int huntMonster(Player *player, Monster mon) {
	int hp = mon.getHp();
	int timer = 0;

	while (hp > 0) {
		hp -= player->getAtk();
		timer++;
	}

	player->setExp(mon.getExp());

	return timer;
}

int enterDungeon(Player *player, vector<Monster> *dungeon) {
	int monsterCount = dungeon->size();
	int playTime = 0;

	for (auto iter = dungeon->begin(); iter != dungeon->end(); ++iter) {
		playTime += huntMonster(player, *iter);
	}
	player->isLevelUP();

	return playTime;
}

saveData clearDungeon(Player player, vector<Monster> *dungeon, int playTime) {
	//적합도 계산 = 스테이지 보정값 * 캐릭터 레벨 * 획득한 경험치량 - 플레이타임
	int score = (NUMOFSTAGE - player.getStage() + 1) * player.getLevel() * player.getExp() - playTime;
	
	saveData data(player, *dungeon, score);

	return data;
}

saveData wheel(vector<saveData> data) {

	int sumOfAll = 0;
	int point = 0;
	int sum = 0;

	for (auto iter = data.begin(); iter != data.end(); ++iter) {
		sumOfAll += (*iter).getScore();
	}

	point = rand() % (sumOfAll + 1);

	for (auto iter = data.begin(); iter != data.end(); ++iter)
	{
		sum += (*iter).getScore();
		if (point < sum)
			return *iter;
	}

}


saveData corssOver(Player *player, saveData firstData, saveData secondData) {
	vector<Monster> firstMonsters;
	vector<Monster> secondMonsters;
	vector<Monster> newMonsters;

	if (firstData.getMonster().size() > secondData.getMonster().size()) {
		firstMonsters = secondData.getMonster();
		secondMonsters = firstData.getMonster();
	}
	else {
		firstMonsters = firstData.getMonster();
		secondMonsters = secondData.getMonster();
	}

	int numOfMonsters = (firstMonsters.size() + secondMonsters.size()) / 2;
	int index;
	
	if (firstMonsters.size() < numOfMonsters) index = firstMonsters.size();
	else if(secondMonsters.size() < numOfMonsters) index = secondMonsters.size();
	else index= rand() % numOfMonsters;

	for (int i = 0; i < index; i++) {
		newMonsters.push_back(firstMonsters.at(i));
	}
	for (int i = index; i < numOfMonsters; i++) {
		newMonsters.push_back(secondMonsters.at(i));
	}

	return clearDungeon(*player, &newMonsters, enterDungeon(player, &newMonsters));
}

saveData crossOver(Player player, vector<saveData> data) {
	int size = data.size();
	int random = rand() % size;
	return corssOver(&player, data.at(rand() % size), data.at(rand() % size));

}

void mutation(vector<saveData> *data, vector<Monster> monsters) {
	vector<Monster> dungeon;

	int index = rand() % data->size();

	createDungeon(&data->at(index).getPlayer(), &monsters, &dungeon);

	data->at(index) = clearDungeon(data->at(index).getPlayer(), &dungeon, enterDungeon(&data->at(index).getPlayer(), &dungeon));
}

void evolution(Player *player, vector<saveData> data) {

	int index = rand() % data.size();
	saveData tempData = data.at(index);
	vector<Monster> tempMonsters = tempData.getMonster();


	*player = tempData.getPlayer();

	player->printInfo();

	for (auto itor = tempMonsters.begin(); itor != tempMonsters.end(); ++itor) {
		itor->printInfo();
	}

	cout << "==================================================" << endl;
}

void main() {

	srand((unsigned int)time(NULL));

	string monsterName[10]	= { "Slime", "Goblin", "Imp", "Orc", "Werewolf", "Ogre", "Golem", "Lich", "Hydra", "Dragon" };
	int getExp[10]			= { 1, 2, 5, 7, 10, 15, 20, 30, 50, 100 };
	int hp[10]				= { 5, 10, 15, 20, 50, 100, 200, 300, 500, 1000 };


	vector<saveData> simulationData;
	vector<saveData> selectData;
	vector<saveData> crossoverData;
	vector<saveData> mutationData;

	vector<Monster> monsters;

	Player player("pjc", 1, 0, 10, 1, 1);

	while (player.getStage() <= NUMOFSTAGE) {
		for (int i = 0; i < 10; i++) {
			Monster monster(monsterName[i], i + 1, getExp[i], hp[i]);
			monsters.push_back(monster);
		}

		for (int t = 0; t < 100; t++) {
			Player tempPlayer = player;

			vector<Monster> dungeon;

			createDungeon(&tempPlayer, &monsters, &dungeon);
			simulationData.push_back(clearDungeon(tempPlayer, &dungeon, enterDungeon(&tempPlayer, &dungeon)));
		}

		int count = 0;
		while (count < MAXGENERATION) {
			for (int t = 0; t < 50 / (count + 1); t++) {
				selectData.push_back(wheel(simulationData));
			}


			for (int t = 0; t < 50 / (count + 1); t++) {
				Player tempPlayer = player;
				crossoverData.push_back(crossOver(tempPlayer, selectData));
			}
			count++;

			if (count == 9) {
				cout << endl;
			}

			float random = rand() % 100 * 0.0001f;
			float mutationVal = 100 - random*(1 - count / MAXGENERATION);

			if (mutationVal <= 0.003f) {
				mutation(&crossoverData, monsters);
				simulationData = mutationData;
			}
			else simulationData = crossoverData;



			selectData.clear();
			crossoverData.clear();
			mutationData.clear();
		}

		evolution(&player, simulationData);
		player.setStage();
	}
	char a;
	cin >> a;
}