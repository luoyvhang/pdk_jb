#include <iostream>
#include "card.h"
#include "card_type.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_find.h"

using namespace std;

static char *card_type_str[] =
{
	(char*)"CARD_TYPE_ERROR",
	(char*)"CARD_TYPE_ONE", // single
	(char*)"CARD_TYPE_ONELINE", // single_straight(at least five cards);can start form '3' to 'A'(not '2')
	(char*)"CARD_TYPE_TWO", // double
	(char*)"CARD_TYPE_TWOLINE", // doble_straight(at least four cards);can start from '3' to 'A'
	(char*)"CARD_TYPE_THREE", // only if no more cards can take '3+0'
	(char*)"CARD_TYPE_THREELINE", // triple line(at least six cards);can start from '3' to 'A'
	(char*)"CARD_TYPE_THREEWITHONE", // only if no more cards can take '3+1'
	(char*)"CARD_TYPE_THREEWITHTWO", // must be '3+2';only if no more cards can take '3+1' or '3+0'
	//(char*)"CARD_TYPE_PLANELINE",//plane(at least eight cards);can start form '3' to 'A'(not '2');can be '3n+n' or '3n+2n'
	(char*)"CARD_TYPE_PLANEWITHONE",
	(char*)"CARD_TYPE_PLANEWITHWING",
	(char*)"CARD_TYPE_PLANWHITHLACK",
	(char*)"CARD_TYPE_FOURWITHONE",  // only if no more cards can take '4+1'
	(char*)"CARD_TYPE_FOURWITHTWO", // only if no more cards can take '4+2'
	(char*)"CARD_TYPE_FOURWITHTHREE", // '4+3'
	(char*)"CARD_TYPE_BOMB" // booms
};

CardAnalysis::CardAnalysis()
{
	
}

