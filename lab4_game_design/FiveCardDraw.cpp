#include "stdafx.h"
#include <memory>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include<random>
#include "FiveCardDraw.h"
#include "Card.h"
#include <iostream>
using namespace std;


FiveCardDraw::FiveCardDraw() :dealerpos(0) {
	Card::RANK allranks[] = { Card::RANK::two, Card::RANK::three, Card::RANK::four, Card::RANK::five, Card::RANK::six, Card::RANK::seven, Card::RANK::eight, Card::RANK::nine, Card::RANK::ten, Card::RANK::jack, Card::RANK::queen, Card::RANK::king, Card::RANK::ace };
	Card::SUIT allsuits[] = { Card::SUIT::club, Card::SUIT::diamond, Card::SUIT::heart, Card::SUIT::spade };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 13; j++) {
			//struct Card card1 = { allsuits[i], allranks[j] };
			Card card1 = Card(allsuits[i], allranks[j]);
			mainDeck.add_card(card1);
		}
	}
}

int FiveCardDraw::before_turn(Player& someplayer) {
	if (someplayer.isfold) { return 0; };
	cout << "Name: " << someplayer.name << endl;
	cout << "Hand: " << someplayer.hand << endl;
	cout << "index of cards to discard (from 1 to 5). Enter 0 if don't want to discard any:" << endl;
	cout << "if you want to discard more than one card, type in space between each index." << endl;
	string indexes;

	int index;
	vector<size_t> vecindex;
	stringstream ss;
	bool invalidinput = true;

	while (invalidinput) { //iterates until valid input
		bool inrange = true;
		ss.clear();
		vecindex.clear();
		getline(cin, indexes);
		//cin.ignore();
		ss << indexes;
		while (ss >> index) { //iterates through the input
			if (index > someplayer.hand.size()) { inrange = false; } //check if input index is in range
			else {
				vecindex.push_back(index);
			}
		}
		sort(vecindex.begin(), vecindex.end()); //detect the 0 in input as well as sort for later removal
		if ((vecindex.size() <= 5) && inrange) {
			invalidinput = false;
		}
		else if (vecindex.size() > 1 && vecindex[0] == 0) { invalidinput = true; } //if 0 is not the only input index, the input is invalid
		else {
			invalidinput = true;
		}
	}
	if (!(vecindex.size() == 1 && vecindex[0] == 0)) { //check if player want to discard any cards
		unsigned int i = 0;

		for (unsigned int i = 0; i < vecindex.size(); i++) {
			discardDeck.add_card(someplayer.hand[vecindex[i] - i]);
			someplayer.hand.remove_card(vecindex[i] - i);
		}
	}
	return 0;
}


int FiveCardDraw::before_round() {
	mainDeck.shuffle();
	int numPlayers = players.size();
	int c = 0;
	while (c < 5) { //dealing the cards
		//start from the next player after dealer
		for (int i = 1; i <= numPlayers; i++) {
			(*players[(i + dealerpos) % numPlayers]).hand << mainDeck;
		}
		
		c++;
	}


	pot = 0;
	for (int i = 0; i < numPlayers; i++) {
		(*players[i]).chipaccount -= 1;
		pot += 1;
		(*players[i]).isfold = false;    //reset the folded flag
		(*players[i]).chipbet = 0;    //reset the chipbet each round
		cout <<(*players[i]).name<<"'s hand: "<< (*players[i]).hand << endl;
	}

	numfolded = 0;
	roundDonebyFold = false;
	betPhase(); //the first bet phase
	if (roundDonebyFold) return 0;// this round already end;
	//start from the next player after dealer
	for (int j = 1; j <= numPlayers; j++) {
		before_turn((*players[(j + dealerpos) % numPlayers]));
	}
	return 0;
}

int FiveCardDraw::round() {
	if (roundDonebyFold) return 0;// this round already end;
	int numPlayers = players.size();
	for (int j = 1; j <= numPlayers; j++) {
		(*players[j - 1]).chipbet = 0;	//reset the chipbet each phase
	}

	for (int j = 1; j <= numPlayers; j++) {
		int k = turn((*players[(j + dealerpos) % numPlayers]));
		if (k != 0) { return k; }
		after_turn((*players[(j + dealerpos) % numPlayers]));
	}

	betPhase(); //the second bet phase

	return 0;
}

