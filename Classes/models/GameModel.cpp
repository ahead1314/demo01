#include "GameModel.h"
#include "views/CardView.h"  // 包含CardView头文件

GameModel* GameModel::_instance = nullptr;

GameModel* GameModel::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new GameModel();
    }
    return _instance;
}

GameModel::GameModel()
{
    // 初始化主牌区
    mainCardImages = {
        {"big_black_A.png","small_black_A.png","spade.png"},
        {"big_black_3.png","small_black_3.png","spade.png"},
        {"big_red_5.png","small_red_5.png","heart.png"},
        {"big_black_2.png","small_black_2.png","spade.png"},
        {"big_black_4.png","small_black_4.png","spade.png"},
        {"big_Black_6.png","small_Black_6.png","club.png"}
    };
    // 初始化手牌区
    handCardImages = {
        {"big_black_2.png","small_black_2.png","spade.png"},
        {"big_black_3.png","small_black_3.png","club.png"},
        {"big_red_4.png","small_red_4.png","diamond.png"},
    };


    undoStack.clear();
    mainCardsInfo.clear();
}
