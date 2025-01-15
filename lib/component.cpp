//
// Created by triil on 1/4/2025.
//


#include "element.h"
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <rapidcsv.h>

#include <utility>

namespace elm {
    std::string Component::GLOB_PATH_;
    PathHandler Component::global_path("","","",0);
    int Component::CLICK_COUNT = 0;

    void Component::usingPaste() const {
        if (IsKeyPressed(KEY_V) && IsKeyDown(KEY_LEFT_CONTROL)) // Ctrl + V
        {
            const char *clipboardText = GetClipboardText();
            if (clipboardText != nullptr)
            {
                strncpy_s(textbox.textBoxText, clipboardText, sizeof(textbox.textBoxText) - 1); //length-1 is total index not the length
                textbox.textBoxText[sizeof(textbox.textBoxText) - 1] = '\0'; // Pastikan null-terminated
            }
        }
    }

    Component::Component(int width, int height,int x,int y):
    bbox_component(y,x,width,height)
    ,colShape{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}
    ,mode(Mode::button),eval_table(0,0)
    {}

    Component::Component(const std::vector<std::string> &header,int width, int height,int x,int y): //for table
    bbox_component(y,x,0,0)
    ,colShape{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}
    ,mode(Mode::button), eval_table(static_cast<float>(width),static_cast<float>(height)), header(header)
    {
        eval_table.cols = static_cast<float>(header.size());
        // std::cout <<"Ukuran kolom: "<< eval_table.cols << std::endl;
        bbox_component.pan_width = static_cast<int>(header.size()) * width;
        bbox_component.pan_height = height;
        // bbox_component.right_X = bbox_component.left_X+(width*static_cast<int>(header.size()));
        // pan_heigth akan di inputkan di setClass
    }

    Component::Component(const char *default_text, int width, int height, std::string icon,int x, int y ):
    bbox_component(y,x,width,height)
    ,colShape{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}
    ,mode(Mode::button), icon(std::move(icon)), eval_table(0,0)
    {
        textbox.default_text = default_text;
        strncpy_s(textbox.textBoxText, default_text, sizeof(textbox.textBoxText)-1);
        textbox.textBoxText[sizeof(textbox.textBoxText) - 1] = '\0';

    }

    Component & Component::setComponentPos(int x, int y) {

        bbox_component.left_X = x;
        bbox_component.top_Y = y;
        return *this;
    }

    Component & Component::setColShape() { //colShape == collusion shape
        this->colShape.x = static_cast<float>(bbox_component.left_X);
        this->colShape.y = static_cast<float>(bbox_component.top_Y);
        return *this;
    }

    Component & Component::applyMode() { //declare in while loop
        switch (this->mode) {
            case Mode::button:
            {
                if (!GuiButton(colShape, this->textbox.textBoxText)) {
                    isClicked = false;
                    // std::cout << "button isn't clicked " << std::endl;
                }else isClicked = true;
            } break;
            case Mode::image:
            {
                if (!GLOB_PATH_.empty()) {
                    im_handler.image = cv::imread(GLOB_PATH_);
                    if (im_handler.image.empty()) {
                        std::cerr << "Failed to load image: " << im_handler.image << "\n";
                        break;
                    }
                }else {
                    im_handler.image = cv::imread(global_path.image);
                    if (im_handler.image.empty()) {
                        std::cerr << "Failed to load image: " << im_handler.image << "\n";
                        break;
                    }
                }
                //resize gambar
                cv::Mat resized_img;
                cv::resize(im_handler.image,resized_img, cv::Size(224, 224));

                std::vector<unsigned char> buffer;
                if (!imencode(".jpg", resized_img, buffer)) {
                    std::cerr << "Failed to encode image to buffer.\n";
                    return *this;
                }

                Image raylibImage = LoadImageFromMemory(".jpg", buffer.data(), static_cast<int>(buffer.size()));
                if (raylibImage.data == nullptr) {
                    std::cerr << "Failed to create raylib image from OpenCV data.\n";
                    return *this; // Kembali jika gagal
                }

                im_handler.image_texture = LoadTextureFromImage(raylibImage);
                UnloadImage(raylibImage);
            }break;
            case Mode::textbox:
            {
                this->mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse,this->colShape )) {
                    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        memset(textbox.textBoxText, 0, sizeof(textbox.textBoxText));
                        textbox.textBoxActive = true;
                    }
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        strncpy_s(textbox.textBoxText, textbox.textBoxText, sizeof(textbox.textBoxText)-1);
                        textbox.textBoxText[sizeof(textbox.textBoxText) - 1] = '\0';
                        textbox.textBoxActive = true;
                    }

