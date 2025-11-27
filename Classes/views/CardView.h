#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__

#include "cocos2d.h"
#include <vector>
#include <string>

class CardView : public cocos2d::Node
{
public:
    // cardId：卡牌编号
    // bgImage：卡牌背景图片
    // imageFiles：子图片
    // size：卡牌固定大小
    static CardView* create(int cardId, const std::string& bgImage,
        const std::vector<std::string>& imageFiles, const cocos2d::Size& size);

    bool init(int cardId, const std::string& bgImage,
        const std::vector<std::string>& imageFiles, const cocos2d::Size& size);

    void setClickCallback(const std::function<void(int)>& callback);

    // 新增：获取卡牌点数和花色
    int getCardValue() { return _cardValue; }
    std::string getCardSuit() { return _cardSuit; }

private:
    int _cardId;
    cocos2d::Size _cardSize;
    cocos2d::Sprite* _bgSprite = nullptr;
    std::vector<cocos2d::Sprite*> _sprites;
    std::function<void(int)> _clickCallback;

    // 新增：卡牌点数和花色
    int _cardValue = 0;
    std::string _cardSuit = "";
    // 新增：从文件名解析卡牌信息
    void extractCardInfo(const std::string& imageFile);
};

#endif