int CardAnalysis::analysis(vector<int> &cards,unsigned int handsno)
{

	type = CARD_TYPE_ERROR;

	/*20160713
	//ì??tóà??2??á3éá￠.
	if (handsno < 1 && handsno > 16)
	{
		return type;
	}
	*/
	if (handsno < 1 || handsno > 16) return type;	//20160713

	vector<Card> cards0;
	for (unsigned int i = 0; i < cards.size(); i++)
	{
		Card card(cards[i]);
		cards0.push_back(card);
	}

	CardStatistics card_stat;
	card_stat.statistics(cards0);

	int is_all;
	
	len = card_stat.len;
	if (len==handsno)
	{
		is_all=1;
	} 
	else
	{
		is_all=0;
	}

	/*20160713
	//ì??tóà??2??á3éá￠.
	if (len < 1 && len > 16)
	{
		return type;
	}
	*/
	if (len < 1 || len > 16) return type;	//20160713

	if (len == 1)
	{
		face = card_stat.card1[0].face;
		type = CARD_TYPE_ONE; // 3
		return type;
	}

	if (len == 2)
	{
		if (card_stat.line1.size() == 1
			&& card_stat.card2.size() == 2)
		{
			face = card_stat.card2[1].face;
			type = CARD_TYPE_TWO; // 33
			return type;
		}
	}

	if (len == 3)
	{
		if (card_stat.card3.size() == 3)
		{
			if (is_all)
			{
				face = card_stat.card3[2].face;
				type = CARD_TYPE_THREE; // 333
				return type;
			}					
		}
	}

	if (len == 4)
	{
		if (card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_BOMB; // 3333
			return type;
		}
		else if (card_stat.card3.size() == 3)
		{
			if (is_all)
			{
				face = card_stat.card3[2].face;
				type = CARD_TYPE_THREEWITHONE; // 333 4
				return type;
			} 
		}
	}

	if (len == 5)
	{
		if (card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHTWO; // 333 44
			return type;
		}
		//20160824?¨μˉ2?′???
		/*
		else if (card_stat.card4.size() == 4)
		{
			if (is_all)
			{
				face = card_stat.card4[3].face;
				type = CARD_TYPE_FOURWITHONE; // 3333 4
				return type;
			}
		}
		*/
	}
	//20160824?¨μˉ2?′???
	/*
	if (len == 6)
	{
		if (card_stat.card4.size() == 4)
		{
			if (is_all)
			{
				face = card_stat.card4[3].face;
				type = CARD_TYPE_FOURWITHTWO; // 3333 4 5
				return type;
			}
		}
	}

	if (len == 7)
	{
		if (card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTHREE; // 3333 4 5 6
			return type;
		}
	}
	*/
	//resolve 10 10 2 Q J 9 8 7 6 5 4 3  modifyed20140827
	if (card_stat.card1.size() == card_stat.line1.size()&&card_stat.card2.size() ==0&&card_stat.card3.size() ==0) {
		if (check_is_line(card_stat, 1)) {
			face = card_stat.card1[0].face;
			type = CARD_TYPE_ONELINE;
			return type;
		}
	}

	if (len == card_stat.card2.size()
		&& card_stat.card2.size() == card_stat.line2.size()) {
			if (check_is_line(card_stat, 2)) {
				face = card_stat.card2[0].face;
				type = CARD_TYPE_TWOLINE;
				return type;
			}
	}

	unsigned int left_card_len;
	if (card_stat.card3.size() != 0 && check_arr_is_line(card_stat.card3, 3))
	{
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card4.size();
		//resolve 5 5 5 4 4 4 12 12 A 10 6 3  modifyed20141008
		if((left_card_len * 3) > (card_stat.card3.size() * 2))
		{
			return type;
		}
		if (left_card_len == 0)
		{
			face = card_stat.card3[0].face;
			type = CARD_TYPE_THREELINE;
			return type;
		}

		if (left_card_len * 3 == card_stat.card3.size()) {
			//face = card_stat.card3[card_stat.card3.size() - 1].face;
			if (is_all)
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_PLANEWITHONE; // 333 444 555 666 9 9 9 9
				return type;
			}		
		}
		if (left_card_len * 3 == card_stat.card3.size() * 2)
		{
			//face = card_stat.card3[card_stat.card3.size() - 1].face;
			face = card_stat.card3[0].face;
			type = CARD_TYPE_PLANEWITHWING; // 333 444 99 99 //bug 444555666 888397
			return type;
		}
		if (left_card_len >0 && is_all)
		{
			//face = card_stat.card3[card_stat.card3.size() - 1].face;
			face = card_stat.card3[0].face;
			type = CARD_TYPE_PLANWHITHLACK; // 333 444 6 7 3
			return type;
		}
	}
	cout<<"card_stat.line3.size() "<<card_stat.line3.size()<<endl;
	if (card_stat.line3.size()>6)
	{
		vector<Card> straight_three_longest;
		unsigned int cnt = 0; // continues length
		unsigned int last_cnt = 0; //last time continues length
		unsigned int index = 0; // last time anti_continues length
		unsigned int i = 0; // cursor
		Card temp;
		int flag = 0; // 0 continues,1 anti_continues
		for (i = 0; i < card_stat.line3.size(); i += 3)
		{
			//printf("[%d][%d][%d][%d][%d][%d]\n", type, input[i].face, temp.face, cnt,last_cnt, index);
			if ((card_stat.line3[i].face - temp.face) != 1)
			{
				if (cnt > last_cnt)
				{
					index = i;
					last_cnt = cnt;
				}
				flag = 1;
				cnt = 0;
			}
			else
			{
				if (card_stat.line3[i].face!=15)
				{
					flag = 0;
				}
			}
			cnt += type;
			temp = card_stat.line3[i];
		}

		if (flag == 0)
		{
			if (cnt > last_cnt)
			{
				index = i;
				last_cnt = cnt;
			}
		}
		straight_three_longest.clear();
		for (unsigned int i = (index - last_cnt); i < index; i++)
		{
			straight_three_longest.push_back(card_stat.line3[i]);
		}
		unsigned int len1=straight_three_longest.size();
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card4.size();
		unsigned int len2 = left_card_len-len1;
		cout<<"len len1 len2"<<left_card_len<<" "<<len1<<" "<<len2<<endl;
		if ((len1*2)==(len2*3))
		{
			face = straight_three_longest[len1-1].face;
			type = CARD_TYPE_PLANWHITHLACK; // 333 444 6 7 3
			return type;
		}
		
		//bug 444 555 666 888 739  can not recognize
	}
	
	

	return type;
}

