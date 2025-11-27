#include "CardView.h"
USING_NS_CC;

CardView* CardView::create(int cardId, const std::string& bgImage,
    const std::vector<std::string>& imageFiles, const Size& size)
{
    CardView* ret = new CardView();
    if (ret && ret->init(cardId, bgImage, imageFiles, size))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CardView::init(int cardId, const std::string& bgImage,
    const std::vector<std::string>& imageFiles, const Size& size)
{
    if (!Node::init()) return false;
    _cardId = cardId;
    _cardSize = size;

    // **设置 Node 自身大小**，
    this->setContentSize(_cardSize);

    // 背景 Sprite
    _bgSprite = Sprite::create(bgImage);
    if (_bgSprite)
    {
        _bgSprite->setContentSize(_cardSize);
        _bgSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        _bgSprite->setPosition(_cardSize.width / 2, _cardSize.height / 2);
        this->addChild(_bgSprite, -1);
    }

    // 添加多张子图片
    //float offsetX = 10; float offsetY = 10;
    if (imageFiles.size() >= 3)
    {
        // 第一张图片：放在中间
        auto sprite1 = Sprite::create(imageFiles[0]);
        sprite1->setAnchorPoint(Vec2(0.5f, 0.5f)); // 中心点
        sprite1->setPosition(Vec2(_cardSize.width / 2, _cardSize.height / 2 - 30));
        this->addChild(sprite1);
        _sprites.push_back(sprite1);

        // 第二张图片：放左上角
        auto sprite2 = Sprite::create(imageFiles[1]);
        sprite2->setAnchorPoint(Vec2(0, 1)); // 左上角
        sprite2->setPosition(Vec2(20, _cardSize.height - 20));
        this->addChild(sprite2);
        _sprites.push_back(sprite2);

        // 第三张图片：放右上角
        auto sprite3 = Sprite::create(imageFiles[2]);
        sprite3->setAnchorPoint(Vec2(1, 1)); // 右上角
        sprite3->setPosition(Vec2(_cardSize.width - 20, _cardSize.height - 20));
        this->addChild(sprite3);
        _sprites.push_back(sprite3);
    }

    // 从文件名解析卡牌信息
    if (!imageFiles.empty()) {
        extractCardInfo(imageFiles[0]);
    }

    // 点击事件：直接判断 Node 包围盒
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        Vec2 loc = this->getParent()->convertToNodeSpace(touch->getLocation());
        if (this->getBoundingBox().containsPoint(loc))
        {
            //if (_clickCallback) _clickCallback(_cardId);
            return true;
        }
        return false;
        };
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_clickCallback) _clickCallback(_cardId);
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

//从文件名解析卡牌信息
void CardView::extractCardInfo(const std::string& imageFile)
{
    // 从文件名解析点数和花色
    // 示例文件名: "big_red_3.png", "big_black_A.png", "big_black_Q.png"
    std::string filename = imageFile;

    // 移除文件扩展名
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }

    // 按"_"分割
    std::vector<std::string> parts;
    size_t start = 0, end = 0;
    while ((end = filename.find('_', start)) != std::string::npos) {
        parts.push_back(filename.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(filename.substr(start));

    // 解析花色和点数
    if (parts.size() >= 3) {
        // 花色在第二个词
        _cardSuit = parts[1]; // "red", "black"

        // 点数在第三个词
        std::string valueStr = parts[2];

        // 处理特殊点数
        if (valueStr == "A") _cardValue = 1;
        else if (valueStr == "J") _cardValue = 11;
        else if (valueStr == "Q") _cardValue = 12;
        else if (valueStr == "K") _cardValue = 13;
        else _cardValue = std::stoi(valueStr);

        CCLOG("Card parsed: suit=%s, value=%d", _cardSuit.c_str(), _cardValue);
    }
}


void CardView::setClickCallback(const std::function<void(int)>& callback)
{
    _clickCallback = callback;
}
