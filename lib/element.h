//
// Created by triil on 1/4/2025.
//

#pragma once
// #define RAYGUI_IMPLEMENTATION
#include <string>
#include <utility>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
// #include "raygui.h"
#include "raylib.h"


namespace elm {

    struct BBox {
        int top_Y, left_X;
        int right_X, bottom_Y;
        int center_H, center_V;
        int pan_width, pan_height;

        BBox(int top, int left, int width, int height): top_Y(top), left_X(left),
                                                        right_X(0), bottom_Y(0), center_H(0),
                                                        center_V(0),
                                                        pan_width(width),
                                                        pan_height(height) {}
    };

    struct TextBox {
        mutable char textBoxText[1024];
        bool textBoxActive;
        mutable std::string default_text;
        mutable std::string final_path;

        TextBox(): textBoxText(""), textBoxActive(false),
        default_text(R"(#8#Inputkan path file)") {}
    };

    struct ImageHandler {
        mutable cv::Mat image;
        mutable Texture2D image_texture;

        ImageHandler(): image({}), image_texture( {}) {}
    };

    struct SpinnerHandler {
        mutable int spinnerValue;
        int minValue;
        int maxValue;
        bool editMode;
        std::string spin_text;

        SpinnerHandler(): spinnerValue(224), minValue(0),maxValue(2000), editMode(false), spin_text("Input size: ") {}

    };

    struct PathHandler {
        std::string module;
        std::string image;
        std::string label;
        int spinner_v;
        PathHandler(std::string module, std::string image,std::string label, int spinner_v):
        module(std::move(module)), image(std::move(image)), label(std::move(label)) ,spinner_v(spinner_v) {}
    };

    struct TableHandler {
        float rows;
        float cols;
        float element_width;
        float element_height;

        TableHandler(float element_width, float element_height): rows(0), cols(0), element_width(element_width),
                                                             element_height(element_height) {
        }
    };

    //CLASS FROM HERE
    class Component {
        enum class Mode {button, image, textbox, spinner, table,text};
        static PathHandler global_path;
        BBox bbox_component;
        Rectangle colShape;
        Mode mode;
        TextBox textbox;
        ImageHandler im_handler;
        static std::string GLOB_PATH_;
        Vector2 mouse = { 0.0f, 0.0f };
        SpinnerHandler spin_handler;
        FilePathList drop_file{0,0, nullptr};
        static int CLICK_COUNT;
        std::string icon;

        //Table Section
        std::vector<std::string> classes{};
        TableHandler eval_table;
        std::vector<std::string> header;
        std::vector<std::array<float, 2>> eval_value{};

        void usingPaste() const;
    public:
        mutable bool isClicked{false};

        explicit Component(int width=0, int height=0, int x=0,int y=0 );
        explicit Component(const std::vector<std::string> &header,int width, int height,int x=0,int y=0);//for table
        explicit Component(const char *default_text="", int width=0, int height=0, std::string icon="",int x=0,int y=0 );
        Component& setComponentPos(int x, int y);
        Component& setColShape();
        Component& applyMode();
        Component& droppedMode();
        Component& setStaticPath(int id=1);
        void setEvalValue(const bool &theme, std::vector<std::array<float, 2>> in_value);
        [[nodiscard]] std::string getCorectPath() const;
        [[nodiscard]] static PathHandler getGlobalPath();
        [[nodiscard]] std::vector<int> getBBox() const;
        Component& initAsButton();
        Component& initAsTextBox();
        Component& initAsImage();
        Component& initAsSpinner();
        Component& initAsEvalTable();
        Component& initAsText();
        void display() const;

        //Table Handler section
        void columnCounter();
        void rowCounter();
        ~Component();
    protected:
        void drawRectangleWithLines();
        // void
    };
}
