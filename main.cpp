#include "raylib.h"
#include "lib/rayoutgen.hpp"
#include "lib/raygui.h"
#include "lib/element.h"
#include <iostream>
#include "lib/libtransform.h"
#include "torch/torch.h"
#include <torch/script.h>
#include <chrono>

struct PathTemp {
    std::string theme;
    std::string win_icon;
    std::string icon_path;

    PathTemp(std::string theme, std::string win_icon, std::string icon_path): theme(std::move(theme)), win_icon(std::move(win_icon)), icon_path(std::move(icon_path)) {}
};

inline void themeMode(const PathTemp &path_temp, Image &iconWin) {
    GuiLoadStyle(path_temp.theme.c_str());
    iconWin = LoadImage(path_temp.win_icon.c_str());
    SetWindowIcon(iconWin);
    UnloadImage(iconWin);
}

bool interrupt(const int &frameCount,int &previousFrame, int interval, int fps) {
    if (frameCount-previousFrame >= interval*fps) { //detol
        previousFrame = frameCount;
        return true;
    }
    return false;
}

Color hex_to_rgb(const std::string &color="979dac") {
    std::string hexColor = std::string("0x") + color + std::string("ff");
    unsigned int colorValue = std::stoul(hexColor, nullptr, 16); // Basis 16 untuk hexadecimal
    Color rgbColor = GetColor(colorValue);
    return rgbColor;
}


