#include <algorithm>
#include <set>
#include <ctime>
#include "deck.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_type.h"
#include "card_find.h"

#include <iterator>

//suit  0  Diamond  1  Club   2 Heart     3 Spade
//remove jokers and three of '2'(keep the spade '2') and spade 'A',48 cards in all.
static int card_arr[] = {
0x01, 0x11, 0x21,    		//A 14
				  0x32,		//2 15
0x03, 0x13, 0x23, 0x33,		//3 3
0x04, 0x14, 0x24, 0x34,		//4 4
0x05, 0x15, 0x25, 0x35,		//5 5
0x06, 0x16, 0x26, 0x36,		//6 6
0x07, 0x17, 0x27, 0x37,		//7 7
0x08, 0x18, 0x28, 0x38,		//8 8
0x09, 0x19, 0x29, 0x39,		//9 9
0x0A, 0x1A, 0x2A, 0x3A,		//10 10
0x0B, 0x1B, 0x2B, 0x3B,		//J 11
0x0C, 0x1C, 0x2C, 0x3C,		//Q 12
0x0D, 0x1D, 0x2D, 0x3D		//K 13
};

static int card_arr15[] = {
				0x21,    		//A 14
					  0x32,		//2 15
	0x03, 0x13, 0x23, 0x33,		//3 3
	0x04, 0x14, 0x24, 0x34,		//4 4
	0x05, 0x15, 0x25, 0x35,		//5 5
	0x06, 0x16, 0x26, 0x36,		//6 6
	0x07, 0x17, 0x27, 0x37,		//7 7
	0x08, 0x18, 0x28, 0x38,		//8 8
	0x09, 0x19, 0x29, 0x39,		//9 9
	0x0A, 0x1A, 0x2A, 0x3A,		//10 10
	0x0B, 0x1B, 0x2B, 0x3B,		//J 11
	0x0C, 0x1C, 0x2C, 0x3C,		//Q 12
	0x0D, 0x1D, 0x2D			//K 13
};

void Deck::initcards(std::vector<Card>& v)
{
	v.clear();
	const int cards = sizeof(card_arr) / sizeof(card_arr[0]);
	for (int i = 0; i < cards; ++i) {
		v.push_back(Card(card_arr[i]));
	}
}

void Deck::fill(int type)
{
	cards.clear();
	if (type == 0) {
		for (int i = 0; i < 48; i++)
		{
			Card c(card_arr[i]);
			push(c);
		}
		_cardNo = 16;
	}
	else {
		for (int i = 0; i < 45; i++)
		{
			Card c(card_arr15[i]);
			push(c);
		}
		_cardNo = 15;
	}
}

void Deck::empty()
{
	cards.clear();
}

int Deck::count() const
{
	return cards.size();
}

bool Deck::push(Card card)
{
	cards.push_back(card);
	return true;
}

bool Deck::pop(Card &card)
{
	if (!count())
		return false;

	card = cards.back();
	cards.pop_back();
	return true;
}

bool Deck::remove_d(Card &card)
{
	for(vector<Card>::iterator it=cards.begin(); it!=cards.end();)
	{
		Card temp = *it;
		if (temp.value==card.value)
		{
			it = cards.erase(it);
		}
		else
		{
			++it;
		}
	}
	return true;
}

bool Deck::shuffle(int seed)
{
	srand(time(NULL) + seed);
	random_shuffle(cards.begin(), cards.end());
	return true;
}
int Deck::random_num(int start, int end, int seed)
{
	std::vector<int> myvector;
	for (int i = start; i <= end; ++i) {
		myvector.push_back(i);
	}
	std::srand(unsigned(std::time(NULL)) + seed);
	std::random_shuffle(myvector.begin(), myvector.end());
	return myvector[0];
}

/*srand((unsigned) time(NULL)); //为了提高不重复的概率
rand()%(n - m + 1) + m; */


void Deck::make_boom(HoleCards &holecards){
	CardStatistics stat;
	stat.statistics(cards);
	unsigned int len_c = stat.card4.size();
//	unsigned int i;
//	if (len_c>5)
//	{
//		i = random_num(1,len_c-1,time(NULL));
//	}
	if (len_c>0)
	{
		srand(time(NULL) +1);
		random_shuffle(stat.card4.begin(),stat.card4.end());
		Card card;
		Card random_card;
		random_card = stat.card4.back();
		int count = 0;
		vector<Card>::iterator it = cards.begin();
		for (; it != cards.end();){
			if(random_card == *it){
				card = *it;
				it = cards.erase(it);
				holecards.add_card(card);
				if (holecards.size()>13)
				{
					return;
				}

				count++;
				if(4 == count){
					count = 0;
					return;
				}
			}else{
				++it;
			}
		}
	}
}

