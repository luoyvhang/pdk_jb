#ifndef _DECK_H_
#define _DECK_H_

#include <vector>

#include "card.h"
#include "hole_cards.h"
//#include "community_cards.h"


using namespace std;

class Deck
{
public:
	static void initcards(std::vector<Card>& v);

	void fill(int type = 0);
	void empty();
	int count() const;
	int type;

	bool push(Card card);
	bool pop(Card &card);
	bool remove_d(Card &card);
	bool shuffle(int seed);
	int random_num(int start, int end, int seed);

	void get_hole_cards(HoleCards &holecards);
	int get_random_card();
	//void make_cards(HoleCards &holecards, int type);
	void debug();
	void makecards();
	void make_card();
	//void make_plane(HoleCards &holecards);
	void make_boom(HoleCards &holecards);
	void make_one_line(HoleCards &holecards);
	//void make_two_line(HoleCards &holecards);
	void make_three_line(HoleCards &holecards);
	void make_three_two(HoleCards &holecards);
	void make_double(HoleCards &holecards);
	void make_one(HoleCards &holecards);

	void create_duizi(HoleCards &holecards);
	void create_zhadan(HoleCards &holecards);
	void create_sandai(HoleCards &holecards);
	void create_shunzi(HoleCards &holecards);
	void create_liandui(HoleCards &holecards);
	void create_feiji(HoleCards &holecards);
	void create_one(HoleCards &holecards);
public:
	vector<Card> cards;
	vector<Card> goodcards;

private:
	int			_cardNo;
	/*void make_plane(HoleCards &holecards);
	void make_boom(HoleCards &holecards);
	void make_one_line(HoleCards &holecards);
	void make_two_line(HoleCards &holecards);
	void make_three_line(HoleCards &holecards);
	void make_three_two(HoleCards &holecards);
	void make_double(HoleCards &holecards);
	*/
	void make_plane();
	void make_boom();
	void make_one_line();
	void make_two_line();
	void make_three_line();
	void make_three_two();
	void make_double();
};

#endif /* _DECK_H_ */
