// Phase 0 spike: cocos2d-x 3.10 on Nintendo Switch.
// Checks GLES2 rendering, system-font labels (Device::getTextureDataForText -> shared font)
// and multi-touch (libnx HID -> GLView::handleTouches*).
#include <switch.h>
#include "cocos2d.h"

USING_NS_CC;

class HelloScene : public Layer
{
public:
    CREATE_FUNC(HelloScene);

    bool init() override
    {
        if (!Layer::init())
            return false;
        const Size vs = Director::getInstance()->getVisibleSize();

        addChild(LayerColor::create(Color4B(32, 40, 64, 255)));

        // version string comes from the linked library, not hardcoded
        auto title = Label::createWithSystemFont(
            std::string("Kirikiroid2-Switch: ") + cocos2dVersion() + " OK\n吉里吉里 テスト 中文测试", "sans", 36);
        title->setPosition(vs.width / 2, vs.height - 80);
        addChild(title);

        _status = Label::createWithSystemFont("touch the screen, + to exit", "sans", 28);
        _status->setPosition(vs.width / 2, 60);
        addChild(_status);

        _dots = DrawNode::create();
        addChild(_dots);

        auto touch = EventListenerTouchAllAtOnce::create();
        touch->onTouchesBegan = CC_CALLBACK_2(HelloScene::onTouches, this);
        touch->onTouchesMoved = CC_CALLBACK_2(HelloScene::onTouches, this);
        touch->onTouchesEnded = [this](const std::vector<Touch*>& t, cocos2d::Event*) {
            _status->setString(StringUtils::format("released %d touch(es)", (int)t.size()));
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&_pad);
        scheduleUpdate();
        return true;
    }

    void update(float) override
    {
        padUpdate(&_pad);
        if (padGetButtonsDown(&_pad) & HidNpadButton_Plus)
            Director::getInstance()->end();
    }

private:
    void onTouches(const std::vector<Touch*>& touches, cocos2d::Event*)
    {
        _dots->clear();
        for (auto t : touches)
            _dots->drawSolidCircle(t->getLocation(), 40, 0, 32, Color4F(1.f, 0.6f, 0.2f, 1.f));
        const Vec2 p = touches[0]->getLocation();
        _status->setString(StringUtils::format("%d touch(es)  first=(%.0f, %.0f)", (int)touches.size(), p.x, p.y));
    }

    Label* _status = nullptr;
    DrawNode* _dots = nullptr;
    PadState _pad;
};

class AppDelegate : public Application
{
public:
    void initGLContextAttrs() override
    {
        GLContextAttrs attrs = {8, 8, 8, 8, 24, 8};
        GLView::setGLContextAttrs(attrs);
    }

    bool applicationDidFinishLaunching() override
    {
        auto director = Director::getInstance();
        auto glview = director->getOpenGLView();
        if (!glview)
        {
            glview = GLViewImpl::createWithRect("Kirikiroid2", Rect(0, 0, 1280, 720));
            director->setOpenGLView(glview);
        }
        glview->setDesignResolutionSize(1280, 720, ResolutionPolicy::SHOW_ALL);
        director->setDisplayStats(true);
        director->setAnimationInterval(1.0f / 60);

        auto scene = Scene::create();
        scene->addChild(HelloScene::create());
        director->runWithScene(scene);
        return true;
    }

    void applicationDidEnterBackground() override { Director::getInstance()->stopAnimation(); }
    void applicationWillEnterForeground() override { Director::getInstance()->startAnimation(); }
};

int main(int argc, char** argv)
{
    socketInitializeDefault();
    nxlinkStdio(); // printf/CCLOG -> nxlink -s on the PC
    AppDelegate app;
    int rc = Application::getInstance()->run();
    socketExit();
    return rc;
}
