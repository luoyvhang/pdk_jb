#include "hole_cards.h"
#include "card.h"
#include "hole_cards.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_find.h"

HoleCards::HoleCards()
{

}

void HoleCards::add_card(Card card)
{
	cards[card.value] = card;
}

int HoleCards::findCard(int value)
{
	if (cards.find(value) != cards.end())
		return 1;
	/*
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		if (value == it->second.value)
		{
			return 1;
		}
	}
	*/
	return 0;
}

void HoleCards::copy_cards(std::vector<Card> *v)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		v->push_back(it->second);
	}
}

void HoleCards::copy_cards(std::vector<int> *v)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		Card &card = it->second;
		v->push_back(card.value);
	}
}

int HoleCards::get_one_little_card()
{
	std::vector<Card> v;

	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		v.push_back(it->second);
	}

	Card::sort_by_descending(v);

	Card card = v.back();

	cards.erase(card.value);

	return card.value;
}



int HoleCards::robot(std::vector<int> &v)
{
	std::vector<int> cards_int;
	copy_cards(&cards_int);

	// to find straight
	CardFind::find_straight(cards_int, v); // find the longest
	if (v.size() > 0)
	{
		//remove(v);
		return 0;
	}
	// can not find straight
	vector<Card> cards_obj;
	copy_cards(&cards_obj);
	CardStatistics card_stat;
	card_stat.statistics(cards_obj);

	// to find three with two;if no more cards,can be three with one or zero
	if (card_stat.card3.size() > 0)
	{
		v.push_back(card_stat.card3[0].value);
		v.push_back(card_stat.card3[1].value);
		v.push_back(card_stat.card3[2].value);

		if (card_stat.card2.size() > 0)
		{
			v.push_back(card_stat.card2[0].value);
			v.push_back(card_stat.card2[1].value);
			//remove(v);
			return 0;
		}

		if (card_stat.card1.size() > 1)
		{
			if (card_stat.card1[0].face != 16 && card_stat.card1[0].face != 17)
			{
				v.push_back(card_stat.card1[0].value);
				v.push_back(card_stat.card1[1].value);
				//remove(v);
				return 0;
			}
		}
		else if (card_stat.card4.size()>0)
		{
			v.push_back(card_stat.card4[0].value);
			v.push_back(card_stat.card4[1].value);
			return 0;
		}
		else  if (card_stat.card1.size() > 0)
		{
			v.push_back(card_stat.card1[0].value);
			return 0;
		}

		//remove(v);
		return 0;
	}

	//the smallest single card
	if (card_stat.card1.size() > 0)
	{
		v.push_back(card_stat.card1[0].value);
		//remove(v);
		return 0;
	}

	//the boom
	if (card_stat.card4.size() > 0)
	{
		v.push_back(card_stat.card4[0].value);
		v.push_back(card_stat.card4[1].value);
		v.push_back(card_stat.card4[2].value);
		v.push_back(card_stat.card4[3].value);
		//remove(v);
		return 0;
	}

	return -1;
}

/*int HoleCards::discard(int L,CardType T,std::vector<Card> &v)
{
CardType t = T;
int len = L;
vector<Card> cards_obj;
copy_cards(&cards_obj);
CardStatistics card_stat;
card_stat.statistics(cards_obj);
switch (t)
{
case CARD_TYPE_ERROR:
break;
case CARD_TYPE_ONE:
if (card_stat.card1.size() > 0)
{
v.push_back(card_stat.card1[0]);
return 0;
}
break;
case CARD_TYPE_ONELINE:
break;
case CARD_TYPE_TWO:
break;
case CARD_TYPE_TWOLINE:
break;
case CARD_TYPE_THREE:
//game over
break;
case CARD_TYPE_THREELINE:
break;
case CARD_TYPE_THREEWITHONE:
//game over
break;
case CARD_TYPE_THREEWITHTWO:

break;
case CARD_TYPE_PLANEWITHONE:
//game over
break;
case CARD_TYPE_PLANEWITHWING:
break;
case CARD_TYPE_PLANWHITHLACK:
//game over
break;
case CARD_TYPE_FOURWITHONE:
break;
case CARD_TYPE_FOURWITHTWO:
break;
case CARD_TYPE_FOURWITHTHREE:
break;
case CARD_TYPE_BOMB:
break;
default:
break;
}
}*/

