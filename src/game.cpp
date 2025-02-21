//
// Created by lixh1 on 25-2-8.
//
#include <game.h>

Game *newgame;

Game::~Game() {
    // delete hero;
    // for (auto it = bullets_hero.begin(); it != bullets_hero.end(); ++it) {
    //     delete *it;
    // }
    // bullets_hero.clear(); // 清空容器
    //
    // // 删除 bullets_monster 中的所有子弹
    // for (auto it = bullets_monster.begin(); it != bullets_monster.end(); ++it) {
    //     delete *it;
    // }
    // bullets_monster.clear(); // 清空容器
    //
    // // 删除 monster 中的所有怪物
    // for (auto it = monster.begin(); it != monster.end(); ++it) {
    //     delete *it;
    // }
    // monster.clear(); // 清空容器
}

void Game::game() {
    tick = SDL_GetTicks();
    for (auto & i : walls_1) {
        walls.emplace_back(i[0], i[1], i[2], i[3]);
    }
    while (!IsQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    IsQuit = true;
                break;
                case SDL_KEYDOWN:
                    do_keydown(event);
                break;
                case SDL_MOUSEBUTTONDOWN:
                    do_mouse_down(event);
                break;
                case SDL_MOUSEMOTION:
                    do_mouse_motion(event);
                break;
                default:
                    break;
            }
        }
        if (hero) {
            hero->Move(MousePos);
            if (!monster.empty()) {
                for (const auto & i : monster) {
                    i->Move(*hero);
                }
            }
        }
        level_control();

        display();
        SDL_Delay(1000 / FPS);
        if (hero && hero->getHP() == 0){
            showSystemMessageBox("Game Over!", "INFO");
            IsQuit = true;
        }
    }
    IsQuit = false;
}




void Game::do_keydown(const SDL_Event &event) {
    switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE:
            break;
        case SDL_SCANCODE_SPACE:
            IsQuit = true;
        break;
        case SDL_SCANCODE_RETURN:
            CreateHero();
        break;
        case SDL_SCANCODE_J:
            Fire();
        break;
        case SDL_SCANCODE_Z :
            if (hero) {
                monster.push_back(new Monster_type3(5,200,3.5,{900,600},{41,36},"../rsc/Demon bat.png", bullet_2));
            }
        break;
        case SDL_SCANCODE_X:
            if (hero) {
                monster.push_back(new Monster_type2(5,200,3.5,{8,12},{700,500},{24,24},"../rsc/ghost.png",weapon_1));
            }
        break;
        case SDL_SCANCODE_C:
            if (hero) {
                monster.push_back(new Monster_type4(100,200,6.5,{700,500},{111,123}));
            }
        default:
            break;
    }
}
void Game::do_mouse_down(const SDL_Event &event) const {
    if (event.button.button == SDL_BUTTON_LEFT) {
        Fire();
    }
}
void Game::do_mouse_motion(const SDL_Event &event) {
    MousePos.x = event.motion.x;
    MousePos.y = event.motion.y;
}
void Game::display() {
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);
    // 加载背景、帧率
    Render_ground_game();
    Render_background();
    SDL_DestroyTexture(background_texture);
    const Uint32 duration = SDL_GetTicks() - tick;
    fps = 1000 / static_cast<int>(duration);
    tick = SDL_GetTicks();
    Render_fps(fps);
    SDL_DestroyTexture(fps_texture);
    Render_Elements();
    Present();
}



