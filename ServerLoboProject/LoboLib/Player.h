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
	enum ROLE{_WOLF = 1,_VILLAGER = 0,_CHILD = 2,_UNKNOWN=3};
	enum Turn { _DAY, _WOLVES };

	ROLE role;
	int id;

	std::string GetRoleAsString() {
		std::string temp = "";

		switch (role) {
		case ROLE::_VILLAGER:
			temp = "V";
			break;
			case ROLE::_WOLF:
			temp = "W";
			break;
		case ROLE::_UNKNOWN:
			temp = "U";
			break;
		case ROLE::_CHILD:
			temp = "C";
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

	~PlayerServer() {
		if(socket!=nullptr)
		delete socket;
	}

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