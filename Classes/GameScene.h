#pragma once

#include "cocos2d.h"
#include "models/GameModel.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void undoLastRemove(cocos2d::Ref* sender);

    CREATE_FUNC(GameScene);

private:
    GameModel* _model;       // 数据模型

    std::vector<CardView*> handCards;// 手牌区卡牌数组
    void initBackground();   // 声明
    void initMainCards();    // 声明
    void initUndoButton();   // 声明
    void initHandCards();   // 手牌区初始化
    void updateCardCallbacks();

    void replaceTopHandCard(int clickedIndex);//手牌区替换

    // 新增方法声明
    bool checkMatchWithTopHand(CardView* mainCard);
    void replaceTopWithMainCard(CardView* mainCard, const cocos2d::Vec2& mainCardPos);
    bool isBottomCardInColumn(CardView* card); // 检查是否是列中最下面的牌
    void refreshAllBottomCards(); // 新增：刷新所有底部牌状态
    void updateBottomCardInColumn(int column);
};