int CardAnalysis::analysis(CardStatistics &card_stat, int game_type, int fourdaithree, int threeAbomb)//analysize the card type
{
	type = CARD_TYPE_ERROR;
	
	len = card_stat.len;
	
	if (len == 0)
	{
		return type;
	}
	
	if (len == 1)
	{
		face = card_stat.card1[0].face;
		type = CARD_TYPE_ONE; // 3
		return type;
	}
	
	if (len == 2)
	{
		if (card_stat.line1.size() == 1
			&& card_stat.card2.size() == 2)
		{
			face = card_stat.card2[1].face;
			type = CARD_TYPE_TWO; // 33
			return type;
		}
	}
	
	if (len == 3)
	{
		if (1==threeAbomb)
		{
			if (card_stat.card3.size() == 3)
			{
				//如果是3个A，那么算炸
				if (card_stat.card3[2].face==14)
				{
					face = card_stat.card3[2].face;
					type = CARD_TYPE_BOMB; // AAA
					return type;
				}
				else
				{
					face = card_stat.card3[2].face;
					type = CARD_TYPE_THREE; // 333
					return type;
				}
			}
		}
		else
		{
			if (card_stat.card3.size() == 3)
			{
				face = card_stat.card3[2].face;
				type = CARD_TYPE_THREE; // 333
				return type;
			}
		}
	}
	
	if (len == 4)
	{
		if (card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_BOMB; // 3333
			return type;
		}
		else if (card_stat.card1.size() == 1
				 && card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHONE; // 333 4
			return type;
		}
		
	}
	
	if (len == 5)
	{
		if (card_stat.card2.size() == 2
			&& card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHTWO; // 333 44
			return type;
		}
		//20160824?¨μˉ2?′???
		
		else if (card_stat.card4.size() == 4
				&& card_stat.card1.size() == 1)
		{
			face = card_stat.card4[3].face;
		//	type = CARD_TYPE_FOURWITHONE; // 3333 4
			type = CARD_TYPE_THREEWITHTWO;
			return type;
		}
		
		else if (card_stat.card1.size() == 2
				&& card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHTWO; // 3333 4 5
			return type;
		}
		
		
	}
	//20160824?¨μˉ2?′???
    if (1==fourdaithree)
    {
        if (len == 6)
        {
            if (card_stat.card1.size() == 2
                && card_stat.card4.size() == 4)
            {
                face = card_stat.card4[3].face;
                type = CARD_TYPE_FOURWITHTWO; // 3333 4 5
                return type;
            }
            else if (card_stat.card2.size() == 2
                && card_stat.card4.size() == 4)
            {
                face = card_stat.card4[3].face;
                type = CARD_TYPE_FOURWITHTWO; // 3333 44
                return type;
            }

        }

        if (len == 7)
        {
            if (card_stat.card4.size() == 4
                && card_stat.card1.size() == 3)
            {
                face = card_stat.card4[3].face;
                type = CARD_TYPE_FOURWITHTHREE; // 3333 4 5 6
                return type;
            }
            else if (card_stat.card4.size() == 4
                && card_stat.card1.size() == 1
                && card_stat.card2.size() == 2)
            {
                face = card_stat.card4[3].face;
                type = CARD_TYPE_FOURWITHTHREE; // 3333 4 55
                return type;
            }
            else if (card_stat.card4.size() == 4
                && card_stat.card3.size() == 3)
            {
                face = card_stat.card4[3].face;
                type = CARD_TYPE_FOURWITHTHREE; // 3333 444
                return type;
            }
        }
    }
	
	if (card_stat.card1.size() == card_stat.line1.size()&&card_stat.card2.size() ==0&&card_stat.card3.size() ==0&&card_stat.card4.size() ==0) {
		if (check_is_line(card_stat, 1)) {
			face = card_stat.card1[0].face;
			type = CARD_TYPE_ONELINE;
			return type;
		}
	}
	
	if (len == card_stat.card2.size()
		&& card_stat.card2.size() == card_stat.line2.size()) {
		if (check_is_line(card_stat, 2)) {
			face = card_stat.card2[0].face;
			type = CARD_TYPE_TWOLINE;
			return type;
		}
	}
	
	if (len < 5)
	{
		return type;
	}
	
	unsigned int left_card_len;
	if (card_stat.card3.size() == card_stat.line3.size()
		&& card_stat.card4.size() == 0 && card_stat.card3.size() != 0)
	{
		if (check_is_line(card_stat, 3))
		{
			left_card_len = card_stat.card1.size() + card_stat.card2.size();
			if (left_card_len == 0)
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_THREELINE;
				return type;
				
			}
			else if (left_card_len * 3 == card_stat.card3.size())
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_PLANEWITHONE;//555 666 8 9
				return type;
			}
			//else if (card_stat.card1.size() == 0 && left_card_len * 3 == card_stat.card3.size() * 2)
			else if (left_card_len * 3 == card_stat.card3.size() * 2)//20140819
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_PLANEWITHWING;//555 666 88 99
				return type;
			}
		}
		
	}

	//20161024
	//2e?¨μˉDèòaí?2?line3.
	//?aà???ê??D??·é?ú??Díμ?.
	if (card_stat.card4.size() != 0) {
		int bob = card_stat.card4.size() / 4;
		for (int i = 0; i < bob; ++i) {
			int idx = i * 4;
			card_stat.card3.push_back(card_stat.card4[idx]);
			card_stat.card3.push_back(card_stat.card4[idx + 1]);
			card_stat.card3.push_back(card_stat.card4[idx + 2]);
			card_stat.card1.push_back(card_stat.card4[idx + 3]);
		}
		card_stat.card4.clear();
	}
	
	if (card_stat.card3.size() != 0 && check_arr_is_line(card_stat.card3, 3))
	{
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card4.size();
		if (left_card_len * 3 == card_stat.card3.size()) {
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			type = CARD_TYPE_PLANEWITHONE; // 333 444 555 666 9 9 9 9
			return type;
		}
		//else if (card_stat.card1.size() == 0 && left_card_len * 3 == card_stat.card3.size() * 2)
		else if (left_card_len * 3 == card_stat.card3.size() * 2)//20140819
		{
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			type = CARD_TYPE_PLANEWITHWING; // 333 444 99 99
			return type;
		}
		//in this situation:the last hands has not enough cards
		else if (left_card_len * 3 < card_stat.card3.size() * 2)
		{
			if (len % 5 == 0) {
				type = CARD_TYPE_PLANEWITHWING; // 333 444 9;333 444 8 99;333444 7 8 9
				//20161024 333 444 555 6 ,cardfind error.
				left_card_len = len / 5;
				int card3 = (int)card_stat.card3.size() / 3;
				for (int i = left_card_len; i < card3; ++i) {
					for (int n = 0; n < 3; ++n) {
						Card temp;
						temp = *card_stat.card3.begin();
						card_stat.card1.push_back(temp);
						card_stat.card3.erase(card_stat.card3.begin());
						for (std::vector<Card>::iterator iter = card_stat.line3.begin();
							iter != card_stat.line3.end(); ++iter) {
							if (temp.face == iter->face && temp.value == iter->value) {
								card_stat.line3.erase(iter);
								break;
							}
						}
					}
				}
			}
			else {
				type = CARD_TYPE_PLANWHITHLACK;
			}
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			return type;
		}
	}

	if (card_stat.line3.size() >= 6)//bug 444 555 666 888 739  can not recognize
	{
		//2016-08-11
		//bug 888 999 77 78
		if (check_arr_is_line(card_stat.line3, 3)) {
			int cout3 = card_stat.line3.size();
			//′?μ???2??ü3?1yèy??μ?êyá?.
			if (len % 5 == 0 && (int)len - cout3 < cout3) {
				face = card_stat.line3[0].face;
				type = CARD_TYPE_PLANEWITHWING;
				return type;
			}
		}
		vector<Card> straight_three_longest;
		unsigned int cnt = 0; // continues length
		unsigned int last_cnt = 0; //last time continues length
		unsigned int index = 0; // last time anti_continues length
		unsigned int i = 0; // cursor
		Card temp;
		int flag = 0; // 0 continues,1 anti_continues
		for (i = 0; i < card_stat.line3.size(); i += 3)
		{
			//printf("[3][%d][%d][%d][%d][%d]\n",  card_stat.line3[i].face, temp.face, cnt,last_cnt, index);
			if ((card_stat.line3[i].face - temp.face) != 1)
			{
				if (cnt > last_cnt)
				{
					index = i;
					last_cnt = cnt;
				}
				flag = 1;
				cnt = 0;
			}
			else
			{
				if (card_stat.line3[i].face!=15)
				{
					flag = 0;
				}
			}
			cnt += 3;
			temp = card_stat.line3[i];
		}

		if (flag == 0)
		{
			if (cnt > last_cnt)
			{
				index = i;
				last_cnt = cnt;
			}
		}
		straight_three_longest.clear();
		for (unsigned int i = (index - last_cnt); i < index; i++)
		{
			straight_three_longest.push_back(card_stat.line3[i]);
		}
		unsigned int len1=straight_three_longest.size();

		if (game_type == 0)
		{
			if (len1 >= 12 && len >= 16) {
				face = straight_three_longest[len1 - 1].face;
				type = CARD_TYPE_PLANEWITHWING; // 333 444 555 666 
				return type;
			}
		}
		else
		{
			if (len1 >= 12 && len >= 15) {
				face = straight_three_longest[len1 - 1].face;
				type = CARD_TYPE_PLANEWITHWING; // 333 444 555 666 
				return type;
			}
		}

        //?D???a????Dí 444 555 101010
        if (len1 >=6&&((len-len1)<(len1/3*2)))
        {
            face = straight_three_longest[len1 - 1].face;
            type = CARD_TYPE_PLANWHITHLACK;
            return type;
        }

		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card3.size() + card_stat.card4.size();
		unsigned int len2 = left_card_len-len1;
		if ((len1*2)==(len2*3)) 
		{
			//20161012  bug 333 777 888 4,   ê?é???óDèyá??3ê?′ò2??eμ?.
			//′ó3?μ????D,??cardfindòaó?μ?line3DT?y.
			for (std::vector<Card>::iterator it = card_stat.line3.begin();
				it != card_stat.line3.end();) {

				bool del = true;
				for (int i = (int)straight_three_longest.size(); i > 0; i -= 3) {
					Card& temp = straight_three_longest[i - 1];
					if (temp.face == it->face) {
						del = false;
						break;
					}
				}
				if (del) {
					Card& tmp = *it;
					card_stat.card1.push_back(tmp);

					for (std::vector<Card>::iterator iter = card_stat.card3.begin();
						iter != card_stat.card3.end();) {
						if (iter->face == tmp.face) iter = card_stat.card3.erase(iter);
						else ++iter;
					}

					it = card_stat.line3.erase(it);
				}
				else ++it;
			}
			/////////////////////////////
			face = straight_three_longest[len1-1].face;
			type = CARD_TYPE_PLANEWITHWING; // 333 444 6 7 3
			return type;
		}	
	}
	return type;
}