void Deck::make_one_line(HoleCards &holecards){
	CardStatistics stat;
	stat.statistics(cards);
	vector<Card> cards_line;
	vector<Card> cards_stra;
	cards_line = stat.line1;
	CardFind::find_straight(cards_line,cards_stra);
	int len_line = cards_stra.size();
	if (len_line>0)
	{
		Card card;
		Card random_card;
		int count = 0;
		int random_face;
		int first_face;
		int last_face;
		int temp_face;
		srand(time(NULL) + 1);
		random_shuffle(cards_stra.begin(),cards_stra.end());
		random_card = cards_stra.back();
		Card::sort_by_ascending(cards_stra);
		first_face = cards_stra.front().face;
		last_face = cards_stra.back().face;
		random_face = random_card.face;
		temp_face = random_face - 4;
		vector<Card>::iterator it = cards.begin();
		if(5 == len_line){
			for(int i = 0; i < len_line;i++){
				for(;it != cards.end();){
					if (cards_stra[i] == *it){
						card = *it;
						it = cards.erase(it);
						holecards.add_card(card);
						if (holecards.size()>13)
						{
							return;
						}
						break;
					}else{
						++it;
					}
				}
			}
		}
		else if(( last_face - random_face) >= 4){
			for(;it != cards.end();){
				if(random_face == it->face){
					random_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
		else if((random_face - first_face) >= 4 && (last_face - random_face) < 4){
			for(;it != cards.end();){
				if(temp_face == it->face){
					temp_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
		else if((random_face - first_face) < 4 && (last_face - random_face) < 4){
			for(;it != cards.end();){
				if(first_face == it->face){
					first_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
	}
}


void Deck::make_three_line(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line3;
	vector<Card> vec_card1;
	Card random_card;

	if (vec_card.size() < 9) {
		return;
	}

	int i = 0;
	for (i = 3; i < (int)vec_card.size(); i += 3) {
		if ((vec_card[i].face == vec_card[i - 3].face + 1)) {
			vec_card1.push_back(vec_card[i]);
		}
	}

	if (vec_card1.size() < 1) {
		return;
	}

	random_card = vec_card1.back();
	for (i = 0; i < 9; i++) {
		for (itcard = cards.begin(); itcard != cards.end(); itcard++) {
			if (itcard->face == random_card.face) {
				holecards.add_card(*itcard);
				cards.erase(itcard);
				if (i%3 == 2) {
					random_card.face -= 1;
				}
				break;
			}
		}
	}

}

void Deck::make_three_two(HoleCards &holecards) {
	Card card;
	Card random_card;
	std::vector<Card> cards_three;
	std::vector<Card> cards_card3;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	cards_card3 = card_stat.line3;
	if(cards_card3.size() == 0)
	{
		return ;
	}
	srand(time(NULL) +1);
	random_shuffle(cards_card3.begin(),cards_card3.end());
	random_card = cards_card3.back();

	int count = 0;
	vector<Card>::iterator it = cards.begin();
	for (; it != cards.end();){
		if(random_card == *it){
			card = *it;
			it = cards.erase(it);
			holecards.add_card(card);
			if (holecards.size()>13)
			{
				return;
			}
			count++;
			if(3 == count){
				count = 0;
				return ;
			}
		}else{
			++it;
		}
	}
}
void Deck::make_double(HoleCards &holecards){
	Card random_card;
	Card card;

	vector<Card> cards_card2;//用来获取是对子的所有的牌
	CardStatistics card_stat;//用来放置整理好了的牌

	int count = 0;
	srand(time(NULL) + 1);
	card_stat.statistics(cards);
	cards_card2 = card_stat.line2;
	int len = cards_card2.size()/2;
	int i  = random_num(1,len,time(NULL));;
	while (i--)
	{
		card_stat.statistics(cards);
		cards_card2 = card_stat.line2;
		srand(time(NULL) +1);
		random_shuffle(cards_card2.begin(),cards_card2.end());
		random_card = cards_card2.back();

		vector<Card>::iterator it = cards.begin();
		for (; it != cards.end();){
			if(random_card == *it){
				card = *it;
				it = cards.erase(it);
				holecards.add_card(card);
				if (holecards.size()>13)
				{
					return;
				}
				count++;
				if(2 == count){
					count = 0;
					break;
				}
			}else{
				++it;
			}
		}
	}

}

void Deck::make_one(HoleCards &holecards){
	Card random_card;
	Card card;

	vector<Card> cards_card1;
	CardStatistics card_stat;

	srand(time(NULL) + 1);
	card_stat.statistics(cards);
	cards_card1 = card_stat.line1;
	//int len = cards_card1.size();
	srand(time(NULL) +1);
	random_shuffle(cards_card1.begin(),cards_card1.end());
	random_card = cards_card1.back();
	vector<Card>::iterator it = cards.begin();
	for (; it != cards.end();){
		if(random_card == *it){
			card = *it;
			it = cards.erase(it);
			holecards.add_card(card);
			if (holecards.size()>13)
			{
				return;
			}
		}else{
			++it;
		}
	}
}


void Deck::make_card()
{
	int card_arr[] = {
		0x01, 0x11, 0x21,    		//A 14
		0x32,		//2 15
		0x03, 0x13, 0x23, 0x33,		//3 3
		0x04, 0x14, 0x24, 0x34,		//4 4
		0x05, 0x15, 0x25, 0x35,		//5 5
		0x06, 0x16, 0x26, 0x36,		//6 6
		0x07, 0x17, 0x27, 0x37,		//7 7
		0x08, 0x18, 0x28, 0x38,		//8 8
		0x09, 0x19, 0x29, 0x39,		//9 9
		0x0A, 0x1A, 0x2A, 0x3A,		//10 10
		0x0B, 0x1B, 0x2B, 0x3B,		//J 11
		0x0C, 0x1C, 0x2C, 0x3C,		//Q 12
		0x0D, 0x1D, 0x2D, 0x3D		//K 13
	};

	cards.clear();
	
	for (int i = 0; i < 48; i++)
	{
		Card c(card_arr[i]);
		push(c);
	}
	_cardNo = 16;
}

void Deck::get_hole_cards(HoleCards &holecards)
{
	Card card;
	srand(time(NULL) +1);
	random_shuffle(cards.begin(),cards.end());
	//holecards.clear();
	int len = holecards.size();
	if (type==1)
	{
		for (int i = 0; i < 5; i++)
		{
			pop(card);
			holecards.add_card(card);
		}
	}
	else if(type==2)
	{
		for (int i = 0; i < (_cardNo - len); i++)
		{
			pop(card);
			holecards.add_card(card);
		}
	}
	else if (type==0)
	{
		for (int i = 0; i < _cardNo; i++)
		{
			pop(card);
			holecards.add_card(card);
		}
	}
}

int Deck::get_random_card()
{
	int ramdom_CARD = rand()%47;//k=x+rand()%(y-x+1)/*k即为所求范围内随机生成的数，rand()%a的结果最大为a-1*/
	return card_arr[ramdom_CARD];

}

void Deck::makecards()
{
	CardStatistics card_stat;
	card_stat.statistics(cards);
	card_stat.debug();
}

void Deck::debug()
{
//	cout<<"size "<<cards.size()<<endl;
	Card::dump_cards(cards);
	//Card::dump_cards(goodcards);
}


/*void Deck::make_cards(HoleCards &holecards, int type)
{
	holecards.clear();
	switch (type) {
	case 10:
		make_plane(holecards);
		break;
	case 15:
		make_boom(holecards);
		break;
	case 2:
		make_one_line(holecards);
		break;
	case 4:
		make_two_line(holecards);
		break;
	case 6:
		make_three_line(holecards);
		break;
	case 8:
		make_three_two(holecards);
		break;
	default:
		//get_hole_cards(holecards);
		break;
	}
}*/

void Deck::create_zhadan(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.card4;
	Card random_card;

	if (vec_card.size() < 4) {
		return;
	}

	random_shuffle(vec_card.begin(),vec_card.end());
	random_card = vec_card.back();
	int i = 0;
	for (itcard = cards.begin(); itcard != cards.end();) {
		if (itcard->face == random_card.face) {
			holecards.add_card(*itcard);
			itcard = cards.erase(itcard);
			i++;
		} else {
			itcard++;
		}

		if (i == 4) {
			break;
		}
	}
}

void Deck::create_duizi(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line2;
	Card random_card;

	if (vec_card.size() < 2) {
		return;
	}

	random_shuffle(vec_card.begin(),vec_card.end());
	random_card = vec_card.back();
	int i = 0;
	for (itcard = cards.begin(); itcard != cards.end();) {
		if (itcard->face == random_card.face) {
			holecards.add_card(*itcard);
			itcard = cards.erase(itcard);
			i++;
		} else {
			itcard++;
		}

		if (i == 2) {
			break;
		}
	}
}

void Deck::create_sandai(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line3;
	Card random_card;

	if (vec_card.size() < 3) {
		return;
	}

	random_shuffle(vec_card.begin(),vec_card.end());
	random_card = vec_card.back();
	int i = 0;
	for (itcard = cards.begin(); itcard != cards.end();) {
		if (itcard->face == random_card.face) {
			holecards.add_card(*itcard);
			itcard = cards.erase(itcard);
			i++;
		} else {
			itcard++;
		}

		if (i == 3) {
			break;
		}
	}
}

void Deck::create_liandui(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line2;
	vector<Card> vec_card1;
	Card random_card;

	if (vec_card.size() < 4) {
		return;
	}

	int i = 0;
	for (i=2; i<(int)vec_card.size(); i+=2) {
		if ((vec_card[i].face == vec_card[i-2].face + 1)) {
			vec_card1.push_back(vec_card[i]);
		}
	}

	if (vec_card1.size() < 1) {
		return;
	}

	random_shuffle(vec_card1.begin(),vec_card1.end());
	random_card = vec_card1.back();
	for (i=0; i<4; i++) {
		for (itcard = cards.begin(); itcard != cards.end(); itcard++) {
			if (itcard->face == random_card.face) {
				holecards.add_card(*itcard);
				cards.erase(itcard);
				if (i == 1) {
					random_card.face -= 1;
				}
				break;
			}
		}
	}
}

void Deck::create_feiji(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line3;
	vector<Card> vec_card1;
	Card random_card;

	if (vec_card.size() < 6) {
		return;
	}

	int i = 0;
	for (i=3; i<(int)vec_card.size(); i+=3) {
		if ((vec_card[i].face == vec_card[i-3].face + 1)) {
			vec_card1.push_back(vec_card[i]);
		}
	}

	if (vec_card1.size() < 1) {
		return;
	}

	random_shuffle(vec_card1.begin(),vec_card1.end());
	random_card = vec_card1.back();
	for (i=0; i<6; i++) {
		for (itcard = cards.begin(); itcard != cards.end(); itcard++) {
			if (itcard->face == random_card.face) {
				holecards.add_card(*itcard);
				cards.erase(itcard);
				if (i == 2) {
					random_card.face -= 1;
				}
				break;
			}
		}
	}
}

void Deck::create_shunzi(HoleCards &holecards)
{
	vector<Card>::iterator itcard;
	CardStatistics card_stat;
	card_stat.statistics(cards);
	vector<Card> vec_card = card_stat.line1;
	vector<Card> vec_card1;
	Card random_card;

	if (vec_card.size() < 5) {
		return;
	}

	int i = 0;
	for (i=4; i<(int)vec_card.size(); i++) {
		if ((vec_card[i].face == vec_card[i-1].face + 1) && (vec_card[i-1].face == vec_card[i-2].face + 1) &&
			(vec_card[i-2].face == vec_card[i-3].face + 1) && (vec_card[i-3].face == vec_card[i-4].face + 1)) {
			vec_card1.push_back(vec_card[i]);
		}
	}

	if (vec_card1.size() < 1) {
		return;
	}

	random_shuffle(vec_card1.begin(),vec_card1.end());
	random_card = vec_card1.back();
	for (i=0; i<5; i++) {
		for (itcard = cards.begin(); itcard != cards.end(); itcard++) {
			if (itcard->face == random_card.face) {
				holecards.add_card(*itcard);
				cards.erase(itcard);
				random_card.face -= 1;
				break;
			}
		}
	}
}

void Deck::create_one(HoleCards &holecards){
	random_shuffle(cards.begin(),cards.end());

	int i = 0;
	for (i=holecards.size(); i<_cardNo; i++) {
		holecards.add_card(cards.back());
		cards.pop_back();
	}
}
