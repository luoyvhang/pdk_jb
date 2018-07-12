#include "card.h"
#include "card_analysis.h"
#include "card_statistics.h"
#include "card_find.h"

CardFind::CardFind()
{
	m_ntipType=0;
}

void CardFind::clear()
{
	results.clear();
	light_cards_c.clear();
	light_cards_i.clear();
}

int CardFind::find(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &target_card_stat, int fourdaithree, int threeAbomb)
{
	clear();
	if (card_ana.type != CARD_TYPE_ERROR)
	{	
		if (card_ana.type == CARD_TYPE_ONE)
		{
			find_one(card_ana, card_stat, target_card_stat);
			find_two(card_ana, card_stat, target_card_stat);
			find_three(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_TWO)
		{
			find_two(card_ana, card_stat, target_card_stat);
			find_three(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREE)
		{
			find_three(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_ONELINE)
		{
			find_one_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_TWOLINE)
		{
			find_two_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREELINE)
		{
			find_three_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREEWITHONE)
		{
			find_three_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREEWITHTWO)
		{
			find_three_with_two(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_PLANEWITHONE)
		{
			find_plane_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_PLANEWITHWING)
		{
			find_plane_with_wing(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHONE)
		{
			find_four_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHTWO)
		{
			find_four_with_two(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHTHREE)
		{
			find_four_with_three(card_ana, card_stat, target_card_stat);
		}
		find_bomb(card_ana, card_stat, target_card_stat, fourdaithree, threeAbomb);
	}
	
	return 0;
}

void CardFind::find_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_ONE) 
	{
		for (unsigned int i = 0; i < my_card_stat.card1.size(); i++)
		{
			if (my_card_stat.card1[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card1[i]);
				light_cards_c.push_back(my_card_stat.card1[i]);
				results.push_back(cards);
				//return;
			}
		}
	}
}

void CardFind::find_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_ONE) 
	{
		for (unsigned int i = 0; i < my_card_stat.card2.size(); i += 1)
		{
			if (my_card_stat.card2[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card2[i]);
				light_cards_c.push_back(my_card_stat.card2[i]);
				//light_cards_c.push_back(my_card_stat.card2[i+1]);
				results.push_back(cards);
			}
		}
	}
	else if (card_ana.type == CARD_TYPE_TWO) 
	{
		for (unsigned int i = 0; i < my_card_stat.card2.size(); i += 2)
		{
			if (my_card_stat.card2[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card2[i]);
				cards.push_back(my_card_stat.card2[i + 1]);
				light_cards_c.push_back(my_card_stat.card2[i]);
				light_cards_c.push_back(my_card_stat.card2[i+1]);
				results.push_back(cards);
			}
		}
	}
}

void CardFind::find_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_ONE) 
	{
		for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
		{
			if (my_card_stat.card3[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card3[i]);
				light_cards_c.push_back(my_card_stat.card3[i]);
				//light_cards_c.push_back(my_card_stat.card3[i+1]);
				//light_cards_c.push_back(my_card_stat.card3[i+2]);
				results.push_back(cards);
			}
		}
	}
	else if (card_ana.type == CARD_TYPE_TWO) 
	{
		for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
		{
			if (my_card_stat.card3[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card3[i]);
				cards.push_back(my_card_stat.card3[i + 1]);
				light_cards_c.push_back(my_card_stat.card3[i]);
				light_cards_c.push_back(my_card_stat.card3[i+1]);
				//light_cards_c.push_back(my_card_stat.card3[i+2]);
				//Card::dump_cards(cards);
				results.push_back(cards);
			}
		}
	}
	else if (card_ana.type == CARD_TYPE_THREE) 
	{
		for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
		{
			if (my_card_stat.card3[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card3[i]);
				cards.push_back(my_card_stat.card3[i + 1]);
				cards.push_back(my_card_stat.card3[i + 2]);
				light_cards_c.push_back(my_card_stat.card3[i]);
				light_cards_c.push_back(my_card_stat.card3[i+1]);
				light_cards_c.push_back(my_card_stat.card3[i+2]);

				if (my_card_stat.card1.size()>1)
				{
					cards.push_back(my_card_stat.card1[0]);
					cards.push_back(my_card_stat.card1[1]);
					results.push_back(cards);
					return;
				}else if (my_card_stat.card2.size()>0)
				{
					cards.push_back(my_card_stat.card2[0]);
					cards.push_back(my_card_stat.card2[1]);
					results.push_back(cards);
					return;
				}else
				{
					results.push_back(cards);
					return;
				}			
			}
		}
	}
}

void CardFind::find_one_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line1.size() - card_stat.line1.size();
	for (int i = 0; i <= count; i++)
	{
		if (my_card_stat.line1[i].face > card_ana.face)
		{
			if (my_card_stat.line1[i].face!=15)
			{			
				int end = i + card_ana.len;
				if (card_ana.check_arr_is_line(my_card_stat.line1, 1, i, end))
				{
					vector<Card> cards;
					for (int j = i; j < end; j++)
					{
						cards.push_back(my_card_stat.line1[j]);	
						light_cards_c.push_back(my_card_stat.line1[j]);	
						for (unsigned int i = 0; i < my_card_stat.card2.size(); i += 2)
						{
							if (my_card_stat.card2[i].face == my_card_stat.line1[j].face)
							{
								light_cards_c.push_back(my_card_stat.card2[i]);
								light_cards_c.push_back(my_card_stat.card2[i+1]);
							}
						}
						for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
						{
							if (my_card_stat.card3[i].face == my_card_stat.line1[j].face)
							{
								light_cards_c.push_back(my_card_stat.card3[i]);
								light_cards_c.push_back(my_card_stat.card3[i+1]);
								light_cards_c.push_back(my_card_stat.card3[i+2]);
							}
						}
					}
					results.push_back(cards);
				//return;
				}	
			}
		}
	}
}

