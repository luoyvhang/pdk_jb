#ifndef _CARD_ANALYSIS_H_
#define _CARD_ANALYSIS_H_

#include <vector>
#include <string>

#include "card_statistics.h"

using namespace std;

class CardAnalysis
{
public:
	unsigned int len;
	int type;
	int face;

	//int line_count; // x=line_count;x连对，x连顺
	
	CardAnalysis();
	
	void clear();
    int analysis(CardStatistics &card_stat, int game_type, int fourdaithree = 0, int threeAbomb = 0);
	int analysis(vector<int> &cards,unsigned int handsno);
	bool check_is_line(CardStatistics &card_stat, int line_type);
	bool check_arr_is_line(std::vector<Card> &line, int line_type);
	bool check_arr_is_line(std::vector<Card> &line, int line_type, unsigned int begin, unsigned int end);
	
	bool compare(CardAnalysis &card_analysis);
	void debug();
	
	static void format(CardStatistics &stat, vector<int> &cur);
	static void format(CardStatistics &stat, vector<Card> &cur);
	static int isGreater(vector<int> &last, vector<int> &cur, int *card_type, int game_type);
	static int isGreater(vector<Card> &last, vector<Card> &cur, int *card_type, int game_type);
	static int get_card_type(vector<int> &input, int game_type);
	static int get_card_type(vector<Card> &input, int game_type);
	static void test(int input[], int len);
	static void test(int input0[], int len0, int input1[], int len1);
};


#endif /* _CARD_STATISTICS_H_ */
