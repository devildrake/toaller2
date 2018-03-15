#pragma once
#include <iostream>
#include <mutex>
#include <SFML\Network.hpp>

std::mutex m;
void Print(std::string s){
	m.lock();
	std::cout << s << std::endl;
	m.unlock();
}

class Player {
public:
	std::string userName;
	bool alive;
	//Bool para el turno anterior
	bool wasAlive;
	enum ROLE{_WOLF = 1,_VILLAGER = 0,_WITCH = 2,_CHILD = 3,_UNKNOWN=4};
	enum Turn { _DAY, _WOLVES, _WITCHTURN };

	ROLE role;
	int id;

	std::string GetRoleAsString() {
		std::string temp = "";

		switch (role) {
		case ROLE::_VILLAGER:
			temp = "Villager";
			break;
			case ROLE::_WOLF:
			temp = "Wolf";
			break;
		case ROLE::_WITCH:
			temp = "BWitch";
			break;
		case ROLE::_CHILD:
			temp = "Child";
			break;
		}

		return temp;
	}

	std::string GetUserName() {
		return userName;
	}

	Player() {
		wasAlive = true;
		alive = true;
		role = ROLE::_UNKNOWN;

	}

	Player(std::string n, int id) {
		userName = n;
		this->id = id;
		wasAlive = true;
		alive = true;
		role = ROLE::_UNKNOWN;
	}

};

class PlayerServer:public Player {
public:

	sf::TcpSocket* socket;
	int currentVotes = 0;
	bool voted = false;

	PlayerServer(sf::TcpSocket* s, std::string name) {
		socket = s;
		userName = name;
		currentVotes = 0;
		wasAlive = true;
		alive = true;
		role = ROLE::_VILLAGER;

	}

	PlayerServer() {
		wasAlive = true;
		alive = true;
		role = ROLE::_VILLAGER;

	}

};