void CardFind::find_two_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line2.size() - card_stat.line2.size();
	for (int i = 0; i <= count; i += 2)
	{
		if (my_card_stat.line2[i].face > card_ana.face)
		{
			int end = i + card_ana.len;
			if (card_ana.check_arr_is_line(my_card_stat.line2, 2, i, end))
			{
				vector<Card> cards;
				for (int j = i; j < end; j++)
				{
					cards.push_back(my_card_stat.line2[j]);	
					light_cards_c.push_back(my_card_stat.line2[j]);	
					for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
					{
						if (my_card_stat.card3[i].face == my_card_stat.line2[j].face)
						{
							light_cards_c.push_back(my_card_stat.card3[i]);
							light_cards_c.push_back(my_card_stat.card3[i+1]);
							light_cards_c.push_back(my_card_stat.card3[i+2]);
						}
					}
				}
				results.push_back(cards);
			}	
		}
	}
}

void CardFind::find_three_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line3.size() - card_stat.line3.size();
	for (int i = 0; i <= count; i += 3)
	{
		if (my_card_stat.line3[i].face > card_ana.face)
		{
			int end = i + card_ana.len;
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end))
			{
				vector<Card> cards;
				for (int j = i; j < end; j++)
				{
					cards.push_back(my_card_stat.line3[j]);	
					light_cards_c.push_back(my_card_stat.line3[j]);	
				}
				results.push_back(cards);
			}	
		}
	}	
}

void CardFind::find_three_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 4)
	{
		return;
	}
	
	for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
	{
		if (my_card_stat.card3[i].face > card_ana.face)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card3[i]);
			cards.push_back(my_card_stat.card3[i + 1]);
			cards.push_back(my_card_stat.card3[i + 2]);
			light_cards_c.push_back(my_card_stat.card3[i]);
			light_cards_c.push_back(my_card_stat.card3[i + 1]);
			light_cards_c.push_back(my_card_stat.card3[i + 2]);
			if (my_card_stat.card1.size() > 0)
			{
				if (m_ntipType==0)
				{
					cards.push_back(my_card_stat.card1[0]);
				}
				
				//
			}
			else
			{
				for (unsigned int j = 0; j < my_card_stat.line1.size(); j++)
				{
					if (my_card_stat.line1[j].face != cards[0].face) {
						if (m_ntipType==0)
						{
							cards.push_back(my_card_stat.line1[j]);
						}
						
						//
						break;
					}
				}
			}
			results.push_back(cards);	
		}
	}
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}
	}
}

void CardFind::find_three_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 5)
	{
		return;
	}
	
	for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
	{
		if (my_card_stat.card3[i].face > card_ana.face)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card3[i]);
			cards.push_back(my_card_stat.card3[i + 1]);
			cards.push_back(my_card_stat.card3[i + 2]);
			if (m_ntipType==0)
			{
				if (my_card_stat.card1.size() > 1)
				{
					cards.push_back(my_card_stat.card1[0]);
					cards.push_back(my_card_stat.card1[1]);
				}else if (my_card_stat.card2.size() > 0)
				{
					cards.push_back(my_card_stat.card2[0]);
					cards.push_back(my_card_stat.card2[1]);
				}else
				{
					for (unsigned int j = 0; j < my_card_stat.line2.size(); j++)
					{
						if (my_card_stat.line2[j].face != cards[0].face) {
							cards.push_back(my_card_stat.line2[j]);
							cards.push_back(my_card_stat.line2[j + 1]);
							break;
						}
					}
				}
			}
			results.push_back(cards);			
		}
	}
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
}

void CardFind::find_plane_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{		
	int count = my_card_stat.line3.size() - card_stat.line3.size();
	for (int i = 0; i <= count; i += 3)
	{
		if (my_card_stat.line3[i].face > card_ana.face)
		{
			int end = i + card_stat.card3.size();
			
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end))
			{
				vector<Card> cards;
				for (int j = i; j < end; j++)
				{
					cards.push_back(my_card_stat.line3[j]);
					light_cards_c.push_back(my_card_stat.line3[j]);
				}
				if (m_ntipType)
				{
					results.push_back(cards);
				} 
				else
				{
					for (unsigned int j = 0; j < my_card_stat.card1.size(); j++)
					{
						//cards.push_back(my_card_stat.card1[j]);
						//if (cards.size() == card_ana.len)
						if (cards.size() == card_ana.len)
						{
							break;
						}
					}

					if (cards.size() == card_ana.len)
					{ 
						results.push_back(cards);
						continue;
					}

					int flag = 0;
					for (unsigned int j = 0; j < my_card_stat.line1.size(); j++)
					{
						flag = 0;
						for (unsigned int k = 0; k < cards.size(); k++)
						{
							if (cards[k].face == my_card_stat.line1[j].face)
							{
								flag = 1;
								break;
							}
						}

						if (flag == 1)
						{
							continue;
						}

						cards.push_back(my_card_stat.line1[j]);	
						if (cards.size() == card_ana.len)
						{
							break;
						}
					}

					if (cards.size() == card_ana.len)
					{
						results.push_back(cards);
						continue;
					}
				}
				
				
				// printf("aaa1[%u][%u]\n", cards.size(), card_ana.len);
				
			}
		}
	}
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
}

