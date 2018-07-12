#ifndef _CARD_H_
#define _CARD_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
/**
* suit  0 Diamonds  1 Clubs  2 Hearts  3 Spades
*
0x01, 0x11, 0x21, 0x31,		//A 14
0x02, 0x12, 0x22, 0x32,		//2 15
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
0x0D, 0x1D, 0x2D, 0x3D,		//K 13
0x0E,						//red joker
0x0F						//black joker
* @author luochuanting
*/

//suit  0  Diamond  1  Club   2 Heart     3 Spade
//remove jokers and three of '2'(keep the spade '2') and spade 'A',48 cards in all.
typedef enum {
	Three = 3,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Ten,
	Jack,
	Queen,
	King,
	Ace,
	Two,

	FirstFace = Three,
	LastFace = Two
} Face;

typedef enum {
	Diamonds = 0,
	Clubs, // 
	Hearts,
	Spades, //

	FirstSuit = Diamonds,
	LastSuit = Spades
} Suit;

class Card
{
	/*remove the pair of jokers;
	*remove three '2'(remain the Spades '2');
	*remove the Spades 'A';
	*48 cards in all
	*/
public:

	int face;
	int suit;

	int value;

	Card() { face = suit = value = 0; }
	Card(int val) 
	{
		value = val;

		face = value & 0xF; //  get the lower four bits -- face; amount to *ch - '0',higher efficiency
		suit = value >> 4; // get the higher four bits -- suit
		if (face < 3)
			face += 13; //14='A' 15='2'
	}

	bool operator <  (const Card &c) const { return (face < c.face); };
	bool operator >  (const Card &c) const { return (face > c.face); };
	bool operator == (const Card &c) const { return (face == c.face); };

	static int compare(const Card &a, const Card &b)
	{
		if (a.face > b.face)
		{
			return 1;
		}
		else if (a.face < b.face)
		{
			return -1;
		}
		else if (a.face == b.face)
		{
			if (a.suit > b.suit)
			{
				return 1;
			}
			else if (a.suit < b.suit)
			{
				return -1;
			}
		}

		return 0;
	}

	static bool lesser_callback(const Card &a, const Card &b)
	{
		if (Card::compare(a, b) == -1)
			return true;
		else
			return false;
	}

	static bool greater_callback(const Card &a, const Card &b)
	{
		if (Card::compare(a, b) == 1)
			return true;
		else
			return false;
	}

	static void sort_by_ascending(std::vector<Card> &v)
	{
		std::sort(v.begin(), v.end(), Card::lesser_callback);
	}

	static void sort_by_descending(std::vector<Card> &v)
	{
		std::sort(v.begin(), v.end(), Card::greater_callback);
	}
	
	static void dump_cards(const std::vector<Card>& v, const char* key = NULL)
	{

	}

	static void dump_cards(const std::map<int, Card>& v, const char* key = NULL) {

	}

	static void dump_cards(const std::vector<Card>& v, std::string& out) {
		if (!v.size()) return;
		out = "[";
		for (std::vector<Card>::const_iterator iter = v.begin();
			iter != v.end(); ++iter)
		{
			char buf[16];
			snprintf(buf, 16, "%d", iter->value);
			out.append(buf);
			out.push_back(',');
		}
		out.erase(out.begin() - 1);
		out.push_back(']');
	}
};

#endif /* _CARD_H_ */
