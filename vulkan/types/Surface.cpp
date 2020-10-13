//
// Created by petr on 10/12/20.
//

#include "Surface.h"
const vk::SurfaceKHR &pf::vulkan::Surface::getSurface() { return vkSurface.get(); }
std::string pf::vulkan::Surface::info() const { return "Vulkan unique surface"; }
const vk::SurfaceKHR &pf::vulkan::Surface::operator*() const { return *vkSurface; }
vk::SurfaceKHR const *pf::vulkan::Surface::operator->() const { return &*vkSurface; }
