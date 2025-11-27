#pragma once
#include <vector>
#include <string>
#include "cocos2d.h"

// 前向声明
class CardView;

class GameModel
{
public:
    static GameModel* getInstance();

    // 操作类型枚举
    enum OperationType {
        HAND_CARD_SWAP,     // 手牌区交换  
        MATCH_REPLACE       // 匹配替换
    };

    // 操作记录结构
    struct OperationRecord {
        OperationType type;
        CardView* card1;
        CardView* card2;
        cocos2d::Vec2 pos1;
        cocos2d::Vec2 pos2;
        int index1;
        int index2;

        OperationRecord(OperationType t, CardView* c1, CardView* c2,
            cocos2d::Vec2 p1, cocos2d::Vec2 p2, int i1, int i2)
            : type(t), card1(c1), card2(c2), pos1(p1), pos2(p2), index1(i1), index2(i2) {}
    };


    // 主牌区卡牌图片
    std::vector<std::vector<std::string>> mainCardImages;
    // 手牌区卡牌图片
    std::vector<std::vector<std::string>> handCardImages;

    // Undo 操作栈
    std::vector<OperationRecord> undoStack;

    // 新增：主牌区卡牌信息
    struct MainCardInfo {
        CardView* card;
        cocos2d::Vec2 position;
        int column; // 0=左列, 1=右列
        int row;    // 0=最上面, 1=中间, 2=最下面
        bool isBottom; // 是否是该列最下面的牌
        bool isInMainArea; // 新增：标记卡片是否在主牌区
    };

    std::vector<MainCardInfo> mainCardsInfo;


private:
    GameModel();      // 构造函数私有
    static GameModel* _instance;
};