void CardFind::find_plane_with_wing(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line3.size() - card_stat.line3.size();
	for (int i = 0; i <= count; i += 3)
	{
		if (my_card_stat.line3[i].face > card_ana.face)
		{
			int end = i + card_stat.card3.size();
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end))
			{
				vector<Card> cards;
				for (int j = i; j < end; j++)
				{
					cards.push_back(my_card_stat.line3[j]);
				}
				if (m_ntipType)
				{
					results.push_back(cards);
				}
				else
				{
					//2016-08-11
					//修改带牌算法.

					int numCard1 = my_card_stat.card1.size();

					for (int j = 0; j + 1 < numCard1; j += 2) {
						cards.push_back(my_card_stat.card1[j]);
						cards.push_back(my_card_stat.card1[j + 1]);
						if (cards.size() == card_ana.len) break;
					}
					if (cards.size() == card_ana.len) {
						results.push_back(cards);
						continue;
					}

					for (int j = 0; j < (int)my_card_stat.card2.size(); j += 2)
					{
						cards.push_back(my_card_stat.card2[j]);
						cards.push_back(my_card_stat.card2[j + 1]);
						if (cards.size() == card_ana.len)
						{
							break;
						}
					}
					if (cards.size() == card_ana.len)
					{
						results.push_back(cards);
						continue;
					}

					//还需要的牌
					int needCards = card_ana.len - cards.size();
					int bomb = my_card_stat.card4.size();

					if (bomb > count)  bomb -= count;

					if (numCard1 % 2 + count + bomb >= needCards) {
						if (numCard1 % 2 == 1) {
							cards.push_back(my_card_stat.card1.back());
							--needCards;
							if (cards.size() == card_ana.len) {
								results.push_back(cards);
								continue;
							}
						}
						for (int j = 0; j < (int)my_card_stat.line3.size(); j += 3) {
							int flag = 0;
							for (int k = 0; k < (int)cards.size(); k++)
							{
								if (cards[k].face == my_card_stat.line3[j].face)
								{
									flag = 1;
									break;
								}
							}

							if (flag == 1)
							{
								continue;
							}
							cards.push_back(my_card_stat.line3[j]);
							if (--needCards <= 0) break;
							cards.push_back(my_card_stat.line3[j + 1]);
							if (--needCards <= 0) break;
							cards.push_back(my_card_stat.line3[j + 2]);
							if (--needCards <= 0) break;
						}
						if (cards.size() == card_ana.len)
						{
							results.push_back(cards);
							continue;
						}
					}


					if (numCard1 % 2 + (int)my_card_stat.card4.size() >= needCards) {
						if (numCard1 % 2 == 1) {
							cards.push_back(my_card_stat.card1.back());
							--needCards;
							if (cards.size() == card_ana.len) {
								results.push_back(cards);
								continue;
							}
						}
						for (int j = 0; j < (int)my_card_stat.card4.size(); ++j)
						{
							int flag = 0;
							for (int k = 0; k < (int)cards.size(); k++)
							{
								if (cards[k].value == my_card_stat.card4[j].value)
								{
									flag = 1;
									break;
								}
							}

							if (flag == 1)
							{
								continue;
							}

							cards.push_back(my_card_stat.card4[j]);
							if (--needCards <= 0) break;
						}
						if (cards.size() == card_ana.len)
						{
							results.push_back(cards);
							continue;
						}
					}
				}
				// printf("aaa1[%u][%u]\n", cards.size(), card_ana.len);		
			}
		}
	}
	//2016-08-11
	//服务端不需要这些
	/*
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
	*/
}

void CardFind::find_four_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 6)
	{
		return;
	}
	
	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4)
	{
		if (my_card_stat.card4[i].face > card_ana.face)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			if (m_ntipType)
			{
				results.push_back(cards);
			} 
			else
			{
				for (unsigned int j = 0; j < my_card_stat.card1.size(); j++)
				{
					cards.push_back(my_card_stat.card1[j]);
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{ 
					results.push_back(cards);
					continue;
				}

				int flag = 0;
				for (unsigned int j = 0; j < my_card_stat.line1.size(); j++)
				{
					flag = 0;
					for (unsigned int k = 0; k < cards.size(); k++)
					{
						if (cards[k].face == my_card_stat.line1[j].face)
						{
							flag = 1;
							break;
						}
					}

					if (flag == 1)
					{
						continue;
					}

					cards.push_back(my_card_stat.line1[j]);	
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{
					results.push_back(cards);
					continue;
				}
			}
			
			
			
		}
	}
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
}

void CardFind::find_four_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 8)
	{
		return;
	}
	
	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4)
	{
		if (my_card_stat.card4[i].face > card_ana.face)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			if (m_ntipType)
			{
				results.push_back(cards);
			} 
			else
			{
				for (unsigned int j = 0; j < my_card_stat.card2.size(); j += 2)
				{
					cards.push_back(my_card_stat.card2[j]);
					cards.push_back(my_card_stat.card2[j + 1]);
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{ 
					results.push_back(cards);
					continue;
				}

				int flag = 0;
				for (unsigned int j = 0; j < my_card_stat.line2.size(); j += 2)
				{
					flag = 0;
					for (unsigned int k = 0; k < cards.size(); k++)
					{
						if (cards[k].face == my_card_stat.line2[j].face)
						{
							flag = 1;
							break;
						}
					}

					if (flag == 1)
					{
						continue;
					}

					cards.push_back(my_card_stat.line2[j]);
					cards.push_back(my_card_stat.line2[j + 1]);
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{
					results.push_back(cards);
					continue;
				}
			}
			
			
			
		}
	}	
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
}

