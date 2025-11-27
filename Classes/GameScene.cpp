#include "GameScene.h"
#include "views/CardView.h"
#include "models/GameModel.h"
USING_NS_CC;
Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init()) return false;
    // 绑定模型
    _model = GameModel::getInstance();

    initBackground();
    initMainCards();//主牌区
    initHandCards();     // 手牌区
    initUndoButton();
    return true;
}

#pragma mark - 初始化背景（主牌区 + 手牌区）
void GameScene::initBackground()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    float totalHeight = visibleSize.height;
    float mainHeight = totalHeight * 1500.0f / 2080.0f;
    float handHeight = totalHeight * 580.0f / 2080.0f;

    // 主牌区（蓝色）
    auto mainLayer = LayerColor::create(
        Color4B(200, 200, 255, 255),
        visibleSize.width,
        mainHeight + origin.y
    );
    mainLayer->setPosition(Vec2(0, handHeight));
    this->addChild(mainLayer);

    // 手牌区（红色）
    auto handLayer = LayerColor::create(
        Color4B(255, 200, 200, 255),
        visibleSize.width,
        handHeight + origin.y
    );
    handLayer->setPosition(Vec2(0, 0));
    this->addChild(handLayer);
}
#pragma mark - 初始化主牌区（左边 3 张 + 右边 3 张）
void GameScene::initMainCards()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float totalHeight = visibleSize.height;

    cocos2d::Size cardSize(200, 300);
    int totalCards = (int)_model->mainCardImages.size();

    float offX = 50;
    float offY = 100;

    // 清空主牌信息
    _model->mainCardsInfo.clear();
    //-----------------------------
    // 左侧卡牌（0,1,2）
    //-----------------------------
    for (int i = 0; i < totalCards / 2; i++)
    {
        auto card = CardView::create(
            i,
            "card_general.png",
            _model->mainCardImages[i],
            cardSize
        );

        //保留原来的位置逻辑
        Vec2 pos = Vec2(100 + i * offX, totalHeight - 200 - i * offY);
        card->setPosition(pos);
        // 记录主牌信息
        GameModel::MainCardInfo cardInfo;
        cardInfo.card = card;
        cardInfo.position = pos;
        cardInfo.column = 0; // 左列
        cardInfo.row = i;
        cardInfo.isBottom = false; // 初始化为false，后面统一设置
        cardInfo.isInMainArea = true;
        _model->mainCardsInfo.push_back(cardInfo);

        // 修改点击回调：检查是否是底部牌并匹配
        card->setClickCallback([this, card, pos](int id) {
            if (card->isVisible()) {
                // 检查是否是列中最下面的牌
                if (isBottomCardInColumn(card)) {
                    CCLOG("Bottom card clicked, checking match...");
                    // 检查是否与手牌区顶部牌匹配
                    if (checkMatchWithTopHand(card)) {
                        // 匹配：执行替换
                        replaceTopWithMainCard(card, pos);
                    }
                    else {
                        CCLOG("Match failed, no operation");
                        // 不匹配：不执行任何操作
                    }
                }
                else {
                    CCLOG("Not bottom card, cannot match");
                    // 不是底部牌：不执行任何操作
                }
            }
            });
        this->addChild(card);
    }

    //-----------------------------
    // 右侧卡牌（3,4,5）
    //-----------------------------
    for (int i = totalCards - 1; i >= totalCards / 2; i--)
    {
        auto card = CardView::create(
            i,
            "card_general.png",
            _model->mainCardImages[i],
            cardSize
        );

        int index = (totalCards - 1 - i);

        //  右侧摆放，
        Vec2 pos = Vec2(
            visibleSize.width - (100 + index * offX) - 300,
            totalHeight - 200 - index * offY
        );
        card->setPosition(pos);

        // 记录主牌信息
        GameModel::MainCardInfo cardInfo;
        cardInfo.card = card;
        cardInfo.position = pos;
        cardInfo.column = 1; // 右列
        cardInfo.row = index;
        cardInfo.isBottom = false; // 初始化为false，后面统一设置
        cardInfo.isInMainArea = true;
        _model->mainCardsInfo.push_back(cardInfo);


        // 修改点击回调：检查是否是底部牌并匹配
        card->setClickCallback([this, card, pos](int id) {
            if (card->isVisible()) {
                // 检查是否是列中最下面的牌
                if (isBottomCardInColumn(card)) {
                    CCLOG("Bottom card clicked, checking match...");
                    // 检查是否与手牌区顶部牌匹配
                    if (checkMatchWithTopHand(card)) {
                        // 匹配：执行替换
                        replaceTopWithMainCard(card, pos);
                    }
                    else {
                        CCLOG("Match failed, no operation");
                        // 不匹配：不执行任何操作
                    }
                }
                else {
                    CCLOG("Not bottom card, cannot match");
                    // 不是底部牌：不执行任何操作
                }
            }
            });

        this->addChild(card);
    }
    // 初始化底部牌状态
    refreshAllBottomCards();
}