int FiveCardDraw::turn(Player &player) {
	int numReplace = 5 - player.hand.size();
	/*
	if (numReplace > 0) {
		if (mainDeck.size() + discardDeck.size() < numReplace) {
			cout << "Not enough cards left to replace discarded cards" << endl;
			return 1;
		}
		else {
			if (mainDeck.size() >= numReplace) {
				for (int i = 0; i < numReplace; i++) {
					player.hand << mainDeck;
				}
			}
			else {
				int mainRe = mainDeck.size();
				for (int j = 0; j < mainRe; j++) {
					player.hand << mainDeck;
				}
				discardDeck.shuffle();
				int disRe = numReplace - mainRe;
				for (int k = 0; k < disRe; k++) {
					player.hand << discardDeck;
				}
			}
		}
	}
	*/
	while (numReplace != 0) {
		if (mainDeck.size() == 0) {
			if (discardDeck.size() == 0) {
				cout << discardDeck.size() << endl;
				cout << "Not enough cards left to replace discarded cards" << endl;
				return 5;
			}
			discardDeck.shuffle();
			player.hand << discardDeck;
		}
		else player.hand << mainDeck;
		--numReplace;
	}
	return 0;
}

int FiveCardDraw::after_turn(Player &player) {
	if (player.isfold) { return 0; };
	cout << "Name: " << player.name << endl;
	cout << "Hand (after_turn): " << player.hand << endl;
	return 0;
}

bool FiveCardDraw::player_Poker_rank(shared_ptr<Player> player1, shared_ptr<Player> player2) {
	if (!player1) return false;
	else {
		if (!player2) return true;
		else return poker_rank(player1->hand, player2->hand);
	}
}

int FiveCardDraw::after_round() {
	vector<shared_ptr<Player>> temp(players);
	sort(temp.begin(), temp.end(), &player_Poker_rank);

	bool winPrint = true;
	for (auto i = temp.begin(); i != temp.end(); i++) {
		if ((!(*i)->isfold) && (winPrint == true)) {
			(*i)->win++;
			(*i)->chipaccount += pot;
			if (!roundDonebyFold) {
				cout << (*(*i)) << "won with hand: " << endl;
				cout << (*i)->hand << endl;
			}
			else {
				cout << (*(*i)) << "won" << endl;
			}

			winPrint = false;


		}
		else {
			(*i)->los++;
			if (!(*i)->isfold) {
				cout << (*(*i)) << "lost with hand: " << endl;
				cout << (*i)->hand << endl;
			}
			else {
				cout << (*(*i)) << "folded." << endl;
			}
		}
		for (size_t j = 5; j > 0; j--) {
			mainDeck.add_card((*i)->hand[j]);
			(*i)->hand.remove_card(j);
		}
	}

	mainDeck.cards.insert(mainDeck.cards.end(),discardDeck.cards.begin(),discardDeck.cards.end());
	discardDeck.cards.clear();


	while (1) {

		cout << "Is there any player want to leave? (yes/no) " << endl;
		string leave;
		cin >> leave;
		cin.ignore();
		
		if (leave == "no") {
			break;
		}
		if(leave == "yes") {  
			cout << "Please tell me the name of the player? " << endl;
			string pname;
			cin >> pname;
			cin.ignore();

			shared_ptr<Player> player = find_player(pname);

			if (player) {
				ofstream out;
				out.open(player->name);
				out << player->name << " " << player->win << " " << player->los <<" "<<player->chipaccount<< endl;
				out.close();
				cout<<player->name<< " is leaving." << endl;
				for (auto k = players.begin(); k != players.end(); ++k) {
					if ((*k)->name == pname) {
						players.erase(k);
						break;
					}
				}
			
			}
			else cout << "Don't find that player." << endl;
			if (players.size() == 0) {
				cout << "There's no player left ." << endl;
				break;
			}
		}

	}

	while (1) {
		if (players.size() == 10) {
			cout << "Maximum number of players reached. (10 players)" << endl;
			break;
		}
		cout << "Is there any player want to join the game? (yes/no)" << endl;
		string join;
		cin >> join;
		cin.ignore();
		if (join == "no") {
			break;
		}
		if (join =="yes") {
			cout << "Please tell me the name of the player? " << endl;
			string pname;
			cin >> pname;
			cin.ignore();

			try {
				add_player(pname);
			}
			catch (const char* msg) {
				cout << msg << endl;
			}
		}
	}
	for (auto k = players.begin(); k != players.end(); ++k) {
		if ((*k)->broke()) {
			while (1) {
				cout << (*k)->name << " is broke. You have to quit or put 20 in. Type (quit/continue):" << endl;
				string action3;
				cin >> action3;
				if (action3 == "quit") {
					ofstream out;
					out.open((*k)->name);
					out << (*k)->name << " " << (*k)->win << " " << (*k)->los << " " << (*k)->chipaccount << endl;
					out.close();
					cout << (*k)->name << " is leaving." << endl;
					players.erase(k);
					break;
				}
				if (action3 == "continue") {
					(*k)->chipaccount = 20;
					break;
				}
			}
		}	
	}

	if (dealerpos<players.size()) dealerpos++;
	else dealerpos = 0;


	return 0;

}