#define WIN32 1 // Compilation platform
#define panel_2 1
#if WIN32
int WinMain() {
#else
int main(int argc, char *argv[]) {
#endif

    ////LIBTORCH INSTANCIATE
    libtrh::LibTransform lib_transform;

    constexpr int screenWidth = 600;
    constexpr int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Model Evaluation Machine");
    // bool style_mode = false;
    PathTemp icon_theme_path(R"(D:\project-inf\remake-evalution-software\icon\white_red.rgs)",
        R"(D:\project-inf\remake-evalution-software\icon\card_joker_red.png)",
            R"(D:\project-inf\remake-evalution-software\icon\iconset.rgi.rgi)");
    GuiLoadStyle(icon_theme_path.theme.c_str());
    GuiLoadIcons(icon_theme_path.icon_path.c_str(),true);

    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    Image iconWin = LoadImage(icon_theme_path.win_icon.c_str());
    SetWindowIcon(iconWin);
    UnloadImage(iconWin);

    //// Panel Session
    rtg::Panel4Panel<elm::Component>::WIN_WIDTH_ = screenWidth;
    rtg::Panel4Panel<elm::Component>::WIN_HEIGHT_ = screenHeight;

    elm::PathHandler get_all_path("","","",0);
    elm::Component get_static(0,0,0,0);

    //textbox
    int panel_height = 300;
    rtg::Panel4Panel<elm::Component> panel1(0,40,800, panel_height);
    panel1.marginAll(10);


    std::array< std::shared_ptr<elm::Component>, 3> text_box;
    std::array< std::string, 3> label = {"#8#Path model", "#8#Path Image", "#8#Path csv file"};

    int height_comp = 30, gap = 5, current_pos=0, last_position = 0;
    for (int i=0; i < text_box.size(); i++) {
        text_box[i] = std::make_shared<elm::Component>(label[i].c_str(),300, height_comp);
        panel1.setElement(text_box[i]).topRight(0,current_pos);
        text_box[i]->initAsTextBox();

        current_pos = height_comp*(i+1)+gap; //memberikan jarak antar komponen
        gap *= 2;
        if (i == text_box.size() - 1) {
            last_position = height_comp*(i+1)+gap;
        }
    }
    // std::cout << last_position << std::endl;

    //button
    auto classify_button = std::make_shared<elm::Component>("CLASSIFY",100, 50,"#77#");
    panel1.setElement(classify_button).bottomRight(0,0);
    classify_button->initAsButton();

    //Image
    auto image_area = std::make_shared<elm::Component>(224, 224);
    panel1.setElement(image_area).topLeft(5,5);
    image_area->initAsImage();

    //Spinner
    auto spinner_mode = std::make_shared<elm::Component>(200, 30);
    panel1.setElement(spinner_mode).topRight(0,last_position);
    spinner_mode->initAsSpinner();

#if panel_2
    std::vector<std::string> header = {"Classes", "Prediction", "Probabilities"};
    rtg::Panel4Panel<elm::Component> panel2(0,350,800, 900);
    panel2.marginAll(10);

    auto eval_table = std::make_shared<elm::Component>(header, 150, 30);
    panel2.setElement(eval_table).centerTop(0,-10);
    eval_table->initAsEvalTable();

    auto project_maker = std::make_shared<elm::Component>("#150#Created by TIAF (a.k.a Tri Ilman A. Fattah)",
        400, 20);
    panel2.setElement(project_maker).bottomLeft(0,0);
    project_maker->initAsText();
#endif

    //control condition
    bool is_error = false;
    int frameCounter = 0;
    int previousFrame =0;
    bool is_theme_change = false;
    while (!WindowShouldClose())
    {
        //icon and theme control
        frameCounter++;
        if (IsKeyPressed(KEY_ONE) && (IsKeyDown(KEY_LEFT_SHIFT)|| IsKeyDown(KEY_RIGHT_SHIFT))) {
            icon_theme_path.theme = R"(D:\project-inf\remake-evalution-software\icon\white_red.rgs)";
            icon_theme_path.win_icon = R"(D:\project-inf\remake-evalution-software\icon\card_joker_red.png)";
            is_theme_change = false;
            themeMode(icon_theme_path,iconWin);
        } else if (IsKeyPressed(KEY_TWO) && (IsKeyDown(KEY_LEFT_SHIFT)|| IsKeyDown(KEY_RIGHT_SHIFT))) {
            icon_theme_path.theme = R"(D:\project-inf\remake-evalution-software\icon\dark.rgs)";
            icon_theme_path.win_icon = R"(D:\project-inf\remake-evalution-software\icon\card_joker_black.png)";
            is_theme_change = true;
            themeMode(icon_theme_path,iconWin);
        }
        //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

        spinner_mode->applyMode().setStaticPath(4);
        classify_button->applyMode();
        eval_table->applyMode();
        int i = 0;
        for (auto &text : text_box) {
            text->applyMode().droppedMode();

            if (text->isClicked) {
                text->setStaticPath(i+1); // it's +1 because the id start from 1 not 0
                if (i==1) image_area->applyMode();
            }
            ++i;
        }

        if (classify_button->isClicked) {

            //duration declaration
            std::chrono::duration<double> duration{0};
            get_all_path = elm::Component::getGlobalPath();
            if (!(IsFileExtension(get_all_path.image.c_str(), ".jpeg")||
                IsFileExtension(get_all_path.image.c_str(), ".jpg")) ||
                !(IsFileExtension(get_all_path.module.c_str(), ".pt") ||
                IsFileExtension(get_all_path.module.c_str(), ".pth")))
            {
                is_error = true;
            }
            else {
                try{
                    lib_transform(get_all_path).toCvImage().resizeImage();
                    lib_transform.reverseImageForm().toRangeTensor();
                    lib_transform.normalization().toInputValue(); //get tensor input type

                    lib_transform.loadModel(); // load model
                    if (lib_transform.loadModel() != 0) {
                        std::cerr << "Model loading failed, skipping inference.\n";
                        return -1;
                    }
                     // start timer
                    auto start = std::chrono::high_resolution_clock::now();

                    // forward propagation
                    lib_transform.forward();

                    // Hentikan timer
                    auto end = std::chrono::high_resolution_clock::now();
                    duration = end - start;

                }catch (const std::exception &e) {
                    std::cerr << "Error during pipeline: " << e.what() << std::endl;
                }
                std::cout << "Inference time: " << duration.count() << std::endl;
                std::vector<std::array<float, 2>> outputs = lib_transform.getStdOutput(); // Get Output after forward
                eval_table->setEvalValue(is_theme_change,outputs, duration.count()); // get evaluation value

            }
        }

        BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        panel1.displayAll();

#if panel_2
        if (is_theme_change) {
            panel2.display(DARKGRAY);
            panel2.displayAll();
        } else {
            panel2.display(hex_to_rgb("8d98a7"));
            panel2.displayAll();
        }
#endif

        //============DEFER SECTION================
        if (is_error)
        { // error notification box
            bool close_window = GuiMessageBox({static_cast<int>(screenWidth/2-100),static_cast<int>(screenHeight/2-100),200,200},"#152#Warning","Tidak ada model/image\n yang dimasukkan!","#56#");
            if (interrupt(frameCounter,previousFrame,5,60) || close_window == 0) {
                is_error = false;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}