void CardFind::find_four_with_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 8)
	{
		return;
	}

	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4)
	{
		if (my_card_stat.card4[i].face > card_ana.face)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			if (m_ntipType)
			{
				results.push_back(cards);
			} 
			else
			{
				for (unsigned int j = 0; j < my_card_stat.card3.size(); j += 3)
				{
					cards.push_back(my_card_stat.card3[j]);
					cards.push_back(my_card_stat.card3[j + 1]);
					cards.push_back(my_card_stat.card3[j + 2]);
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{ 
					results.push_back(cards);
					continue;
				}

				int flag = 0;
				for (unsigned int j = 0; j < my_card_stat.line3.size(); j += 3)
				{
					flag = 0;
					for (unsigned int k = 0; k < cards.size(); k++)
					{
						if (cards[k].face == my_card_stat.line3[j].face)
						{
							flag = 1;
							break;
						}
					}

					if (flag == 1)
					{
						continue;
					}

					cards.push_back(my_card_stat.line3[j]);
					cards.push_back(my_card_stat.line3[j + 1]);
					cards.push_back(my_card_stat.line3[j + 2]);
					if (cards.size() == card_ana.len)
					{
						break;
					}
				}

				if (cards.size() == card_ana.len)
				{
					results.push_back(cards);
					continue;
				}
			}



		}
	}	
	if (results.size()>0)
	{
		if (my_card_stat.card1.size()>1)
		{
			for (unsigned int c = 0;c<my_card_stat.card1.size();c++)
			{
				light_cards_c.push_back(my_card_stat.card1[c]);
			}
		}
		if (my_card_stat.card2.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card2.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card2[a]);
			}
		}
		if (my_card_stat.card3.size()>0)
		{
			for (unsigned int a = 0;a<my_card_stat.card3.size();a++)
			{
				light_cards_c.push_back(my_card_stat.card3[a]);
			}
		}
		if (my_card_stat.card4.size()>0)
		{
			for (unsigned int b = 0;b<my_card_stat.card4.size();b++)
			{
				light_cards_c.push_back(my_card_stat.card4[b]);
			}
		}		
	}
}

void CardFind::find_bomb(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat, int fourdaithree, int threeAbomb)
{
	if (card_ana.type == CARD_TYPE_BOMB)
	{
		for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4)
		{
			if (my_card_stat.card4[i].face > card_ana.face)
			{
				vector<Card> cards;
				cards.push_back(my_card_stat.card4[i]);
				cards.push_back(my_card_stat.card4[i + 1]);
				cards.push_back(my_card_stat.card4[i + 2]);
				cards.push_back(my_card_stat.card4[i + 3]);
				light_cards_c.push_back(my_card_stat.card4[i]);
				light_cards_c.push_back(my_card_stat.card4[i + 1]);
				light_cards_c.push_back(my_card_stat.card4[i + 2]);
				light_cards_c.push_back(my_card_stat.card4[i + 3]);
				results.push_back(cards);
			}
		}

		if (1 == threeAbomb)
		{
			if (results.empty())
			{
				if (!my_card_stat.card3.empty())
				{
					for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
					{
						if (my_card_stat.card3[i].face==14)
						{
							vector<Card> cards;
							cards.push_back(my_card_stat.card3[i]);
							cards.push_back(my_card_stat.card3[i + 1]);
							cards.push_back(my_card_stat.card3[i + 2]);
							light_cards_c.push_back(my_card_stat.card3[i]);
							light_cards_c.push_back(my_card_stat.card3[i + 1]);
							light_cards_c.push_back(my_card_stat.card3[i + 2]);
							results.push_back(cards);
							return;
						}
					}
				}
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4)
		{
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			light_cards_c.push_back(my_card_stat.card4[i]);
			light_cards_c.push_back(my_card_stat.card4[i + 1]);
			light_cards_c.push_back(my_card_stat.card4[i + 2]);
			light_cards_c.push_back(my_card_stat.card4[i + 3]);
			results.push_back(cards);
		}

		if (1==threeAbomb)
		{
			if (results.empty())
			{
				if (!my_card_stat.card3.empty())
				{
					for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3)
					{
						if (my_card_stat.card3[i].face == 14)
						{
							vector<Card> cards;
							cards.push_back(my_card_stat.card3[i]);
							cards.push_back(my_card_stat.card3[i + 1]);
							cards.push_back(my_card_stat.card3[i + 2]);
							light_cards_c.push_back(my_card_stat.card3[i]);
							light_cards_c.push_back(my_card_stat.card3[i + 1]);
							light_cards_c.push_back(my_card_stat.card3[i + 2]);
							results.push_back(cards);
							return;
						}
					}
				}
			}
		}
	}	
}


int CardFind::find_straight(vector<int> &input, vector<int> &output) // find the longest
{
	if (input.size() == 0)
	{
		return -1;
	}
	
	vector<Card> input_format; // 
	vector<Card> longest_straight; // 
	for (unsigned int i = 0; i < input.size(); i++) // input format
	{
		Card card(input[i]);
		input_format.push_back(card);
	}
	
	CardStatistics input_statistics;
	input_statistics.statistics(input_format);
	CardFind ::get_straight (input_statistics, longest_straight); // find the longest
	
	output.clear();
	
	for (unsigned int i = 0; i < longest_straight.size(); i++)  //output
	{
		output.push_back(longest_straight[i].value);
	}

	return 0;
}

int CardFind::find_straight(vector<Card> &input, vector<Card> &output) // find the longest
{
	if (input.size() == 0)
	{
		return -1;
	}
	output.clear();
	CardStatistics input_statistics;
	input_statistics.statistics(input);
	CardFind::get_straight(input_statistics, output);
	return 0;
}

int CardFind::get_straight(CardStatistics &card_stat, vector<Card> &output) // find the longest
{
	vector<Card> straight_one_longest;
	vector<Card> straight_two_longest;
	vector<Card> straight_three_longest;

	get_longest_straight(card_stat.line1, 1, straight_one_longest); // 
	//get_longest_straight(card_stat.line2, 2, straight_two_longest); // 
	//get_longest_straight(card_stat.line3, 3, straight_three_longest); // 
	
	//Card::dump_cards(straight_one_longest, "One");
	//Card::dump_cards(straight_two_longest, "Two");
	//Card::dump_cards(straight_three_longest, "Three");
	
	output.clear();

	int cnt = get_max(straight_one_longest.size(), straight_two_longest.size(), straight_three_longest.size()); // the longest
	if (cnt == 1)
	{
		output = straight_one_longest;
		//Card::dump_cards(straight_one_longest, "One");
	}
	else if (cnt == 2)
	{
		output = straight_two_longest;
		//Card::dump_cards(straight_two_longest, "Two");
	}
	else if (cnt == 3)
	{
		output = straight_three_longest;
		//Card::dump_cards(straight_three_longest, "Three");
#if 0
		int one,cnt;
		one = straight_three_longest.size() / 3;
		//plane type,must be 3+2;if no more hole cards , can be 3+1 or 3+0
		
		cnt = card_stat.card2.size() / 2;
		//cout<<"one cnt "<<one<<" "<<cnt<<endl;
		if (cnt >= one)
		{
			for (int i = 0; i < cnt*2; i++)
			{
				if (i == one*2)
				{
					return 0;
				}
				output.push_back(card_stat.card2[i]);
			}
			return 0;
		}

		cnt = card_stat.card1.size();
		if (cnt >= one*2)
		{
			for (int i = 0; i < cnt; i++)
			{
				if (i == one*2)
				{
					return 0;
				}
				output.push_back(card_stat.card1[i]);
			}
			return 0;
		}



		/*vector<Card> straight_three_longest;
		unsigned int cnt = 0; // continuous length
		unsigned int last_cnt = 0; // last continuous length
		unsigned int index = 0; // last anti_continuous length
		unsigned int i = 0; // cursor
		Card temp;
		int flag = 0; // 0 continuous,1 anti_continuous
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
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card3.size() + card_stat.card4.size();
		unsigned int len2 = left_card_len-len1;*/
#endif		
	}
	
	return 0;
}