bool CardAnalysis::check_is_line(CardStatistics &card_stat, int line_type)
{
	if (line_type == 1)
	{
		return check_arr_is_line(card_stat.line1, line_type);
	}
	else if (line_type == 2)
	{
		return check_arr_is_line(card_stat.line2, line_type);
	}
	else if (line_type == 3)
	{
		return check_arr_is_line(card_stat.line3, line_type);
	}
	
	return false;
}

bool CardAnalysis::check_arr_is_line(std::vector<Card> &line, int line_type)
{
	return check_arr_is_line(line, line_type, 0, line.size());
}

bool CardAnalysis::check_arr_is_line(std::vector<Card> &line, int line_type, unsigned int begin, unsigned int end)
{
	int len = 1;
	Card card = line[begin];
	for (unsigned int i = (line_type + begin); i < end; i += line_type)
	{
		if ((card.face + 1) == line[i].face && line[i].face != 15) { // 2 is not straight (Line)
			len++;
			card = line[i];
		}
		else
		{
			return false;
		}
	}
	
	if (line_type == 1 && len > 4)
		return true;
	else if (line_type == 2 && len > 1)
		return true;
	else if (line_type == 3 && len > 1) 
		return true;
	
	return false;
}

bool CardAnalysis::compare(CardAnalysis &card_analysis) //  win : true
{
	/*
	printf("compare type[%s] len[%d] face[%d] vs type[%s] len[%d] face[%d]\n",
		   card_type_str[type], len, face, card_type_str[card_analysis.type], card_analysis.len, card_analysis.face);
	*/
	if (card_analysis.type == CARD_TYPE_ERROR)
	{
		return false;
	}
	
	if (type == card_analysis.type)
	{
		if (len == card_analysis.len
			&& face > card_analysis.face)
		{
			return true;
		}
	}
	else
	{
		if (type == CARD_TYPE_BOMB)
		{
			return true;
		}
	}
	return false;
}