int FiveCardDraw::betPhase() { // new add
	int numPlayers = players.size();
	int k = 0;
	canCheck = true;
	maxbet = 0;

	int idle = 0;
	for (int j = 0; j < numPlayers; j++) {
		if ((*players[j]).broke() || ((*players[j]).isfold)) {
			idle++;
		}
	}
	if (idle == (numPlayers - 1)) {
		return 0;
	}

	while (1) {
		if (roundDonebyFold) break;
		for (int j = 1; j <= numPlayers; j++) {
			if (numfolded < (numPlayers - 1)) { //if number of players folded < everyone - 1
				k += betturn((*players[(j + dealerpos) % numPlayers]));
			}
			else {
				roundDonebyFold = true;
				break;
			}
		}
		if (numfolded == (numPlayers - 1)) { //everybody folded
			roundDonebyFold = true;
			break;
		};
		if (k == numPlayers) { //everybody checks
			break;
		}

		int samebet = 0;
		int numbetting = 0;
		for (int i = 0; i < numPlayers; i++) {
			if (!(((*players[i]).broke()) || (*players[i]).isfold)) { //if player didn't fold or didn't run out of money
				numbetting += 1;
				if ((*players[i]).chipbet == maxbet) {
					samebet += 1;
				}
			}
		}
		if (samebet == numbetting) {
			break;
		}
	}

	return 0;
}

int FiveCardDraw::betturn(Player &player) {
	if ((!canCheck) && (!player.isfold) && (!player.broke()) && (player.chipbet != maxbet)) {
		cout << player.name << "'s turn begins. Chips bet in the phase:" << player.chipbet << " Chips remaining: " << player.chipaccount << " Pot: " << pot << endl;

		while (1) {
			cout << "What do you wanna do? (fold/call/raise) " << endl;
			string action2;
			cin >> action2;
			cin.ignore();
			if (action2 == "fold") {
				player.isfold = true;
				numfolded += 1;
				break;
			}
			if (action2 == "call") {
				player.chipaccount -= (maxbet - player.chipbet);
				if (player.chipaccount >= 0) {
					pot += (maxbet - player.chipbet);
					player.chipbet = maxbet;
				}
				else {
					pot += (maxbet + player.chipaccount);
					player.chipbet += (maxbet + player.chipaccount);
					player.chipaccount = 0;
				}
				break;
			}
			if (action2 == "raise") {
				int amountraise;
				cout << "How much do you wanna raise? (1 or 2)" << endl;
				cin >> amountraise;
				cin.ignore();
				maxbet += amountraise;
				player.chipaccount -= (maxbet - player.chipbet);
				pot += (maxbet - player.chipbet);
				player.chipbet = maxbet;
				break;
			}
		}
		cout << player.name << "'s bet turn ends. Chips bet in the phase: " << player.chipbet << " Chips remaining: " << player.chipaccount << endl;
	}
	if (canCheck && (!player.isfold) && (!player.broke())) {
		cout << player.name << "'s turn begins. Chips bet in the phase:" << player.chipbet << " Chips remaining: " << player.chipaccount << " Pot: " << pot << endl;

		while (1) {

			cout << "What do you wanna do? (check/bet) " << endl;
			string action1;
			cin >> action1;
			cin.ignore();

			if (action1 == "check") {
				cout << "checked" << endl;
				return 1;
			}
			if (action1 == "bet") {
				canCheck = false;
				int amountbet;
				cout << "How much do you wanna bet? (1 or 2)" << endl;
				cin >> amountbet;
				cin.ignore();
				maxbet = amountbet;
				player.chipbet += amountbet;
				player.chipaccount -= amountbet;
				pot += amountbet;
				break;
			}
		}
		cout << player.name << "'s bet turn ends. Chips bet in the phase: " << player.chipbet << " Chips remaining: " << player.chipaccount << endl;
	}
	return 0;
}