int CardFind::get_max(unsigned int a, unsigned int b, unsigned int c)
{
	if (c >= a && c >= b && c >= 6)
	{
		return 3;
	}
	
	if (b >= a && b >= c && b >= 6)
	{
		return 2;
	}
	
	if (a >= b && a >= c && a >= 5)
	{
		return 1;
	}
	
	return 0;
}

void CardFind::get_longest_straight(vector<Card> &input, int type, vector<Card> &output)
{
	unsigned int cnt = 0; // continuous length
	unsigned int last_cnt = 0; // last continuous length
	unsigned int index = 0; // last anti_continuous length
	unsigned int i = 0; // cursor
	Card temp;
	int flag = 0; // 0 continuous,1 anti_continuous
	for (i = 0; i < input.size(); i += type)
	{
		//printf("[%d][%d][%d][%d][%d][%d]\n", type, input[i].face, temp.face, cnt,last_cnt, index);
		if ((input[i].face - temp.face) != 1)
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
			if (input[i].face!=15)
			{
				flag = 0;
			}
		}
		cnt += type;
		temp = input[i];
	}
	
	if (flag == 0)
	{
		if (cnt > last_cnt)
		{
			index = i;
			last_cnt = cnt;
		}
	}
	output.clear();
	//printf("copy[%d][%u][%u]\n", type, index - last_cnt, index);
	for (unsigned int i = (index - last_cnt); i < index; i++)
	{
		output.push_back(input[i]);
	}
}

/*void CardFind::get_longest_straight_tip(vector<Card> &input, int type)
{
	unsigned int cnt = 0; // continuous length
	unsigned int last_cnt = 0; // last continuous length
	unsigned int index = 0; // last anti_continuous length
	unsigned int i = 0; // cursor
	Card temp;
	int flag = 0; // 0 continuous,1 anti_continuous
	for (i = 0; i < input.size(); i += type)
	{
		//printf("[%d][%d][%d][%d][%d][%d]\n", type, input[i].face, temp.face, cnt,last_cnt, index);
		if ((input[i].face - temp.face) != 1)
		{
			if (cnt > last_cnt)
			{
				index = i;
				last_cnt = cnt;
			}
			flag = 1;
			cnt = 0;
			int len=index-last_cnt;
			if (len>4)
			{
				vector<Card> temp;
				for (unsigned int i = (index - last_cnt); i < index; i++)
				{
					temp.push_back(input[i]);
				}
				results.push_back(temp);
			}
			
		}
		else
		{
			if (input[i].face!=15)
			{
				flag = 0;
			}
		}
		cnt += type;
		temp = input[i];
	}

	if (flag == 0)
	{
		if (cnt > last_cnt)
		{
			index = i;
			last_cnt = cnt;
		}
	}
	//printf("copy[%d][%u][%u]\n", type, index - last_cnt, index);
	for (unsigned int i = (index - last_cnt); i < index; i++)
	{
		output.push_back(input[i]);
	}
}*/


void CardFind::debug()
{
	for (unsigned int i = 0; i < results.size(); i++)
	{
		Card::dump_cards(results[i], "tip");
	}
}


int CardFind::tip(vector<int> &cur, int game_type)
{
	if (cur.size() == 0)
	{
		return -1;
	}
	clear();

	vector<Card> cards1;
	for (unsigned int i = 0; i < cur.size(); i++)
	{
		Card card(cur[i]);
		cards1.push_back(card);
	}
	CardStatistics card_stat1;
	card_stat1.statistics(cards1);
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat1, game_type);

	//find(card_ana0, card_stat0, card_stat1);

	//CARD_TYPE_ONE
	for (unsigned int i = 0; i < card_stat1.card1.size(); i++)
	{
			vector<Card> cards;
			cards.push_back(card_stat1.card1[i]);
			results.push_back(cards);
			//return;
	}

	//CARD_TYPE_ONE
	for (unsigned int i = 0; i < card_stat1.card2.size(); i += 2)
	{
			vector<Card> cards;
			cards.push_back(card_stat1.card2[i]);
			cards.push_back(card_stat1.card2[i + 1]);
			results.push_back(cards);
	}

	//CARD_TYPE_THREE

	int count;
	//find_one_line
	count = card_stat1.line1.size();
	for (int i = 0; i < count; i++)
	{
			if (card_stat1.line1[i].face!=15)
			{
				int end = i +5;
				if (card_ana0.check_arr_is_line(card_stat1.line1, 1, i, end))
				{
					vector<Card> cards;
					for (int j = i; j < end; j++)
					{
						cards.push_back(card_stat1.line1[j]);	
					}
					results.push_back(cards);
				}	
			}
	}

	//find_two_line
	
	//find_three_with_two
	if (card_stat1.len >= 5)
	{
		for (unsigned int i = 0; i < card_stat1.card3.size(); i += 3)
		{
				vector<Card> cards;
				cards.push_back(card_stat1.card3[i]);
				cards.push_back(card_stat1.card3[i + 1]);
				cards.push_back(card_stat1.card3[i + 2]);
				if (card_stat1.card2.size() > 0)
				{
					cards.push_back(card_stat1.card2[0]);
					cards.push_back(card_stat1.card2[1]);
				}else if (card_stat1.card1.size() > 1)
				{
					cards.push_back(card_stat1.card1[0]);
					cards.push_back(card_stat1.card1[1]);
				}
				results.push_back(cards);
		}
	}
	return 0;
}