/*int HoleCards::trustship(std::vector<Card> &last,std::vector<Card> &cur)
{
if (last.size() == 0)
{
return -1;
}

CardStatistics card_stat_last;
card_stat_last.statistics(last);
CardAnalysis card_ana_last;
card_ana_last.analysis(card_stat_last);


vector<Card> cards_hand;
copy_cards(&cards_hand);
CardStatistics card_stat_hand;
card_stat_hand.statistics(cards_hand);
CardAnalysis card_ana_hand;
card_ana_hand.analysis(card_stat_hand);

switch (card_ana_last.type)
{
case CARD_TYPE_ERROR:
break;
case CARD_TYPE_ONE:
if (card_stat0.card1.size() > 0)
{
cur.push_back(card_stat0.card1[0]);
return 0;
}
break;
case CARD_TYPE_ONELINE:
break;
case CARD_TYPE_TWO:
break;
case CARD_TYPE_TWOLINE:
break;
case CARD_TYPE_THREE:
//game over
break;
case CARD_TYPE_THREELINE:
break;
case CARD_TYPE_THREEWITHONE:
//game over
break;
case CARD_TYPE_THREEWITHTWO:

break;
case CARD_TYPE_PLANEWITHONE:
//game over
break;
case CARD_TYPE_PLANEWITHWING:
break;
case CARD_TYPE_PLANWHITHLACK:
//game over
break;
case CARD_TYPE_FOURWITHONE:
break;
case CARD_TYPE_FOURWITHTWO:
break;
case CARD_TYPE_FOURWITHTHREE:
break;
case CARD_TYPE_BOMB:
break;
default:
break;
}
return 0;
}*/

int HoleCards::robot(std::vector<Card> &v)//the first to output cards 
{
	std::vector<Card> cards_int;
	copy_cards(&cards_int);

	// to find straight
	CardFind::find_straight(cards_int, v);
	if (v.size() > 0)
	{
		//remove(v);
		//cout<<"find the straight"<<endl;
		return 0;
	}

	//can not find the straight
	vector<Card> cards_obj;
	copy_cards(&cards_obj);
	CardStatistics card_stat;
	card_stat.statistics(cards_obj);

	// to find three or with one or with two
	if (card_stat.card3.size() > 0)
	{
		if (card_stat.card1.size() > 1)
		{
			//if (card_stat.card1[0].face != 16 && card_stat.card1[0].face != 17)
			//{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card1[0]);
			v.push_back(card_stat.card1[1]);

			return 0;
			//}
		}
		if (card_stat.card2.size() > 0)
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card2[0]);
			v.push_back(card_stat.card2[1]);

			return 0;
		}
		if (card_stat.card3.size() > 3)
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card3[3]);
			v.push_back(card_stat.card3[4]);

			return 0;
		}
		if (card_stat.card4.size() > 0)//four with three
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card4[0]);
			v.push_back(card_stat.card4[1]);
			v.push_back(card_stat.card4[2]);
			v.push_back(card_stat.card4[3]);

			return 0;
		}
		if (card_stat.card1.size() > 0)
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card1[0]);

			return 0;
		}

	}

	//the lastest card
	if (card_stat.len == card_stat.card3.size()) {
		v.push_back(card_stat.card3[0]);
		v.push_back(card_stat.card3[1]);
		v.push_back(card_stat.card3[2]);
		return 0;
	}

	//can not find three with one or two
	if (card_stat.card2.size() > 0)
	{
		v.push_back(card_stat.card2[0]);
		v.push_back(card_stat.card2[1]);
		//remove(v);

		return 0;
	}

	if (card_stat.card1.size() > 0)
	{
		v.push_back(card_stat.card1[0]);
		//remove(v);
		return 0;
	}

	if (card_stat.card4.size() > 0)
	{
		v.push_back(card_stat.card4[0]);
		v.push_back(card_stat.card4[1]);
		v.push_back(card_stat.card4[2]);
		v.push_back(card_stat.card4[3]);
		//remove(v);

		return 0;
	}

	if (card_stat.line1.size() > 0)
	{
		v.push_back(card_stat.line1[0]);
		//remove(v);

		return 0;
	}

	return -1;
}


void HoleCards::remove(std::vector<Card> &v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		cards.erase(v[i].value);
	}
}

void HoleCards::remove(std::vector<int> &v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		cards.erase(v[i]);
	}
}

int HoleCards::size()
{
	return (int)(cards.size());
}

void HoleCards::debug()
{
	Card::dump_cards(cards);
}
