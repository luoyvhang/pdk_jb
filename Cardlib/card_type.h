#ifndef _CARD_TYPE_H_
#define _CARD_TYPE_H_

enum CardType
{
	CARD_TYPE_ERROR = 0,			// error type
	CARD_TYPE_ONE = 1,			    // single
	CARD_TYPE_ONELINE = 2,		    // straight
	CARD_TYPE_TWO = 3,			    // double
	CARD_TYPE_TWOLINE = 4,		    // double straight
	CARD_TYPE_THREE = 5,			// triple	
	CARD_TYPE_THREELINE = 6,		// 三顺：点数相连的2个及以上的牌，可以从3连到A。
	CARD_TYPE_THREEWITHONE = 7,	    // 最后一手没多余牌的情况下可以出三带一	
	CARD_TYPE_THREEWITHTWO = 8,  	// 3同张必须带2张其他牌，带的牌不要求同点数
	CARD_TYPE_PLANEWITHONE = 9, 	// 飞机带羿，多个三带一,牌不够时出
	CARD_TYPE_PLANEWITHWING = 10,	// 飞机带翅，多个三带二
	CARD_TYPE_PLANWHITHLACK=11,	    //最后一手牌不够时允许三带一张或不带
	CARD_TYPE_FOURWITHONE = 12, 	// 4个带一（牌不够时可以少带，带1-2张）
	CARD_TYPE_FOURWITHTWO = 13, 	// 4个带二（牌不够时可以少带，带1-2张）
	CARD_TYPE_FOURWITHTHREE=14,     // 4张牌也可以带3张其他牌，这时不算炸弹
	CARD_TYPE_BOMB = 15,			// 4个		炸弹
};

#endif /* _CARD_TYPE_H_ */