int CardFind::light_cards(vector<int> &last, vector<int> &hands, int game_type)
{
	if (last.size() == 0)
	{
		return -1;
	}

	if (hands.size() == 0)
	{
		return -2;
	}

	clear();

	vector<Card> last_cards;
	for (unsigned int i = 0; i < last.size(); i++)
	{
		Card card(last[i]);
		last_cards.push_back(card);
	}
	CardStatistics last_stat;
	last_stat.statistics(last_cards);
	CardAnalysis last_ana;
	last_ana.analysis(last_stat, game_type);
	if (last_ana.type == 0)
	{
		return -1;
	}

	vector<Card> hands_cards;
	for (unsigned int i = 0; i < hands.size(); i++)
	{
		Card card(hands[i]);
		hands_cards.push_back(card);
	}
	CardStatistics hands_stat;
	hands_stat.statistics(hands_cards);

	find(last_ana, last_stat, hands_stat);

	if (results.size()<=0)
	{
		return 0;
	} 
	else
	{
		int size_s = results.size();
		Card::sort_by_ascending(light_cards_c);
		for (unsigned int i=0;i<light_cards_c.size();i++)
		{
			Card card_temp=light_cards_c[i];
			light_cards_i.push_back(card_temp.value);
		}
		

		if (light_cards_i.size()>0)
		{
			light_cards_i.erase(unique(light_cards_i.begin(), light_cards_i.end()),light_cards_i.end()); 
		}
		
		return size_s;
	}
	

	//return 0;
}

int CardFind::tip(vector<int> &last, vector<int> &cur, int game_type)
{
	if (last.size() == 0)
	{
		return -1;
	}
	
	if (cur.size() == 0)
	{
		return -2;
	}
	
	clear();
	
	vector<Card> cards0;
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
	
	vector<Card> cards1;
	for (unsigned int i = 0; i < cur.size(); i++)
	{
		Card card(cur[i]);
		cards1.push_back(card);
	}
	CardStatistics card_stat1;
	card_stat1.statistics(cards1);
	
	find(card_ana0, card_stat0, card_stat1);
	
	return 0;
}

int CardFind::tip(vector<Card> &last, vector<Card> &cur, int game_type, int fourdaithree, int threeAbomb)
{
	if (last.size() == 0)
	{
		return -1;
	}
	
	if (cur.size() == 0)
	{
		return -2;
	}
	
	clear();
	
	CardStatistics card_stat0;
	card_stat0.statistics(last);
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat0, game_type, fourdaithree, threeAbomb);
	if (card_ana0.type == 0)
	{
		return -1;
	}
	
	CardStatistics card_stat1;
	card_stat1.statistics(cur);
	
	find(card_ana0, card_stat0, card_stat1);
	
	return 0;
}


int CardFind:: tip_up(vector<int> &select,vector<int> &hands)//tip up the line
{
	if (select.size() <= 1)
	{
		return 0;
	}
	if (hands.size() <= 1)
	{
		return 0;
	}
	vector<Card> hands_c,select_c; 
	for (unsigned int i = 0; i < hands.size(); i++)
	{
		Card card(hands[i]);
		hands_c.push_back(card);
	}
	for (unsigned int i = 0; i < select.size(); i++)
	{
		Card card(select[i]);
		select_c.push_back(card);
	}
	///*step1:find longest straight
	CardStatistics hands_statistics;
	hands_statistics.statistics(hands_c);
	vector<Card> straight_one_longest;
	get_longest_straight(hands_statistics.line1, 1, straight_one_longest); 
	Card::sort_by_ascending(straight_one_longest);
	if (straight_one_longest.size()<5)
	{
		return 0;
	}
	/*
	for (unsigned int i=0;i<straight_one_longest.size();i++)
	{
		cout<<straight_one_longest[i].face<<" ";
	}
	cout<<endl;
	*/

	//step2:mark the cursor of selected cards
	int search_group[16];//initialize 0  the position of selected card is evaluated 1
	for (int i =0;i<16;i++)
	{
			search_group[i]=0;
	}
	for (unsigned int select_cur=0;select_cur<select_c.size();select_cur++)
	{
		int select_face=select_c[select_cur].face;
		for (unsigned int straight_one_cur=0;straight_one_cur<straight_one_longest.size();straight_one_cur++)
		{
			int temp=straight_one_longest[straight_one_cur].face;
			if (select_face==temp)
			{
				search_group[select_face]++;
			}
		}
	}
	
	///*step3:decide output type
	for (unsigned int select_cur=0;select_cur<select_c.size();select_cur++)
	{
		int select_face=select_c[select_cur].face;
		if (search_group[select_face]!=1)//return 0 when has select the card which is not on the line
		{
			return 0;
		}
	}
	
	tipup_i.clear();
	///*step4 choose the right length
	int length_c=select_c.size()-1;
	Card::sort_by_ascending(select_c);
	int first_v=select_c[0].face;
	int end_v=select_c[length_c].face;
	unsigned int len_sel=end_v-first_v+1;
	unsigned int location_f=0;
//	unsigned int end_f=0;
	int flag=0;
	
	for (unsigned int i =0;i<straight_one_longest.size();i++)
	{
		//to find the head and the tail
		if (first_v==straight_one_longest[i].face)
		{
			location_f=i;
			//cout<<"location_f "<<location_f<<endl;
		}
		if (end_v==straight_one_longest[i].face)
		{
//			end_f=i;
			//cout<<"end_f "<<end_f<<endl;
		}
	}
	if (len_sel<=5)
	{
		unsigned int len = straight_one_longest.size();
		unsigned int a = len - location_f;
		unsigned int b;
		if (a<5)
		{
			b = 5-a;
		} 
		else
		{
			b = 0;
		}
		
//		cout<<"len a b "<<len<<" "<<a<<" "<<b<<endl;
		for (int k=0;k<5;k++)
		{
			if (location_f+k<len)
			{
				int temp = straight_one_longest[k+location_f].face;
//				cout<<"len< temp:"<<temp<<endl;
				flag=0;
				for (unsigned int l = 0;l < select_c.size();l++)
				{			
					if (select_c[l].face==temp)//the selected cards can not be put in the results
					{				
						flag=1;
						break;
					} 
				}
				if (flag==0)
				{
					//tipup_i.push_back(temp);
					tipup_i.push_back(straight_one_longest[k+location_f].value);
				}
				
			}	
		}	
		if (tipup_i.size()<5 && b>0)
		{
			for (unsigned int i =1;i<=b;i++)
			{
				int temp = straight_one_longest[location_f-i].face;
				flag=0;
				for (unsigned int l = 0;l < select_c.size();l++)
				{			
					if (select_c[l].face==temp)//the selected cards can not be put in the results
					{				
						flag=1;
						break;
					} 
				}
				if (flag==0)
				{
					//tipup_i.push_back(temp);
					tipup_i.push_back(straight_one_longest[location_f-i].value);
				}
				
			}
		}
	} 
	else
	{
		for (unsigned int k=location_f;k<len_sel;k++)
		{
			int temp = straight_one_longest[k+location_f].face;
			flag=0;
			for (unsigned int l = 0;l < select_c.size();l++)
			{
				if (select_c[l].face==temp)
				{
					flag=1;
					break;
				}
			}
			if (flag==0)
			{
				tipup_i.push_back(straight_one_longest[k+location_f].value);
			}
			
		}
	}

	/*
	for (unsigned int i =0;i<tipup_i.size();i++)
	{
		cout<<tipup_i[i]<<endl;
	}
	*/
	
	return 1;
}