void CardAnalysis::debug()
{
	cout << "type: " << card_type_str[type] << " face: " << face << endl;
}

void CardAnalysis::format(CardStatistics &stat, vector<int> &cur)
{
	int len;
	cur.clear();
	
	len = stat.card4.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card4[i].value);
	}
	
	len = stat.card3.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card3[i].value);
	}
	
	len = stat.card2.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card2[i].value);
	}
	
	len = stat.card1.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card1[i].value);
	}
}

void CardAnalysis::format(CardStatistics &stat, vector<Card> &cur)
{
	int len;
	cur.clear();
	
	len = stat.card4.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		Card card(stat.card4[i].value);
		cur.push_back(card);
	}
	
	len = stat.card3.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		Card card(stat.card3[i].value);
		cur.push_back(card);
	}
	
	len = stat.card2.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		Card card(stat.card2[i].value);
		cur.push_back(card);
	}
	
	len = stat.card1.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		Card card(stat.card1[i].value);
		cur.push_back(card);
	}
}

int CardAnalysis::isGreater(vector<int> &last, vector<int> &cur, int *card_type, int game_type)
{
	if (last.size() == 0)
	{
		return -1;
	}
	
	if (cur.size() == 0)
	{
		return -2;
	}
	
	vector<Card> cards0;//copy the last cards 
	for (unsigned int i = 0; i < last.size(); i++)
	{
		Card card(last[i]);
		cards0.push_back(card);
	}
	CardStatistics card_stat0;
	card_stat0.statistics(cards0);
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat0, game_type);
	if (card_ana0.type == 0)
	{
		return -1;
	}
	
	vector<Card> cards1;//copy current cards
	for (unsigned int i = 0; i < cur.size(); i++)
	{
		Card card(cur[i]);
		cards1.push_back(card);
	}
	CardStatistics card_stat1;
	card_stat1.statistics(cards1);
	CardAnalysis card_ana1;
	card_ana1.analysis(card_stat1, game_type);
	if (card_ana1.type == 0)
	{
		return -2;
	}
	
	*card_type = card_ana1.type;
	bool res = card_ana1.compare(card_ana0);
	if (res)
	{
		CardAnalysis::format(card_stat1, cur);
		return 1;
	}
	else
	{
		return 0;
	}
}

