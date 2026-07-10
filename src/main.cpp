#include <Geode/Geode.hpp>
#include <Geode/modify/ColorSelectPopup.hpp>
#include <Geode/utils/web.hpp>
#include <string>
#include <vector>
#include <sstream>

using namespace geode::prelude;

// Помощна функция за преобразуване на Hex стринг (напр. "ff5733") в cocos2d::ccColor3B
cocos2d::ccColor3B hexToColor(const std::string& hex) {
    if (hex.length() != 6) return cocos2d::ccc3(255, 255, 255);
    unsigned int r, g, b;
    std::stringstream ssR, ssG, ssB;
    ssR << std::hex << hex.substr(0, 2); ssR >> r;
    ssG << std::hex << hex.substr(2, 2); ssG >> g;
    ssB << std::hex << hex.substr(4, 2); ssB >> b;
    return cocos2d::ccc3(r, g, b);
}

// Функция, която извлича Hex кодовете от Coolors URL адрес
// Пример: https://coolors.co -> ["264653", "2a9d8f", ...]
std::vector<std::string> extractHexFromCoolors(std::string url) {
    std::vector<std::string> colors;
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash == std::string::npos) return colors;
    
    std::string hexPart = url.substr(lastSlash + 1);
    std::stringstream ss(hexPart);
    std::string item;
    while (std::getline(ss, item, '-')) {
        // Coolors понякога добавя допълнителни параметри, филтрираме само валидни 6-символни hex кодове
        if (item.length() == 6) {
            colors.push_back(item);
        }
    }
    return colors;
}