                    if (textbox.textBoxActive) {
                        usingPaste();
                    }
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        textbox.final_path = textbox.textBoxText;
                        isClicked = true;
                        textbox.textBoxActive = false; // Mengaktifkan text box saat Enter ditekan
                    } else isClicked = false;
                }
                else //no collusion
                {
                    if ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && textbox.textBoxActive ) || IsKeyPressed(KEY_ENTER)) //&& IsPathFile(textbox.textBoxText
                    {
                        textbox.final_path = textbox.textBoxText;
                        isClicked = true;
                        textbox.textBoxActive = false;
                    }else isClicked = false;

                    if (textbox.textBoxActive)
                    { //paste mode
                        usingPaste();
                    }
                    if (textbox.textBoxText[0] == '\0' && !textbox.textBoxActive) {
                        strncpy_s(textbox.textBoxText, textbox.default_text.c_str(), sizeof(textbox.textBoxText) - 1); //length-1 is total index not the length
                        textbox.textBoxText[sizeof(textbox.textBoxText) - 1] = '\0'; // Pastikan null-terminated
                    }
                }
            }break;
            case Mode::table:
            {
                if (!global_path.label.empty() && IsFileExtension(global_path.label.c_str(),".csv")) {
                    rapidcsv::Document csv_doc(global_path.label);
                    this->classes = csv_doc.GetColumn<std::string>("Name");
                    eval_table.rows = static_cast<float>(classes.size());
                    // bbox_component.pan_height = bbox_component.pan_height * eval_table.rows;
                    bbox_component.bottom_Y = bbox_component.top_Y + bbox_component.pan_height;
                    bbox_component.right_X = bbox_component.left_X+(bbox_component.pan_width);

                }else break;
            }break;
            case Mode::spinner:
            {
                this->mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse,this->colShape )) {
                    spin_handler.spinnerValue -= static_cast<int>(GetMouseWheelMove());
                }
            }break;
            default: ;
        }
        return *this;
    }

    Component & Component::droppedMode() {
        this->mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse,this->colShape )) {
            if (IsFileDropped()) {
                drop_file = LoadDroppedFiles();
                textbox.textBoxActive = true;
                memset(textbox.textBoxText, 0, sizeof(textbox.textBoxText));
                if (IsFileExtension(drop_file.paths[0], ".jpg") || IsFileExtension(drop_file.paths[0], ".pt")
                    || IsFileExtension(drop_file.paths[0], ".pth") || IsFileExtension(drop_file.paths[0], ".csv")) {
                    TextCopy(textbox.textBoxText, drop_file.paths[0]);
                    UnloadDroppedFiles(drop_file);
                }else {
                    UnloadDroppedFiles(drop_file);
                    std::cout << "your file is not supported" << std::endl;
                }
                if (IsFileExtension(textbox.textBoxText, ".jpg")) {
                    elm::Component::GLOB_PATH_ = textbox.textBoxText;
                    textbox.final_path = textbox.textBoxText;
                    this->isClicked = true;
                    textbox.textBoxActive = false;
                } else {
                    textbox.final_path = textbox.textBoxText;
                    this->isClicked = true;
                    textbox.textBoxActive = false;
                }
            }
        }

        return *this;
    }

    Component & Component::setStaticPath(int id) {
        switch (id) {
            case 1: {
                if (IsFileExtension(textbox.final_path.c_str(), ".pt") || IsFileExtension(textbox.final_path.c_str(), ".pth")) {
                    global_path.module = std::move(textbox.final_path);
                }else std::cout << "The file is no matched with models"<< std::endl;
            }break;
            case 2: {
                if (IsFileExtension(textbox.final_path.c_str(), ".jpg") || IsFileExtension(textbox.final_path.c_str(), ".jpeg")) {
                    GLOB_PATH_ = textbox.final_path;
                    global_path.image = std::move(textbox.final_path);
                }else std::cout << "The file is no matched with image"<< std::endl;
            }break;
            case 3: {
                if (IsFileExtension(textbox.final_path.c_str(), ".csv") || IsFileExtension(textbox.final_path.c_str(), ".txt")) {
                    global_path.label = std::move(textbox.final_path);
                }else std::cout << "The file is no matched with csv"<< std::endl;
            }break;
            case 4: {
                if (spin_handler.spinnerValue) {
                    global_path.spinner_v = spin_handler.spinnerValue;
                }else std::cout << "is not expected"<< std::endl;
            }break;
            default:;
        }


        return *this;
    }

    std::string Component::getCorectPath() const {
        if (!textbox.final_path.empty()) {
            return textbox.final_path;
        }
        return GLOB_PATH_;
    }

    PathHandler Component::getGlobalPath() {
        return global_path;
    }

    std::vector<int> Component::getBBox() const {
        return {bbox_component.pan_width, bbox_component.pan_height};
    }

    void Component::setEvalValue(const bool &theme, std::vector<std::array<float, 2>> in_value) { //just for table
        this->isClicked = theme;
        this->eval_value = std::move(in_value);
        std::cout << this->eval_value.size() << std::endl;
    }


    Component & Component::initAsButton() {
        this->mode = Mode::button;
        return *this;
    }

    Component & Component::initAsTextBox() {
        this->mode = Mode::textbox;
        return *this;
    }

    Component & Component::initAsImage() {
        this->mode = Mode::image;
        return *this;
    }

    Component & Component::initAsSpinner() {
        this->mode = Mode::spinner;
        return *this;
    }

    Component & Component::initAsEvalTable() {
        this->mode = Mode::table;
        return *this;
    }

    Component & Component::initAsText() {
        this->mode = Mode::text;
        return *this;
    }


    void Component::display() const {
        switch (this->mode) {
            case Mode::button: {
                if (GuiButton(colShape, TextFormat("%s",textbox.default_text.c_str()))) {
                    isClicked = true;
                    if (IsFileExtension(global_path.image.c_str(), ".jpg") || IsFileExtension(global_path.image.c_str(), ".jpeg")) {
                        CLICK_COUNT++;
                    }
                } else {
                    if ((CLICK_COUNT>0)) {
                        GuiButton(colShape, TextFormat("%s%s",icon.c_str(),textbox.default_text.c_str()));
                    }
                    isClicked = false;
                }
            }break;
            case Mode::image: {
                DrawRectangleRounded(colShape, 0, 500, LIGHTGRAY);
                DrawRectangleRoundedLinesEx(colShape, 0, 500,5, WHITE);
                DrawTexture(im_handler.image_texture, static_cast<int>(colShape.x), static_cast<int>(colShape.y), WHITE);
            }break;
            case Mode::textbox: {
                if (textbox.textBoxActive)
                {
                    GuiTextBox(colShape,textbox.textBoxText, sizeof(textbox.textBoxText), true); //mendapatkan text dari sini
                }
                else
                {
                    GuiTextBox(colShape, textbox.textBoxText,sizeof(textbox.textBoxText), false);
                }
            }break;
            case Mode::spinner: {
                GuiSpinner(colShape, TextFormat("%s", spin_handler.spin_text.c_str()), &spin_handler.spinnerValue, spin_handler.minValue, spin_handler.maxValue, spin_handler.editMode);
            }break;
            case Mode::table: {
                //Header
                Color transparant = {0,0,0,0};
                auto text_color = BLACK;
                for (int col = 0; col < static_cast<int>(eval_table.cols); col++) {
                    auto top_x = bbox_component.left_X + static_cast<int>(eval_table.element_width) * col;
                    auto top_y = bbox_component.top_Y + 32 *0;

                    GuiDrawRectangle({static_cast<float>(bbox_component.left_X)+(eval_table.element_width*static_cast<float>(col)),static_cast<float>(bbox_component.top_Y)+32*static_cast<float>(0), eval_table.element_width,eval_table.element_height},0,transparant,GOLD);
                    GuiDrawText(header[col].c_str(), {static_cast<float>(top_x),static_cast<float>(top_y), eval_table.element_width,eval_table.element_height}, TEXT_ALIGN_CENTER, BLACK);
                }
                //Content
                for (int row = 0; row < static_cast<int>(eval_table.rows); row++) {
                    for (int col = 0; col < static_cast<int>(eval_table.cols); col++) {
                        int next_row = 1;

                        auto top_x = bbox_component.left_X + static_cast<int>(eval_table.element_width) * col;
                        auto top_y = bbox_component.top_Y + 32 *(row+next_row);

                        if (row%2 == 0) { //gray&& row > 0
                            GuiDrawRectangle({static_cast<float>(top_x),static_cast<float>(top_y), eval_table.element_width,eval_table.element_height},2,transparant,LIGHTGRAY);
                        }
                        else { //white
                            GuiDrawRectangle({static_cast<float>(top_x),static_cast<float>(top_y), eval_table.element_width,eval_table.element_height},2,transparant,WHITE);
                        }

                        if (!eval_value.empty() && (row == static_cast<int>(eval_value[row][0]))) { //highlight
                            GuiDrawRectangle({static_cast<float>(top_x), static_cast<float>(top_y), eval_table.element_width, eval_table.element_height}, 2, transparant, BLUE);
                            text_color = WHITE;
                        }else {
                            text_color = BLACK;
                        }

                        if ( col == 0){ // showing class label
                            GuiDrawText(classes[row].c_str(), {static_cast<float>(top_x),static_cast<float>(top_y), eval_table.element_width,eval_table.element_height}, TEXT_ALIGN_LEFT, text_color);
                        }

                        if (col == 1 && !eval_value.empty()) { //showing prediction
                            if (eval_value[row][0] == -1) {
                                GuiDrawText(TextFormat("%03.4f", 0),{static_cast<float>(top_x), static_cast<float>(top_y), eval_table.element_width, eval_table.element_height}, TEXT_ALIGN_CENTER, text_color);
                            }else {
                                GuiDrawText(TextFormat("%03.4f", eval_value[row][0]),{static_cast<float>(top_x), static_cast<float>(top_y), eval_table.element_width, eval_table.element_height}, TEXT_ALIGN_CENTER, text_color);
                            }

                        }
                        if (col == 2 && !eval_value.empty()) { // showing probability softmax
                            GuiDrawText(TextFormat("%03.4f", eval_value[row][1]), // Tampilkan nilai kedua di kolom 2
                                            {static_cast<float>(top_x), static_cast<float>(top_y), eval_table.element_width, eval_table.element_height}, TEXT_ALIGN_CENTER, text_color);
                        }
                        //footer
                        if (row == static_cast<int>(eval_table.rows) - 1 && col >= static_cast<int>(eval_table.cols) - this->header.size()) {
                            GuiDrawRectangle({static_cast<float>(top_x), static_cast<float>(top_y)+32, eval_table.element_width, eval_table.element_height}, 2, transparant, MAROON);
                        }
                    }
                }

            }break;
            case Mode::text: {
                GuiLabel(colShape, textbox.default_text.c_str());
            }
            default: ;
        }
    }

    Component::~Component() = default;

}