#pragma mark - Undo 按钮
void GameScene::initUndoButton()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto undoLabel = Label::createWithSystemFont("UNDO", "Arial", 50);
    undoLabel->setColor(Color3B::WHITE);

    auto undoBtn = MenuItemLabel::create(
        undoLabel,
        CC_CALLBACK_1(GameScene::undoLastRemove, this)
    );

    float marginX = 50;
    float marginY = 50;

    undoBtn->setPosition(Vec2(
        origin.x + visibleSize.width - marginX - 50,
        origin.y + marginY + 100
    ));

    auto menu = Menu::create(undoBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);
}

#pragma mark - 初始化手牌区卡牌
void GameScene::initHandCards()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float handHeight = visibleSize.height * 580.0f / 2080.0f;

    cocos2d::Size cardSize(200, 300); // 手牌比主牌小
    float spacingX = 200;            // 水平间距
    float startX = 200;              // 起始 X

    for (int i = 0; i < _model->handCardImages.size(); i++)
    {
        auto card = CardView::create(i + 100, "card_general.png", _model->handCardImages[i], cardSize);
        Vec2 pos = Vec2(startX + i * spacingX, handHeight);
        card->setPosition(pos);

        // 点击卡牌替换顶部牌
        card->setClickCallback([this, i](int id) {
            replaceTopHandCard(i);
            });

        handCards.push_back(card);
        this->addChild(card);
    }
}

#pragma mark - 点击手牌替换顶部牌
void GameScene::replaceTopHandCard(int clickedIndex)
{
    int topIndex = (int)handCards.size() - 1;
    if (clickedIndex == topIndex) return;

    auto clickedCard = handCards[clickedIndex];
    auto topCard = handCards[topIndex];

    Vec2 clickedPos = clickedCard->getPosition();
    Vec2 topPos = topCard->getPosition();

    // 记录操作
    _model->undoStack.push_back(GameModel::OperationRecord(
        GameModel::HAND_CARD_SWAP,
        clickedCard,
        topCard,
        clickedPos,
        topPos,
        clickedIndex,
        topIndex
    ));

    // 立即交换数组元素
    std::swap(handCards[clickedIndex], handCards[topIndex]);
    updateCardCallbacks();

    // 执行动画
    clickedCard->runAction(MoveTo::create(0.3f, topPos));
    topCard->runAction(MoveTo::create(0.3f, clickedPos));
}

void GameScene::updateCardCallbacks()
{
    for (int i = 0; i < handCards.size(); i++) {
        handCards[i]->setClickCallback([this, i](int id) {
            replaceTopHandCard(i);
            });
    }
}

#pragma mark - 检查是否是列中最下面的牌
bool GameScene::isBottomCardInColumn(CardView* card)
{
    for (const auto& cardInfo : _model->mainCardsInfo) {
        // 只考虑在主牌区的卡片
        if (cardInfo.isInMainArea &&
            cardInfo.card == card &&
            cardInfo.isBottom &&
            card->isVisible()) {
            CCLOG("Card %p is bottom card in column %d", card, cardInfo.column);
            return true;
        }
    }
    CCLOG("Card %p is NOT a bottom card", card);
    return false;
}

#pragma mark - 匹配检查
bool GameScene::checkMatchWithTopHand(CardView* mainCard)
{
    if (handCards.empty()) return false;

    CardView* topHandCard = handCards.back(); // 最右侧是顶部牌
    int mainValue = mainCard->getCardValue();
    int handValue = topHandCard->getCardValue();

    CCLOG("Checking match: MainCard=%d, TopHand=%d", mainValue, handValue);

    // 检查点数差是否为1
    return abs(mainValue - handValue) == 1;
}