// Изскачащ прозорец (Интерфейс) за въвеждане на URL адреса
class PaletteInputDialog : public FLAlertLayer {
    CCTextInputNode* m_inputNode;
    ColorSelectPopup* m_parentPopup;

public:
    static PaletteInputDialog* create(ColorSelectPopup* parent) {
        auto ret = new PaletteInputDialog();
        if (ret && ret->init(parent)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(ColorSelectPopup* parent) {
        if (!FLAlertLayer::init(150)) return false;
        m_parentPopup = parent;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        // Създаване на фонов панел за прозореца
        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({320, 160});
        bg->setPosition(winSize / 2);
        m_mainLayer->addChild(bg);

        // Текст / Заглавие
        auto title = CCLabelBMFont::create("Import Coolors Palette", "bigFont.fnt");
        title->setScale(0.55f);
        title->setPosition(winSize.width / 2, winSize.height / 2 + 55);
        m_mainLayer->addChild(title);

        // Текстово поле за въвеждане (Съвместимо с PC и Смартфони)
        m_inputNode = CCTextInputNode::create(240, 30, "Paste Coolors URL here...", "chatFont.fnt");
        m_inputNode->setPosition(winSize.width / 2, winSize.height / 2 + 10);
        m_inputNode->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/:.");
        m_mainLayer->addChild(m_inputNode);

        // Меню за бутоните
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        // Бутон "PASTE" (Изключително важен за телефони)
        auto pasteSprite = ButtonSprite::create("Paste", 60, true, "goldFont.fnt", "GJ_button_04.png", 25, 0.6f);
        auto pasteBtn = CCMenuItemSpriteExtra::create(pasteSprite, this, menu_selector(PaletteInputDialog::onPaste));
        pasteBtn->setPosition(winSize.width / 2 - 80, winSize.height / 2 - 40);
        menu->addChild(pasteBtn);

        // Бутон "APPLY"
        auto applySprite = ButtonSprite::create("Apply", 60, true, "goldFont.fnt", "GJ_button_01.png", 25, 0.6f);
        auto applyBtn = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(PaletteInputDialog::onApply));
        applyBtn->setPosition(winSize.width / 2, winSize.height / 2 - 40);
        menu->addChild(applyBtn);

        // Бутон "CANCEL" за затваряне
        auto cancelSprite = ButtonSprite::create("X", 25, true, "goldFont.fnt", "GJ_button_06.png", 25, 0.6f);
        auto cancelBtn = CCMenuItemSpriteExtra::create(cancelSprite, this, menu_selector(PaletteInputDialog::onClose));
        cancelBtn->setPosition(winSize.width / 2 + 80, winSize.height / 2 - 40);
        menu->addChild(cancelBtn);

        // Позволява затваряне с 'Escape' или бутона 'Back' на телефона
        this->setKeypadEnabled(true);
        return true;
    }

    void onPaste(CCObject*) {
        // Извиква клипборда на устройството (работи автоматично на Windows, Android и iOS)
        std::string clipboard = utils::clipboard::get();
        if (!clipboard.empty()) {
            m_inputNode->setString(clipboard);
        }
    }

    void onApply(CCObject*) {
        std::string url = m_inputNode->getString();
        auto hexColors = extractHexFromCoolors(url);

        if (hexColors.empty()) {
            FLAlertLayer::create("Error", "Invalid Coolors URL format!", "OK")->show();
            return;
        }

        // Взимаме текущото отворено ниво през редактора
        auto editor = LevelEditorLayer::get();
        if (!editor || !editor->m_levelSettings) return;
        auto effectManager = editor->m_levelSettings->m_effectManager;
        if (!effectManager) return;

        // Взимаме текущия избран канал (ID) от прозореца, който потребителят е отворил
        int startChannel = m_parentPopup->m_colorAction->m_colorID;

        // Нанасяме последователно извлечените цветове, започвайки от текущия канал нагоре
        for (size_t i = 0; i < hexColors.size(); ++i) {
            int currentChannel = startChannel + static_cast<int>(i);
            auto colorAction = effectManager->getColorAction(currentChannel);
            if (colorAction) {
                auto ccColor = hexToColor(hexColors[i]);
                colorAction->m_color = ccColor;
                colorAction->m_copyColor = ccColor;
            }
        }

        // Обновяваме визуално редактора и затваряме диалога
        effectManager->updateColors();
        m_parentPopup->updateColorSprites(); // Обновява UI елементите в самия Color Popup
        
        FLAlertLayer::create("Success", "Palette applied successfully starting from this channel!", "Awesome")->show();
        onClose(nullptr);
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};

// ============================================================================
// 2. HOOK В COLOR SELECT POPUP (За добавяне на "+" бутона в долния десен ъгъл)
// ============================================================================
class $modify(MyColorSelectPopup, ColorSelectPopup) {
    bool init(ColorAction* action, ColorTriggerDelegate* delegate, cocos2d::CCArray* array, bool unknown1, bool unknown2) {
        if (!ColorSelectPopup::init(action, delegate, array, unknown1, unknown2)) return false;

        // Търсим главното меню в прозореца, където държим бутоните
        auto menu = this->getChildByID("main-menu");
        if (!menu) {
            // Ако липсва ID (при по-стари версии), създаваме ново алтернативно меню закачено към прозореца
            menu = CCMenu::create();
            menu->setPosition({0, 0});
            this->addChild(menu);
        }

        // Създаваме "+" бутона със стандартния зелен кръг на Geometry Dash
        auto plusSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if (plusSprite) {
            plusSprite->setScale(0.75f);
            auto plusBtn = CCMenuItemSpriteExtra::create(
                plusSprite,
                this,
                menu_selector(MyColorSelectPopup::onCoolorsPlusClicked)
            );
            plusBtn->setID("coolors-import-button");

            // Позиционираме го точно в долния десен ъгъл на екрана/интерфейса
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            plusBtn->setPosition({ winSize.width / 2 + 165, winSize.height / 2 - 115 });

            menu->addChild(plusBtn);
        }

        return true;
    }

    void onCoolorsPlusClicked(CCObject* sender) {
        // Отваряме нашия къстъм инпут прозорец, подавайки "this" (текущия ColorSelectPopup)
        auto dialog = PaletteInputDialog::create(this);
        if (dialog) {
            dialog->show();
        }
    }
};