void Game::Render_ground_game() {
    SDL_Texture *texture = nullptr;
    LoadImage(texture, "../rsc/mystery-forest.png");
    constexpr SDL_Rect groundRect = {-180, 0, 1500, 1000};
    constexpr SDL_Rect groundRect2 = {200, 0, 1500, 1000};
    SDL_RenderCopy(app.renderer, texture, nullptr, &groundRect);
    SDL_RenderCopy(app.renderer, texture, nullptr, &groundRect2);
    SDL_DestroyTexture(texture);
}
void Game::Render_background() {
    LoadImage(background_texture, "../rsc/background.png");
    constexpr SDL_Rect backgroundRect = {307, 60, 885, 880};
    SDL_RenderCopy(app.renderer, background_texture, nullptr, &backgroundRect);
}
void Game::Render_fps(const int fps) {
    TTF_Font *font = TTF_OpenFont("../rsc/svgafix.fon", 30);
    LoadText(fps_texture, font, "FPS: " + std::to_string(fps), WHITE);
    constexpr SDL_Rect fpsRect = {0, WINDOW_HEIGHT - 30, 80, 30};
    SDL_RenderCopy(app.renderer, fps_texture, nullptr, &fpsRect);
    TTF_CloseFont(font);
}
void Game::Render_Elements() const {
    // 加载人物、武器、子弹、怪物
    if (hero) {
        hero->render(MousePos);
        if (hero->getWeapon()) {
            hero->getWeapon()->UpdatePos(hero->getWeaponPoint().x + hero->getX(), hero->getWeaponPoint().y + hero->getY());
            hero->getWeapon()->render(hero->get_DirAttack(), hero->get_Dir());
        }
    }
    // 检测英雄子弹并渲染
    if (!bullets_hero.empty()) {
        for (auto it_bullet = bullets_hero.begin(); it_bullet != bullets_hero.end();) {
            auto bullet = *it_bullet;
            if (isColliding(*bullet)) {
                delete bullet;
                it_bullet = bullets_hero.erase(it_bullet);
                bullet = nullptr;
            }
            else if (!monster.empty()) {
                for (const auto m : monster) {
                    if (bullet->checkCollision(*m)) {
                        m->Hurt(bullet->get_attack_power());
                        delete bullet;
                        it_bullet = bullets_hero.erase(it_bullet);
                        bullet = nullptr;
                        break;
                    }
                }
            }
            if (bullet != nullptr){
                if (bullet->isFirst())
                    bullet->render();
                else {
                    bullet->Update();
                    bullet->render();
                }
                ++it_bullet;
            }
        }
    }
    // 杀死怪物
    for (auto it_monster = monster.begin(); it_monster != monster.end();) {
        if (const auto m = *it_monster; m->getDelete()) {
            delete m;
            hero->Sub_energy(-5);
            it_monster = monster.erase(it_monster);
        }
        else {
            ++it_monster;
        }
    }
    // 渲染怪物
    if (!monster.empty()) {
        for (const auto monster : monster) {
            monster->Render();
        }
    }
    // 检测怪物子弹并渲染
    if (!bullets_monster.empty()) {
        for (auto it_bullet = bullets_monster.begin(); it_bullet != bullets_monster.end();) {
            if (const auto bullet = *it_bullet; isColliding(*bullet)) {
                delete bullet;
                it_bullet = bullets_monster.erase(it_bullet);
            }
            else if (bullet->checkCollision(*hero)) {
                hero->Hurt(bullet->get_attack_power());
                delete bullet;
                it_bullet = bullets_monster.erase(it_bullet);
            }
            else {
                if (bullet->isFirst())
                    bullet->render();
                else {
                    bullet->Update();
                    bullet->render();
                }
                ++it_bullet;
            }
        }
    }
}
void Game::Fire() const {
    if (hero != nullptr){
        if (const Weapon *w = hero->getWeapon(); w != nullptr){
            w->UpdatePos(hero->getWeaponPoint().x + hero->getX(), hero->getWeaponPoint().y + hero->getY());
            w->UpdateDir(MousePos.x, MousePos.y);
            if (hero->getEnergy() - hero->getWeapon()->getBullet()->energy_consumed >= 0){
                hero->attack();
                hero->Sub_energy(hero->getWeapon()->getBullet()->energy_consumed);
            }
        }
    }
}
void Game::CreateHero() {
    if (!hero)
        hero = new Hero(hero_1);
    if (hero && !hero->getWeapon()) {
        hero->setWeapon(new Weapon_type_1(weapon_1, bullets_hero));
    }
    if (!level)
        level = new Level1();
}
void Game::level_control() {
    if (level) {
        if (!level->getRunning(0))
            level->run();
        else if (monster.empty() && !level->getPass(0)) {
            level->setPass(0);
            if (!level->getRunning(1)) {
                level->run();
            }
        }
        else if (monster.empty() && !level->getPass(1)) {
            level->setPass(1);
            if (!level->getRunning(2)) {
                level->run();
            }
        }
        else if (monster.empty() && !level->getPass(2)) {
            level->setPass(2);
            delete level;
            showSystemMessageBox("You Win!", "INFO");
            IsQuit = true;
        }
    }
}