#pragma mark - 桌面牌替换手牌区顶部牌
void GameScene::replaceTopWithMainCard(CardView* mainCard, const Vec2& mainCardPos)
{
    if (handCards.empty()) return;

    int topIndex = (int)handCards.size() - 1;
    CardView* topHandCard = handCards[topIndex];
    Vec2 topHandPos = topHandCard->getPosition();

    CCLOG("Replacing top hand card with main card");

    // 记录操作 - 保存主牌信息用于undo
    int mainCardId = -1;
    int mainCardColumn = -1;
    int mainCardRow = -1;

    // 找到被匹配的主牌信息
    for (int i = 0; i < _model->mainCardsInfo.size(); i++) {
        if (_model->mainCardsInfo[i].card == mainCard) {
            mainCardId = i;
            mainCardColumn = _model->mainCardsInfo[i].column;
            mainCardRow = _model->mainCardsInfo[i].row;
            break;
        }
    }

    _model->undoStack.push_back(GameModel::OperationRecord(
        GameModel::MATCH_REPLACE,
        mainCard,
        topHandCard,
        mainCardPos,
        topHandPos,
        mainCardId,      // 保存主牌在mainCardsInfo中的索引
        topIndex
    ));

    // 原来的顶部牌消失（隐藏）
    topHandCard->setVisible(false);

    // 将主牌移动到顶部牌位置
    mainCard->runAction(MoveTo::create(0.3f, topHandPos));

    // 将主牌添加到手牌区（替换原来的顶部牌）
    handCards[topIndex] = mainCard;

    // 关键修复：标记主牌已不在主牌区
    if (mainCardId != -1) {
        _model->mainCardsInfo[mainCardId].isInMainArea = false;
    }

    // 设置主牌的点击回调为手牌的回调
    mainCard->setClickCallback([this, topIndex](int id) {
        replaceTopHandCard(topIndex);
        });

    // 立即更新被匹配牌所在列的底部牌状态
    if (mainCardColumn != -1) {
        updateBottomCardInColumn(mainCardColumn);
    }

    // 刷新所有底部牌状态
    refreshAllBottomCards();
}
#pragma mark - 更新特定列的底部牌状态
void GameScene::updateBottomCardInColumn(int column)
{
    CCLOG("Updating bottom card for column %d", column);

    // 重置该列所有牌的底部状态
    for (auto& cardInfo : _model->mainCardsInfo) {
        if (cardInfo.column == column) {
            cardInfo.isBottom = false;
        }
    }

    // 找到该列中新的底部牌（最下面的可见牌）
    int maxRow = -1;
    GameModel::MainCardInfo* newBottomCard = nullptr;

    for (auto& cardInfo : _model->mainCardsInfo) {
        // 关键修复：只考虑在主牌区且可见的卡片
        if (cardInfo.isInMainArea &&
            cardInfo.column == column &&
            cardInfo.card->isVisible() &&
            cardInfo.row > maxRow) {
            maxRow = cardInfo.row;
            newBottomCard = &cardInfo;
        }
    }

    if (newBottomCard) {
        newBottomCard->isBottom = true;
        CCLOG("New bottom card in column %d: row %d, card %p",
            column, newBottomCard->row, newBottomCard->card);
    }
    else {
        CCLOG("No bottom card found in column %d", column);
    }
}


