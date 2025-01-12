//
// Created by triil on 1/4/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include "element.h"

namespace rtg{
    inline Color hex_to_rgb(const std::string &color) {
        const std::string hexColor = std::string("0x") + color + std::string("ff");
        unsigned int colorValue = std::stoul(hexColor, nullptr, 16); // Basis 16 untuk hexadecimal
        Color rgbColor = GetColor(colorValue);
        return rgbColor;
    }

    struct BBox {
        int top_Y, left_X;
        int right_X, bottom_Y;
        int center_H, center_V;
        int pan_width, pan_height;

        BBox(int top, int left, int width, int height): top_Y(top), left_X(left),
                                                        right_X(0), bottom_Y(0), center_H(0),
                                                        center_V(0),
                                                        pan_width(width),
                                                        pan_height(height) {
        }
    };
    struct Margin {
        unsigned int margin_top;
        unsigned int margin_left;
        unsigned int margin_right;
        unsigned int margin_bottom;
        Margin(int m_top, int m_left, int m_right, int m_bottom) : margin_top(m_top),
        margin_left(m_left), margin_right(m_right), margin_bottom(m_left) {}
    };

    template<typename T>
    class Panel4Panel {
        BBox bbox;
        Margin margin_;
        std::vector<std::weak_ptr<T>> elementStack;
        //METHOD
        void setFourAxis()
        {
            bbox.right_X = bbox.left_X + bbox.pan_width;
            bbox.bottom_Y = bbox.top_Y + bbox.pan_height;
            bbox.center_V = bbox.pan_height/2;
            bbox.center_H = bbox.pan_height/2;
        }

    public:
        static int WIN_WIDTH_;
        static int WIN_HEIGHT_;

        //METHOD
        Panel4Panel(int x, int y, int width, int height): bbox(y, x, width, height), margin_(0,0,0,0) {
            setFourAxis();
        };
        Panel4Panel& setElement(const std::shared_ptr<T>& element) {
            if (element) {
                elementStack.emplace_back(element);
            }
            return *this;
        }

        Panel4Panel& marginAll(unsigned int margin=0) {
            bbox.left_X = bbox.left_X+margin;
            bbox.top_Y = bbox.top_Y+margin;

            if (bbox.right_X >= WIN_WIDTH_ ){
                bbox.right_X = WIN_WIDTH_-margin;
            }
            else bbox.right_X = bbox.right_X+margin;
            if (bbox.bottom_Y >= WIN_HEIGHT_) {
                bbox.bottom_Y = WIN_HEIGHT_-margin;
            }
            else bbox.bottom_Y = bbox.bottom_Y+margin;

            bbox.pan_width = bbox.right_X - bbox.left_X;
            bbox.pan_height = bbox.bottom_Y - bbox.top_Y;

            setFourAxis();
            return *this;
        }

        Panel4Panel& margin(Margin margin) {
            this->margin_ = std::move(margin);
            bbox.left_X = bbox.left_X+margin_.margin_left;
            bbox.top_Y = bbox.top_Y+margin_.margin_top;

            if (bbox.right_X >= WIN_WIDTH_ ){
                bbox.right_X = WIN_WIDTH_-margin_.margin_right;
            }
            else bbox.right_X = bbox.right_X+margin_.margin_right;
            if (bbox.bottom_Y >= WIN_HEIGHT_) {
                bbox.bottom_Y = WIN_HEIGHT_-margin_.margin_bottom;
            }
            else bbox.bottom_Y = bbox.bottom_Y+margin_.margin_bottom;

            bbox.pan_width = bbox.right_X - bbox.left_X;
            bbox.pan_height = bbox.bottom_Y - bbox.top_Y;

            setFourAxis();
            return *this;
        }

        void display(Color color=BLUE, float roundness=0.01) const {
            DrawRectangleRounded({static_cast<float>(bbox.left_X),static_cast<float>(bbox.top_Y),
                static_cast<float>(bbox.pan_width),static_cast<float>(bbox.pan_height)},roundness, 500,color);
        }

        void displayAll(const std::string &colorElement="ffffff", float roundness=0.05) const
        { //for display sub panel
            if constexpr (std::is_same_v<T,elm::Component>)
            {
            for (const auto &componentAll : this->elementStack)
            {
            if (auto component = componentAll.lock())
            { // Coba akses objek
                component->display();
            } else std::cout << "Component sudah dihancurkan." << std::endl;
            }
            }
        }

        void topLeft(int l_to_r=0, int t_to_b=0) const
        {
            if constexpr (std::is_same_v<T,elm::Component>)
            { //for component
            if (auto elem = elementStack.back().lock())
            {
                elem->setComponentPos(bbox.left_X + l_to_r, bbox.top_Y + t_to_b).setColShape();
            }else std::cout << "componentStack sudah dihancurkan\n";
            }
        }
        void topRight(int r_to_l=0, int t_to_b=0) const
        {
            if constexpr (std::is_same_v<T,elm::Component>)
            { //for component
            if (auto elem = elementStack.back().lock())
            {
                std::vector<int> borrow_size = elem->getBBox();
                elem->setComponentPos(bbox.right_X-borrow_size[0] + r_to_l, bbox.top_Y + t_to_b).setColShape();
            }else std::cout << "componentStack sudah dihancurkan\n";
            }
        }

        void bottomRight(int r_to_l=0, int b_to_t=0) const
        {
            if constexpr (std::is_same_v<T,elm::Component>)
            { //for component
                if (auto elem = elementStack.back().lock())
                {
                    std::vector<int> borrow_size = elem->getBBox();
                    elem->setComponentPos(bbox.right_X-borrow_size[0] + r_to_l, bbox.bottom_Y-borrow_size[1] - b_to_t).setColShape();
                }else std::cout << "componentStack sudah dihancurkan\n";
            }
        }
        void bottomLeft(int l_to_r=0, int b_to_t=0) const
        {
            if constexpr (std::is_same_v<T,elm::Component>)
            { //for component
                if (auto elem = elementStack.back().lock())
                {
                    std::vector<int> borrow_size = elem->getBBox();
                    elem->setComponentPos(bbox.left_X+ l_to_r, bbox.bottom_Y-borrow_size[1] - b_to_t).setColShape();
                }else std::cout << "componentStack sudah dihancurkan\n";
            }
        }

        void centerTop(int l_to_r=0, int t_to_b=0) const
        {
            if constexpr (std::is_same_v<T,elm::Component>)
            { //for component
                if (auto elem = elementStack.back().lock())
                {
                    std::vector<int> borrow_size = elem->getBBox();
                    elem->setComponentPos((WIN_WIDTH_/2)-(borrow_size[0]/2)+ l_to_r, bbox.top_Y-t_to_b).setColShape();
                }else std::cout << "componentStack sudah dihancurkan\n";
            }
        }

        ~Panel4Panel() = default;
    };

    template<typename T>
    int Panel4Panel<T>::WIN_WIDTH_ = 0;

    template<typename T>
    int Panel4Panel<T>::WIN_HEIGHT_ = 0;
}