int CardAnalysis::isGreater(vector<Card> &last, vector<Card> &cur, int *card_type, int game_type)
{
	if (last.size() == 0)
	{
		return -1;
	}
	
	if (cur.size() == 0)
	{
		return -2;
	}
	CardStatistics card_stat0;
	card_stat0.statistics(last);
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat0, game_type);
	if (card_ana0.type == 0)
	{
		return -1;
	}
	
	CardStatistics card_stat1;
	card_stat1.statistics(cur);
	CardAnalysis card_ana1;
	card_ana1.analysis(card_stat1, game_type);
	if (card_ana1.type == 0)
	{
		return -2;
	}
	
	*card_type = card_ana1.type;
	bool res = card_ana1.compare(card_ana0);
	if (res)
	{
		CardAnalysis::format(card_stat1, cur);
		return 1;
	}
	else
	{
		return 0;
	}
}

int CardAnalysis::get_card_type(vector<int> &input, int game_type)
{
	if (input.size() == 0)
	{
		return 0;
	}
	

	vector<Card> cards;
	for (unsigned int i = 0; i < input.size(); i++)
	{
		Card card(input[i]);
		cards.push_back(card);
	}
	CardStatistics card_stat;
	card_stat.statistics(cards);
	CardAnalysis card_ana;
	card_ana.analysis(card_stat, game_type);
	CardAnalysis::format(card_stat, input);
	
	return card_ana.type;
}

int CardAnalysis::get_card_type(vector<Card> &input, int game_type)
{
	CardStatistics card_stat;
	card_stat.statistics(input);
	CardAnalysis card_ana;
	card_ana.analysis(card_stat, game_type);
	CardAnalysis::format(card_stat, input);
	
	return card_ana.type;
}

void CardAnalysis::test(int input[], int len)
{
	vector<Card> cards;
	for (int i = 0; i < len ; i++)
	{
		Card card(input[i]);
		cards.push_back(card);
	}
	CardStatistics card_stat;
	card_stat.statistics(cards);
	CardAnalysis card_ana;
	card_ana.analysis(card_stat,0);
	Card::dump_cards(cards);
	card_ana.debug();
}

void CardAnalysis::test(int input0[], int len0, int input1[], int len1)
{
	vector<Card> cards0;
	for (int i = 0; i < len0; i++)
	{
		Card card(input0[i]);
		cards0.push_back(card);
	}
	CardStatistics card_stat0;
	card_stat0.statistics(cards0);
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat0, 0);
	Card::dump_cards(cards0);
	card_ana0.debug();
	
	vector<Card> cards1;
	for (int i = 0; i < len1; i++)
	{
		Card card(input1[i]);
		cards1.push_back(card);
	}
	CardStatistics card_stat1;
	card_stat1.statistics(cards1);
	CardAnalysis card_ana1;
	card_ana1.analysis(card_stat1, 0);
	Card::dump_cards(cards1);
	card_ana1.debug();
	
	bool res = card_ana0.compare(card_ana1);
	cout << "res: " << res << endl;
}
