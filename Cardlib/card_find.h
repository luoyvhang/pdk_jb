#ifndef _CARD_FIND_H_
#define _CARD_FIND_H_

#include <vector>
#include <string>

#include "card.h"
#include "card_type.h"
#include "card_analysis.h"
#include "card_statistics.h"


using namespace std;

class CardFind
{
public:
	vector<vector<Card> > results;
	vector<vector<int> > results_i;
	vector<int> light_cards_i;
	vector<Card> light_cards_c;
	vector<int> tipup_i;
	
	CardFind();
	
	void clear();
	int find(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat, int fourdaithree = 0, int threeAbomb = 0);
	
	void find_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_one_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_two_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_plane_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_plane_with_wing(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_bomb(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat, int fourdaithree = 0, int threeAbomb = 0);
	
	void debug();
	int tip(vector<int> &cur, int game_type);
	int tip(vector<int> &last, vector<int> &cur, int game_type);
	int tip(vector<Card> &last, vector<Card> &cur, int game_type, int fourdaithree = 0, int threeAbomb = 0);
	int tip_up(vector<int> &select,vector<int> &hands);
	int tip_up(vector<int> &select,vector<vector<Card> > &results, vector<int> &lights);
	int light_cards(vector<int> &last, vector<int> &hands, int game_type);
	static int find_straight(vector<int> &input, vector<int> &output);
	static int find_straight(vector<Card> &input, vector<Card> &output);
	static int get_straight(CardStatistics &card_stat, vector<Card> &output);
	static int get_max(unsigned int a, unsigned int b, unsigned int c);
	static void get_longest_straight(vector<Card> &input, int type, vector<Card> &output);
	//static void get_longest_straight_tip(vector<Card> &input, int type);
	static void test(int input0[], int len0, int input1[], int len1);
	static void test(int input[], int len);
	int m_ntipType;
private:

};


#endif /* _CARD_FIND_H_ */