/*int CardFind:: tip_up(vector<int> &select,vector<int> &hands)
{
	if (select.size() <= 1)
	{
		return 0;
	}
	if (hands.size() <= 1)
	{
		return 0;
	}
	vector<Card> hands_c,select_c; 
	for (unsigned int i = 0; i < hands.size(); i++)
	{
		Card card(hands[i]);
		hands_c.push_back(card);
	}
	for (unsigned int i = 0; i < select.size(); i++)
	{
		Card card(select[i]);
		select_c.push_back(card);
	}
	//step1:find longest straight
	CardStatistics hands_statistics;
	hands_statistics.statistics(hands_c);

	vector<Card> straight_one_longest;
	vector<Card> straight_two_longest;
	vector<Card> straight_three_longest;

	get_longest_straight(hands_statistics.line1, 1, straight_one_longest); 
	get_longest_straight(hands_statistics.line2, 2, straight_two_longest); 
	get_longest_straight(hands_statistics.line3, 3, straight_three_longest); 

	Card::sort_by_ascending(straight_one_longest);
	Card::sort_by_ascending(straight_two_longest);
	Card::sort_by_ascending(straight_three_longest);
	for (unsigned int i=0;i<straight_one_longest.size();i++)
	{
		cout<<straight_one_longest[i].face<<" ";
	}
	cout<<endl;
	for (unsigned int i=0;i<straight_two_longest.size();i++)
	{
		cout<<straight_two_longest[i].face<<" ";
	}
	cout<<endl;for (unsigned int i=0;i<straight_three_longest.size();i++)
	{
		cout<<straight_three_longest[i].face<<" ";
	}
	cout<<endl;
	//step2:mark the cursor of selected cards
	int search_group[16][3];//initialize 0 the selected card is evaluated 1
	int is_this[3]={1,1,1};//initialize 1 ;identify the type is line or double line
	for (int i =0;i<16;i++)
	{
		for (int k=0;k<3;k++)
		{
			search_group[i][k]=0;
		}
	}
	for (unsigned int select_cur=0;select_cur<select_c.size();select_cur++)
	{
		int select_face=select_c[select_cur].face;
		for (unsigned int straight_one_cur=0;straight_one_cur<straight_one_longest.size();straight_one_cur++)
		{
			int temp=straight_one_longest[straight_one_cur].face;
			if (select_face==temp)
			{
				search_group[select_face][0]++;
			}
		}
		for (unsigned int straight_two_cur=0;straight_two_cur<straight_two_longest.size();straight_two_cur++)
		{
			int temp=straight_two_longest[straight_two_cur].face;
			if (select_face==temp)
			{
				search_group[select_face][1]++;
			}
		}
		for (unsigned int straight_three_cur=0;straight_three_cur<straight_three_longest.size();straight_three_cur++)
		{
			int temp=straight_three_longest[straight_three_cur].face;
			if (select_face==temp)
			{
				search_group[select_face][2]++;
			}
		}
	}
	
	//step3:decide output type
	for (int i =0;i<3;i++)
	{
		for (unsigned int select_cur=0;select_cur<select_c.size();select_cur++)
		{
			int select_face=select_c[select_cur].face;
			if (search_group[select_face][i]==0)//
			{
				is_this[i]=0;
			}
			if (i==0)//line
			{
				if (search_group[select_face][i]>1)// is not line
				{
					is_this[i]=0;
				}
			}
			if (i==1)//doble line
			{
				if (search_group[select_face][i]>4)//
				{
					is_this[i]=0;
				}
			}
		}
	}
	
	int ouput_type=-1;
	int is_only=0;
	for (int i=0;i<3;i++)
	{
		if (is_this[i]==1)
		{
			is_only++;/if the only one group
			ouput_type=i;
		}
	}
	cout<<"output type:"<<ouput_type<<endl;





	if (is_only==1)
	{
		tipup_i.clear();
		ouput_type++;//1 is line 2 is double line 3 is plane
		//step4 choose the right length
		int length_c=select_c.size()-1;
		Card::sort_by_ascending(select_c);
		int first_v=select_c[0].face;
		int end_v=select_c[length_c].face;
		unsigned int len_sel=end_v-first_v+1;
		unsigned int location_f=0;
		int flag=0;
		if (ouput_type==1)//len 5 or more
		{
			for (unsigned int i =0;i<straight_one_longest.size();i++)
			{
				if (first_v==straight_one_longest[i].face)//to find the position of the smallest card in the line
				{
					location_f=i;
					break;
				}
			}
			if (len_sel<=5)
			{
				for (int k=0;k<5;k++)
				{		
					int temp = straight_one_longest[k+location_f].face;
					cout<<"len< temp:"<<temp<<endl;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						
						if (select_c[l].face==temp)//the selected cards can not be put in the results
						{
							
							flag=1;
							break;
						} 
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}	
			} 
			else
			{
				for (unsigned int k=location_f;k<len_sel;k++)
				{
					int temp = straight_one_longest[k+location_f].face;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						if (select_c[l].face==temp)
						{
							flag=1;
							break;
						}
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}
			}	
		}else if(ouput_type==2){// len 2 or more
			for (unsigned int i =0;i<straight_two_longest.size();i++)
			{
				if (first_v==straight_two_longest[i].face)
				{
					location_f=i;
					break;
				}
			}
			if (len_sel<=2)
			{
				for (int k=0;k<4;k++)
				{
					int temp = straight_two_longest[k+location_f].face;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						if (select_c[l].face==temp)
						{
							flag=1;
							break;
						}
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}
			} 
			else
			{
				for (unsigned int k=location_f;k<(len_sel*2);k++)
				{
					int temp = straight_two_longest[k+location_f].face;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						if (select_c[l].face==temp)
						{
							flag=1;
							break;
						} 
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}
			}
		}else if(ouput_type==3){// len 2 or more
			for (unsigned int i =0;i<straight_three_longest.size();i++)
			{
				if (first_v==straight_three_longest[i].face)
				{
					location_f=i;
					break;
				}
			}
			if (len_sel<=2)
			{
				for (int k=0;k<6;k++)
				{
					int temp = straight_three_longest[k+location_f].face;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						if (select_c[l].face==temp)
						{
							flag=1;
							break;
						} 
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}
			} 
			else
			{
				for (unsigned int k=location_f;k<(len_sel*3);k++)
				{
					int temp = straight_three_longest[k+location_f].face;
					flag=0;
					for (unsigned int l = 0;l < select_c.size();l++)
					{
						if (select_c[l].face==temp)
						{
							flag=1;
							break;
						} 
					}
					if (flag==0)
					{
						tipup_i.push_back(temp);
					}
				}
			}
		}






		for (unsigned int i =0;i<tipup_i.size();i++)
		{
			cout<<tipup_i[i]<<endl;
		}
		
		return 1;
	}
	return 0;
}
*/