#pragma mark - Undo 返回最后一张操作
void GameScene::undoLastRemove(cocos2d::Ref* sender)
{
    if (_model->undoStack.empty())
    {
        CCLOG("No operations to undo");
        return;
    }

    auto lastOp = _model->undoStack.back();
    _model->undoStack.pop_back();

    CCLOG("Undoing operation type: %d", lastOp.type);

    switch (lastOp.type) {
    case GameModel::HAND_CARD_SWAP:
        // 手牌交换撤销
    {
        int index1 = lastOp.index1;
        int index2 = lastOp.index2;

        // 安全检查：确保索引在有效范围内
        if (index1 < 0 || index1 >= handCards.size() ||
            index2 < 0 || index2 >= handCards.size()) {
            CCLOG("Error: Invalid hand card indices in undo operation");
            return;
        }

        CCLOG("Undo: Hand card swap from index %d and %d", index1, index2);

        // 交换回来
        std::swap(handCards[index1], handCards[index2]);
        updateCardCallbacks();

        // 动画回到原位置
        handCards[index1]->runAction(MoveTo::create(0.3f, lastOp.pos1));
        handCards[index2]->runAction(MoveTo::create(0.3f, lastOp.pos2));
    }
    break;

    case GameModel::MATCH_REPLACE:
        // 匹配替换撤销
    {
        CCLOG("Undo: Match replace");

        // 安全检查：确保卡片指针有效
        if (lastOp.card1 == nullptr || lastOp.card2 == nullptr) {
            CCLOG("Error: Null card pointer in undo operation");
            return;
        }

        // 安全检查：确保手牌索引有效
        if (lastOp.index2 < 0 || lastOp.index2 >= handCards.size()) {
            CCLOG("Error: Invalid hand card index in undo operation");
            return;
        }

        // 恢复原来的顶部牌
        lastOp.card2->setVisible(true);
        lastOp.card2->setPosition(lastOp.pos2);

        // 恢复手牌数组
        handCards[lastOp.index2] = lastOp.card2;

        // 恢复主牌到原来位置
        lastOp.card1->runAction(MoveTo::create(0.3f, lastOp.pos1));

        // 关键修复：恢复主牌在主牌区的状态
        int mainCardId = lastOp.index1;
        if (mainCardId >= 0 && mainCardId < _model->mainCardsInfo.size()) {
            _model->mainCardsInfo[mainCardId].isInMainArea = true;
        }

        // 恢复主牌的回调函数
        // 找到主牌所在的列和行，用于重建回调
        int mainCardColumn = -1;
        int mainCardRow = -1;
        Vec2 mainCardPos = lastOp.pos1;

        for (const auto& cardInfo : _model->mainCardsInfo) {
            if (cardInfo.card == lastOp.card1) {
                mainCardColumn = cardInfo.column;
                mainCardRow = cardInfo.row;
                break;
            }
        }

        // 重建主牌回调函数
        lastOp.card1->setClickCallback([this, card = lastOp.card1, pos = mainCardPos, column = mainCardColumn, row = mainCardRow](int id) {
            if (card->isVisible()) {
                // 检查是否是列中最下面的牌
                if (isBottomCardInColumn(card)) {
                    CCLOG("Bottom card clicked, checking match...");
                    // 检查是否与手牌区顶部牌匹配
                    if (checkMatchWithTopHand(card)) {
                        // 匹配：执行替换
                        replaceTopWithMainCard(card, pos);
                    }
                    else {
                        CCLOG("Match failed, no operation");
                    }
                }
                else {
                    CCLOG("Not bottom card, cannot match");
                }
            }
            });

        // 设置原来的顶部牌的回调
        lastOp.card2->setClickCallback([this, index = lastOp.index2](int id) {
            replaceTopHandCard(index);
            });

        // 关键修复：根据被撤销的主牌恢复其所在列的底部牌状态
        if (mainCardColumn != -1) {
            updateBottomCardInColumn(mainCardColumn);
        }

        // 刷新所有底部牌状态
        refreshAllBottomCards();
    }
    break;
    }
}

#pragma mark - 刷新所有底部牌状态
void GameScene::refreshAllBottomCards()
{
    CCLOG("Refreshing bottom card states...");

    // 重置所有底部牌状态
    for (auto& cardInfo : _model->mainCardsInfo) {
        cardInfo.isBottom = false;
    }

    // 为每列设置底部牌（每列最下面的可见牌）
    for (int col = 0; col <= 1; col++) {
        int maxRow = -1;
        GameModel::MainCardInfo* bottomCard = nullptr;

        for (auto& cardInfo : _model->mainCardsInfo) {
            // 关键修复：只考虑在主牌区且可见的卡片
            if (cardInfo.isInMainArea &&
                cardInfo.column == col &&
                cardInfo.card->isVisible() &&
                cardInfo.row > maxRow) {
                maxRow = cardInfo.row;
                bottomCard = &cardInfo;
            }
        }

        if (bottomCard) {
            bottomCard->isBottom = true;
            CCLOG("Bottom card in column %d: row %d, card %p",
                col, bottomCard->row, bottomCard->card);
        }
        else {
            CCLOG("No bottom card found in column %d", col);
        }
    }
}