int CardFind::tip_up(vector<int> &select,vector<vector<Card> > &results, vector<int> &lights)
{
	std::map<int, int> lights_m;
	for (unsigned int i = 0;i<lights.size();i++)
	{
		lights_m.insert(map<int,int>::value_type(lights[i],0));
	}
	for (unsigned int ii=0;ii<select.size();ii++)
	{
		std::map<int,int>::iterator it= lights_m.find(select[ii]);
		if(it == lights_m.end()) {
			return 0;
		}
	}
	
	std::map<int, int> search_m;
	int only_card=0;
	for (unsigned int i = 0;i<select.size();i++)
	{
		search_m.insert(map<int,int>::value_type(select[i],0));
	}
	
	for (unsigned int i=0;i<results.size();i++)
	{
		vector<Card> results_son=results[i];
		for (unsigned int k=0;k<results_son.size();k++)
		{
			int temp=results_son[k].value;
			std::map<int,int>::iterator it= search_m.find(temp);
			if(it == search_m.end()) {
				//return 0;
			}
			else {
				search_m[temp]++;
			}
		}
	}

	std::map<int,int>::iterator it;
	int count=0;
	for (it = search_m.begin(); it != search_m.end(); it++)
	{
		if (it->second==1)
		{
			only_card=it->first;
			count++;
			//break;
		}	
	}
	if (count>1)
	{
		return 0;
	}
	
	if (only_card>0)
	{
		for (unsigned int i=0;i<results.size();i++)
		{
			vector<Card> results_son=results[i];
			int is_this=0;
			for (unsigned int k=0;k<results_son.size();k++)
			{
				int temp=results_son[k].value;
				if (temp==only_card)
				{
					is_this=1;
					break;
				}
			}
			if (is_this)
			{
				for (unsigned int k=0;k<results_son.size();k++)
				{
					int temp=results_son[k].value;
					tipup_i.push_back(temp);
				}
				if (select.size()>=tipup_i.size())//resolve select '78' up '88'
				{
					return 0;
				} 
				else
				{
					return 1;
				}
			}
			
		}
		
	}
	return 0;
}

void CardFind::test(int input0[], int len0, int input1[], int len1)
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
	
	CardFind card_find;
	card_find.find(card_ana0, card_stat0, card_stat1);
	card_find.debug();
	
}

void CardFind::test(int input[], int len)
{
	vector<Card> cards;
	for (int i = 0; i < len; i++)
	{
		Card card(input[i]);
		cards.push_back(card);	
	}
	CardStatistics card_stat;
	card_stat.statistics(cards);
	CardFind::get_straight(card_stat, cards);
	Card::dump_cards(cards, "Longest");
}
